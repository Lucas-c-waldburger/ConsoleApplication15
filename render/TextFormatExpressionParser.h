#pragma once
#include <variant>
#include <span>
#include <charconv>
#include "../ecs/Ecs.h"
#include "../atlas/NewTextureRepository.h"
#include "../core/StringUtils.h"

static constexpr char kTextFormatScopeOpen = '{';
static constexpr char kTextFormatScopeClose = '}';
static constexpr char kTextFormatOperationChar = '@';
static constexpr char kTextFormatArgsOpen = '(';
static constexpr char kTextFormatArgsClose = ')';
static constexpr char kTextFormatArgSep = ',';
static constexpr char kTextFormatEscape = '\\';
static constexpr char kTextFormatContentStart = ':';

struct TextFormatGlyphCache
{
	TextFormatGlyphCache() : renderProfiles({ RenderProfile{} }) {}
	explicit TextFormatGlyphCache(const RenderProfile& defaultProfile) :
		renderProfiles({ defaultProfile }) {
	}

	struct Slice
	{
		GlyphCacheData& glyphData;
		SDL_Rect& destRect;
		SDL_FPoint& rotationCenter;
		RenderProfile& renderProfile;
	};

	std::vector<GlyphCacheData> glyphData;
	std::vector<SDL_Rect> destRects;
	std::vector<SDL_FPoint> rotationCenters;
	std::vector<size_t> profileIndices;

	std::vector<RenderProfile> renderProfiles;

	void Reserve(size_t count)
	{
		glyphData.reserve(count);
		destRects.reserve(count);
		rotationCenters.reserve(count);
		profileIndices.reserve(count);
	}

	size_t EmplaceBack(const GlyphCacheData& glyph, const SDL_Rect& destRect,
		const SDL_FPoint& rotationCenter, size_t profileIndex)
	{
		glyphData.emplace_back(glyph);
		destRects.emplace_back(destRect);
		rotationCenters.emplace_back(rotationCenter);
		profileIndices.emplace_back(profileIndex);

		return glyphData.size() - 1;
	}

	Slice GetSlice(size_t idx)
	{
		assert(idx < glyphData.size());
		assert(profileIndices[idx] < renderProfiles.size());

		return Slice{
			.glyphData = glyphData[idx],
			.destRect = destRects[idx],
			.rotationCenter = rotationCenters[idx],
			.renderProfile = renderProfiles[profileIndices[idx]]
		};
	}

};


using TextFormatExpressionOp = Result<Void>(*)(TextFormatGlyphCache::Slice&&,
	const std::vector<std::string_view>&);

class TextFormatExpressionOpMap
{
public:
	static TextFormatExpressionOp GetOperation(std::string_view name)
	{
		auto it = map_.find(name);
		return (it != map_.end()) ? it->second : nullptr;
	}

private:
	using MapType = std::unordered_map<std::string_view, TextFormatExpressionOp>;

	TextFormatExpressionOpMap() = delete;

	static MapType map_;
};

struct TextFormatExpressionComponents
{
	std::string_view operation;
	std::vector<std::string_view> arguments;
	std::string_view text;
	std::vector<TextFormatExpressionComponents> nestedComponents;
};

inline Result<Void>
SetTextColorOperation(TextFormatGlyphCache::Slice&& slice, const std::vector<std::string_view>& args)
{
	auto& [r, g, b] = slice.renderProfile.mods.color;

	TRY_ASSIGN(r, StringViewToInt(args[0]));
	TRY_ASSIGN(g, StringViewToInt(args[1]));
	TRY_ASSIGN(b, StringViewToInt(args[2]));

	return kVoid;
}

inline Result<Void>
SetTextAlphaOperation(TextFormatGlyphCache::Slice&& slice, const std::vector<std::string_view>& args)
{
	TRY_ASSIGN(slice.renderProfile.mods.alpha, StringViewToInt(args[0]));

	return kVoid;
}

inline size_t FindNextCharPos(std::string_view srcStr, char targetChar, size_t startPos, size_t endCap)
{
	size_t currentPos = startPos;
	do
	{
		currentPos = srcStr.find_first_of(targetChar, currentPos);
		if (currentPos == std::string_view::npos || currentPos >= endCap)
		{
			return std::string_view::npos;
		}
	} while (currentPos > 0 && srcStr[currentPos - 1] == kTextFormatEscape);

	return currentPos;
}

class TextFormatExpressionParser
{
public:
	static Result<TextFormatGlyphCache> MakeGlyphCache(std::string_view srcStr, const RenderProfile& profile)
	{
		TRY(ParseText(srcStr), fmtComponents);

		TextFormatGlyphCache cache{ profile };
		cache.Reserve(srcStr.size());

		TRY(MakeGlyphCacheImpl(cache, fmtComponents, profile));

		return cache;
	}

private:
	TextFormatExpressionParser() = delete;

