#include "SerializationSystemUtils.h"
#include "../../core/Algorithms.h"
#include "../../serial/user_types/AtlasJsonUserTypes.h"
#include "../../serial/user_types/RenderableJsonUserTypes.h"
#include "../../serial/Serialization.h"
#include "../../components/RenderableComponent.h"
#include "../../ecs/Ecs.h"

namespace util {

namespace {


//bool IsDuplicateSpriteDescriptor(const SpriteDescriptor& descriptor, 
//					             const std::vector<SpriteDescriptorPackage>& packages)
//{
//	return core::Find(packages, descriptor) != packages.end();
//}

//Result<nlohmann::ordered_json> MakeSerializedSpriteInfo

} // unnamed

//SpriteDescriptorPackageMaker::SpriteDescriptorPackageMaker()
//{
//	spriteSeriesNameToPackageIndex_[kNoSeriesName] = 0;
//	packages_.emplace_back();
//}
//
//Result<Void> SpriteDescriptorPackageMaker::PushBack(SpriteInfo&& info)
//{
//	if (info.filepath.empty())
//	{
//		return MAKE_ERROR("Sprite info filepath was empty");
//	}
//	if (!std::filesystem::exists(info.filepath))
//	{
//		return MAKE_ERROR("Sprite info filepath does not exist");
//	}
//
//	SpriteDescriptor newDescriptor{
//		.spriteName = std::move(info.spriteName),
//		.filepath = std::move(info.filepath)
//	};
//	
//	// Not in a series
//	if (info.seriesName.empty())
//	{
//		// if not duplicate descriptor, add
//		if (core::Find(packages_.front().descriptors, newDescriptor) == 
//			packages_.front().descriptors.end())
//		{
//			packages_.front().descriptors.emplace_back(std::move(newDescriptor));
//		}
//
//		return Void{};
//	}
//	
//	auto it = spriteSeriesNameToPackageIndex_.find(info.seriesName);
//	if (it == spriteSeriesNameToPackageIndex_.end())
//	{
//		// in a new series
//		size_t seriesIdx = packages_.size();
//
//		auto& descriptorRef = packages_.emplace_back(SpriteDescriptorPackage{
//			.descriptors = { std::move(newDescriptor) },
//			.seriesName = std::move(info.seriesName)
//		});
//
//		spriteSeriesNameToPackageIndex_[descriptorRef.seriesName] = seriesIdx;
//
//		return Void{};
//	}
//
//	// part of an existing series
//	const size_t packageIdx = it->second;
//	assert(packageIdx < packages_.size());
//
//	auto& seriesDescriptors = packages_[packageIdx].descriptors;
//	if (seriesDescriptors.size() <= info.seriesIndex)
//	{
//		seriesDescriptors.resize(info.seriesIndex + 1);
//	}
//
//	auto& destination = seriesDescriptors[info.seriesIndex];
//	if (!destination.filepath.empty() && destination == newDescriptor)
//	{
//		return MAKE_ERROR_FMT("Conflicting descriptor data for series '{}' at index '{}'",
//			packages_[packageIdx].seriesName, info.seriesIndex);
//	}
//
//	destination = std::move(newDescriptor);
//
//	return Void{};
//}
//template <typename Fn, typename Arg>
//Result<Void> ExpandTillError(Fn&& fn, Arg&& arg) 
//{
//	return std::invoke(fn, std::forward<Arg>(arg));
//}
//
//template <typename Fn, typename Arg, typename...Args>
//Result<Void> ExpandTillError(Fn&& fn) {
//	if (auto result = f.template operator() < T > ();     // call for T
//		result != FailReturn{})                       // adjust test as needed
//		return result;                                // fail immediately
//
//	if constexpr (sizeof...(Ts) > 0)
//		return short_circuit_expand<FailReturn, F, Ts...>(std::forward<F>(f));
//
//	return FailReturn{};
//}

//std::array stringViews{ std::string_view{svs}... };

//template <typename Fn, typename...Ts, typename...Strs>
//Result<Void> ForEachString(Fn&& fn, const Strs&...strs) 
//	requires ((std::convertible_to<Strs, std::string_view> && ...) && 
//			  (std::is_invocable_r_v<Result<Void>, Fn, std::string_view>))
//{
//	//for (const auto& sv : { std::string_view{svs}... })
//	//{
//	//	TRY(std::invoke(fn, sv));
//	//}
//	//return Void{};
//	auto call = [&](const std::string_view sv) -> Result<Void> {
//		return std::invoke(fn, sv);
//	};
//
//	((TRY(call(std::string_view{ strs }))), ...);
//
//	return Void{};
//}


template <typename BasicJson, typename...Keys> requires 
	(std::convertible_to<Keys, std::string_view> && ...)
Result<Void> ValidateJsonKeys(BasicJson& j, const Keys&...keys) 
{	
	auto impl = [&](std::string_view key) -> Result<Void> {
		if (!j.contains(key))
		{
			return MAKE_ERROR_FMT("Atlas JSON missing key '{}'", key);
		}
		return Void{};
	}

	((TRY(impl(std::string_view{ strs }))), ...);

	return Void{};
}

Result<FixedSpriteAtlas> 
TextureRepositorySerializationHelper::DeserializeSpriteAtlas(nlohmann::json& j, 
															 SDL_Renderer* renderer)
{
	if (!j.is_object())
	{
		return MAKE_ERROR("Json was not an object type");
	}

	TRY(ValidateJsonKeys(j,kSpriteInfoKey, kSeriesIndexRangesKey, kTextureSizeKey));

	return FixedSpriteAtlas::Create(renderer,
		j.at(kSpriteInfoKey).get<SpriteInfoSOA>(),
		j.at(kSeriesIndexRangesKey).get<BaseSpriteAtlas::SeriesIndexRangeMap>(),
		j.at(kTextureSizeKey).get<size_t>()
	);
}

Result<Void>
TextureRepositorySerializationHelper::SerializeSpriteAtlas(const DynamicSpriteAtlas& atlas)
{

}

Result<Void> RenderableSerializationHelper::SerializeSprite(
	const Sprite& sprite, nlohmann::json& spriteComponentJson)
{
	if (!sprite.sourceAtlas.IsValid())
	{
		LOG_ERROR("Sprite source atlas handle was marked invalid");

		return Void{};
	}

	auto* spriteAtlas = textureRepository_.GetAtlas<SpriteAtlas>(sprite.sourceAtlas);
	if (!spriteAtlas)
	{
		return MAKE_ERROR("Sprite source atlas not found in Texture Repository");
	}

	auto info = spriteAtlas->GetSpriteInfo(sprite);
	if (info.filepath.empty())
	{
		return MAKE_ERROR("Sprite filepath was empty, meaning sprite index was invalid");
	}

	if (!std::filesystem::exists(info.filepath))
	{
		return MAKE_ERROR("Sprite filepath does not exist");
	}

	spriteComponentJson[kSpriteInfoJsonKey] = info;

	return Void{};
}

Result<Sprite> RenderableSerializationHelper::DeserializeSprite(
	const nlohmann::json& spriteComponentJson)
{
	assert(renderer_);

	// extract sprite info
	if (!spriteComponentJson.contains(kSpriteInfoJsonKey))
	{
		return MAKE_ERROR_FMT("JSON key '{}' not found", kSpriteInfoJsonKey);
	}

	SpriteInfo spriteInfo;
	from_json(spriteComponentJson.at(kSpriteInfoJsonKey), spriteInfo);

	// validate sprite info
	if (spriteInfo.filepath.empty())
	{
		return MAKE_ERROR("Sprite filepath was empty");
	}
	if (!std::filesystem::exists(spriteInfo.filepath))
	{
		return MAKE_ERROR("Sprite filepath does not exist");
	}

	// extract source atlas hash
	if (!spriteComponentJson.contains(kSourceAtlasJsonKey))
	{
		return MAKE_ERROR_FMT("JSON key '{}' not found", kSourceAtlasJsonKey);
	}

	auto& sourceAtlasField = spriteComponentJson.at(kSourceAtlasJsonKey);
	if (!sourceAtlasField.contains(kHandleHashJsonKey))
	{
		return MAKE_ERROR_FMT("JSON key '{}' not found inside '{}'", 
			kHandleHashJsonKey, kSourceAtlasJsonKey);
	}

	auto& hashField = sourceAtlasField.at(kHandleHashJsonKey);
	if (hashField.is_null())
	{
		return MAKE_ERROR_FMT("Value for JSON key '{}' was null", kHandleHashJsonKey);
	}
	if (!hashField.is_number())
	{
		return MAKE_ERROR_FMT("Value for JSON key '{}' was not a number", kHandleHashJsonKey);
	}

	const size_t handleHash = hashField.get<size_t>();
	
	// make sprite
	SpriteAtlas* spriteAtlas = nullptr;

	auto it = atlasHandleHashMap_.find(handleHash);
	if (it == atlasHandleHashMap_.end())
	{
		// need new sprite atlas
		TRY(SpriteAtlas::Create(renderer_), temp);
		TRY_ASSIGN(spriteAtlas, textureRepository_.AttachAtlas(std::move(temp)));

		assert(spriteAtlas);

		atlasHandleHashMap_[handleHash] = spriteAtlas->GetHandle();
	}
	else
	{
		spriteAtlas = textureRepository_.GetAtlas<SpriteAtlas>(it->second);
	}

	if (!spriteAtlas)
	{
		return MAKE_ERROR("Sprite atlas could not be retrieved from Texture Repository");
	}
	
	auto sprite = spriteAtlas->GetSprite(spriteInfo.spriteName);
	if (sprite.spriteIndex == kSizeMax)
	{
		// sprite needs to be loaded
		TRY_ASSIGN(sprite, spriteAtlas->LoadSprite(renderer_, SpriteDescriptor{
			.spriteName = std::move(spriteInfo.spriteName),
			.filepath = std::move(spriteInfo.filepath)
		}));
	}

	assert(sprite.spriteIndex != kSizeMax);

	return sprite;
}

Result<Void> ComponentSerializationResolver::Serialize(
	const Entity& entity, nlohmann::json& componentsJson)
{
	if (!entity.IsValid())
	{
		return MAKE_ERROR("Entity was invalid");
	}
	if (!componentsJson.is_object())
	{
		return MAKE_ERROR("Components JSON was not of type object");
	}

	if (entity.HasComponent<SpriteRenderableComponent>())
	{
		static constexpr std::string_view spriteComponentName =
			ComponentName<SpriteRenderableComponent>::value;
		if (!componentsJson.contains(spriteComponentName))
		{
			return MAKE_ERROR_FMT("Components JSON did not contain expected key '{}'",
				spriteComponentName);
		}

		TRY(renderableSerializationHelper_.SerializeSprite(
			entity.GetComponent<SpriteRenderableComponent>().sprite,
			componentsJson.at(spriteComponentName)));
	}

	return Void{};
}

Result<Void> ComponentSerializationResolver::Deserialize(
	Entity& entity, const nlohmann::json& componentsJson)
{
	if (!entity.IsValid())
	{
		return MAKE_ERROR("Entity was invalid");
	}
	if (!componentsJson.is_object())
	{
		return MAKE_ERROR("Components JSON was not of type object");
	}

	if (entity.HasComponent<SpriteRenderableComponent>())
	{
		static constexpr std::string_view spriteComponentName =
			ComponentName<SpriteRenderableComponent>::value;
		if (!componentsJson.contains(spriteComponentName))
		{
			return MAKE_ERROR_FMT("Components JSON did not contain expected key '{}'",
				spriteComponentName);
		}

		TRY_ASSIGN(entity.GetComponent<SpriteRenderableComponent>().sprite,
			renderableSerializationHelper_.DeserializeSprite(
			componentsJson.at(spriteComponentName)));
	}

	return Void{};
}

} //util