	static Result<Void> MakeGlyphCacheImpl(TextFormatGlyphCache& resultCache,
										   const std::vector<TextFormatExpressionComponents>& fmtComponents,
										   const RenderProfile& currentProfile)
	{
		for (const auto& components : fmtComponents)
		{
			auto op = !components.operation.empty()
				? TextFormatExpressionOpMap::GetOperation(components.operation)
				: nullptr;

			size_t profileIdx = 0;
			if (op)
			{
				// will need a new profile to be mutated by operation
				profileIdx = resultCache.renderProfiles.size();
				resultCache.renderProfiles.emplace_back(currentProfile);
			}

			size_t glyphIdx = 0;
			for (const char c : components.text)
			{
				// { make glyphs like normal (and cut out \n, etc...) }
				glyphIdx = resultCache.EmplaceBack({}, {}, {}, profileIdx);

				if (op)
				{
					TRY(std::invoke(*op, resultCache.GetSlice(glyphIdx), components.arguments));
				}
			}

			assert(glyphIdx < resultCache.glyphData.size());

			if (!components.nestedComponents.empty())
			{
				TRY(MakeGlyphCacheImpl(resultCache, components.nestedComponents,
					resultCache.renderProfiles[resultCache.profileIndices[glyphIdx]]));
			}
		}

		return kVoid;
	}

	static Result<std::vector<TextFormatExpressionComponents>> ParseText(std::string_view srcStr)
	{
		std::vector<TextFormatExpressionComponents> result;
		size_t head = 0;

		while (head < srcStr.size())
		{
			TRY_ASSIGN(head, ParseTextImpl(srcStr, head, result));
		}

		return result;
	}

	static constexpr std::string_view kTextNestedStr =
		"You're my {@color(255,0,0):BEST {@alpha(112):BUBBA}} dude!";

	static Result<size_t> ParseTextImpl(std::string_view srcStr, size_t head,
										std::vector<TextFormatExpressionComponents>& result)
	{
		if (head >= srcStr.size())
		{
			return head;
		}

		auto findNext = [&srcStr](char targetChar, size_t startPos, size_t endCap) {
			return FindNextCharPos(srcStr, targetChar, startPos, endCap);
			};

		auto& component = result.emplace_back();

		const size_t scopeOpenPos = findNext(kTextFormatScopeOpen, head, srcStr.size());
		if (scopeOpenPos == std::string_view::npos)
		{
			// no more scopes, srcStr just text content or end scope ('}')
			const size_t scopeClosePos = findNext(kTextFormatScopeClose, head, srcStr.size());
			component.text = srcStr.substr(head, scopeClosePos - head + 1);

			return srcStr.size();
		}

		// find potential next scope open '{' (or npos if none)
		size_t nextScopeOpenPos = srcStr.find_first_of(kTextFormatScopeOpen, scopeOpenPos + 1);

		// find operation
		const size_t opPos = findNext(kTextFormatOperationChar, scopeOpenPos + 1, nextScopeOpenPos);
		if (opPos == std::string_view::npos)
		{
			return MAKE_ERROR("No operation char '@' found after scope open '{'");
		}

		// find '('
		const size_t argsOpenPos = findNext(kTextFormatArgsOpen, opPos + 1, nextScopeOpenPos);
		if (argsOpenPos == std::string_view::npos)
		{
			return MAKE_ERROR("No args open char '(' found after text format operation");
		}

		// make sure op is in the map
		auto opStr = TrimWhitespace(srcStr.substr(0, argsOpenPos + 1));
		if (!TextFormatExpressionOpMap::GetOperation(opStr))
		{
			return MAKE_ERROR_FMT("No text format operation named '{}'", opStr);
		}

		// find ')'
		const size_t argsClosePos = findNext(kTextFormatArgsClose, argsOpenPos + 1, nextScopeOpenPos);
		if (argsClosePos == std::string_view::npos)
		{
			return MAKE_ERROR("No ')' found after text format args open char '('");
		}

		// gather operation arguments between '(' and ')'
		std::vector<std::string_view> args;
		size_t currentArgsPos = 0;

		auto argsStr = srcStr.substr(argsOpenPos + 1, argsClosePos - argsOpenPos + 1);
		while (currentArgsPos < argsStr.size())
		{
			size_t commaPos = findNext(kTextFormatArgSep, currentArgsPos, argsStr.size());

			args.emplace_back(TrimWhitespace(argsStr.substr(currentArgsPos, commaPos)));

			currentArgsPos = commaPos + 1;
		}

		// find ':'
		const size_t contentStartPos = findNext(kTextFormatContentStart, argsClosePos + 1, nextScopeOpenPos);
		if (contentStartPos == std::string_view::npos)
		{
			return MAKE_ERROR("No text content begin char ':' found after text format args close char ')'");
		}

		// find '}'
		size_t currentScopeClosePos = findNext(kTextFormatScopeClose, contentStartPos, nextScopeOpenPos);
		if (currentScopeClosePos == std::string_view::npos)
		{
			return MAKE_ERROR("No '}' found after text content");
		}

		head = currentScopeClosePos + 1;
		// see if another operation scope nested inside current scope 
		// ('{' before current level's closing '}'), change head
		if (nextScopeOpenPos < currentScopeClosePos)
		{
			TRY_ASSIGN(head, ParseTextImpl(srcStr, nextScopeOpenPos, component.nestedComponents));
		}

		component.text = srcStr.substr(contentStartPos + 1, currentScopeClosePos - contentStartPos + 1);

		return head;
	}
};
