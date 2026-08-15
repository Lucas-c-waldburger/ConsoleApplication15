#pragma once
#include <sol/sol.hpp>
#include "LuaNativeTypeIdUtils.h"
#include "LuaFunctionTableParser.h"
#include "../core/Dictionary.h"
#include "../core/commonObjects.h"
#include "../core/Result.h"
#include <limits>
#include <cassert>
#include <span>
#include <bitset>

//template <typename T>
//using lua_arg_t = std::remove_reference_t<T>;
//
//template <typename T>
//using lua_raw_t = std::remove_cv_t<std::remove_pointer_t<lua_arg_t<T>>>;

//template <typename T>
//concept AcceptedLuaTypeQualified = 
//	std::same_as<T, raw_type_t<T>> ||
//	std::same_as<T, const raw_type_t<T>> ||
//	std::same_as<T, raw_type_t<T>*> ||
//	std::same_as<T, raw_type_t<T>&> ||
//	std::same_as<T, const raw_type_t<T>*> ||
//	std::same_as<T, const raw_type_t<T>&>;

//template <typename T>
//concept AcceptedLuaTypeQualified =
//std::same_as<std::remove_reference_t<T>, raw_type_t<T>> ||
//std::same_as<std::remove_reference_t<T>, const raw_type_t<T>> ||
//std::same_as<std::remove_reference_t<T>, raw_type_t<T>*> ||
//std::same_as<std::remove_reference_t<T>, const raw_type_t<T>*>;

template <typename T>
concept AcceptedLuaTypeQualified = 
	!std::is_pointer_v<std::remove_pointer_t<std::remove_reference_t<T>>> &&
	!std::is_rvalue_reference_v<T>;

class LuaFunctionCallHandler
{
public:
	template <AcceptedLuaTypeQualified T>
	static sol::object TryMakeArgumentLuaObject(sol::state_view state, T&& value, uint64_t parsedType);

	template <AcceptedLuaTypeQualified...Args>
	static Result<std::vector<sol::object>>
	MakeArgumentLuaObjects(sol::state_view state, std::span<const uint64_t> parsedArgTypes, 
						   Args&&...args);

	template <AcceptedLuaTypeQualified...Args>
	static Result<std::vector<sol::object>>
	MakeArgumentLuaObjects2(sol::state_view state, std::span<const uint64_t> parsedArgTypes,
							Args&&...args);

	template <AcceptedLuaTypeQualified...Args>
	static Result<Void> CallLuaFunctionQualified(sol::function& fn,
												 std::span<const uint64_t> parsedArgTypes,
												 Args&&...args);

	template <size_t N, typename Tup>
	static size_t GetArgBestFitIndex(uint64_t parsedType, const std::bitset<N>& claimed, Tup&& tup);

private:
	template <typename T>
	static bool RawTypesMatch(uint64_t parsedType);

	static constexpr bool IsParsedArgPointer(uint64_t parsedArg)
	{
		return ((parsedArg >> 32) & static_cast<uint64_t>(LuaTypeQualifiers::Ptr)) != 0;
	}

	//template <AcceptedLuaTypeQualified T>
	//static int GetArugmentSimScore(uint64_t parsedType);

	template <AcceptedLuaTypeQualified T>
	static int GetArgSimScore(uint64_t parsedType, T&& val);

	template <size_t I, size_t N, typename Tup>
	static size_t GetArgBestFitIndexImpl(uint64_t parsedType,
		const std::bitset<N>& claimed, Tup&& tup, int simScore, size_t bestIdx);

	template <size_t I, size_t N, typename Tup>
	static void MakeArgumentLuaObjectsImpl(sol::state_view state,
		std::span<const uint64_t> parsedArgTypes,
		std::bitset<N>& claimed, size_t parsedArgsCurrentIdx,
		std::vector<sol::object>& results, Tup&& tup, size_t& failedAtIdx);

	template <size_t N, typename Tup>
	static Result<Void> MakeArgumentLuaObjectsImpl2(sol::state_view state,
		std::span<const uint64_t> parsedArgTypes,
		std::bitset<N>& claimed,
		std::vector<sol::object>& results, Tup&& tup);

	template <size_t I = 0, size_t N, typename Tup>
	static bool CallTryMakeArgumentLuaObject(sol::state_view state, uint64_t parsedType,
		std::bitset<N>& claimed, Tup&& tup, std::vector<sol::object>& results,
		size_t requestedIdx);

	LuaFunctionCallHandler() = default;
};

//template <AcceptedLuaTypeQualified T>
//inline constexpr uint32_t ParseStrongTypeQualifiers()
//{
//	uint32_t qualifiers = 0;
//
//	if constexpr (IsNativeLuaType<raw_type_t<T>>())
//	{
//		return qualifiers;
//	}
//	else if constexpr (std::is_pointer_v<std::remove_reference_t<T>>)
//	{
//		qualifiers |= LuaTypeQualifiers::Ptr;
//
//		if constexpr (std::is_const_v<std::remove_pointer_t<std::remove_reference_t<T>>>)
//		{
//			qualifiers |= LuaTypeQualifiers::Const;
//		}
//
//		return qualifiers;
//	}
//	else if constexpr (std::is_lvalue_reference_v<T>)
//	{
//		qualifiers |= LuaTypeQualifiers::Ref;
//
//		if constexpr (std::is_const_v<std::remove_reference_t<T>>)
//		{
//			qualifiers |= LuaTypeQualifiers::Const;
//		}
//
//		return qualifiers;
//	}
//	else
//	{
//		return qualifiers;
//	}
//}



template <AcceptedLuaTypeQualified T>
inline sol::object LuaFunctionCallHandler::TryMakeArgumentLuaObject(sol::state_view state, 
																	T&& value, uint64_t parsedType)
{
	const uint32_t parsedQuals = static_cast<uint32_t>(parsedType >> 32);

	if constexpr (IsNativeLuaType<raw_type_t<T>>())
	{
		if constexpr (std::is_pointer_v<std::remove_reference_t<T>>)
		{
			if (!value)
			{
				LOG_ERROR("Lua requested a native value type, but argument passed is a null pointer");
				return {};
			}

			return sol::make_object(state, *value);
		}
		else
		{
			return sol::make_object(state, value);
		}
	}

	const bool isRef = (parsedQuals & LuaTypeQualifiers::Ref) != 0;
	const bool isPtr = (parsedQuals & LuaTypeQualifiers::Ptr) != 0;
	const bool isConst = (parsedQuals & LuaTypeQualifiers::Const) != 0;

	if (!(isRef || isPtr)) // value type
	{
		if constexpr (std::is_pointer_v<std::remove_reference_t<T>>)
		{
			if (!value)
			{
				LOG_ERROR("Lua requested a value type, but argument passed is a null pointer");
				return {};
			}

			return sol::make_object(state, *value);
		}
		else
		{
			if constexpr (std::is_lvalue_reference_v<T> && !std::is_copy_constructible_v<raw_type_t<T>>)
			{
				LOG_ERROR("Lua requested a value type, but argument passed is a non-copyable reference");
				return {};
			}

			return sol::make_object(state, value);
		}
	}

	if (isRef)
	{
		if constexpr (std::is_pointer_v<std::remove_reference_t<T>>)
		{
			if constexpr (std::is_const_v<std::remove_pointer_t<std::remove_reference_t<T>>>)
			{
				if (!isConst)
				{
					LOG_ERROR("Lua requested a reference type, but argument passed is a "
						"pointer to a const object");
					return {};
				}
			}

			if (!value)
			{
				LOG_ERROR("Lua requested a reference type, but argument passed is a null pointer");
				return {};
			}

			if (isConst)
			{
				return sol::make_object(state, std::cref(*value));
			}
			else
			{
				return sol::make_object(state, std::ref(*value));
			}
		}
		else if constexpr (std::is_lvalue_reference_v<T>) // non pointer
		{
			if constexpr (std::is_const_v<std::remove_reference_t<T>>)
			{
				if (!isConst)
				{
					LOG_ERROR("Lua requested a reference type, but argument passed is a "
						"a reference to a const object");
					return {};
				}
				else
				{
					return sol::make_object(state, std::cref(value));
				}
			}
			else // non-const l-value ref
			{
				if (isConst)
				{
					return sol::make_object(state, std::cref(value));
				}
				else
				{
					return sol::make_object(state, std::ref(value));

				}
			}
		}
		else // not ptr, not l-value ref
		{
			LOG_ERROR("Lua requested a reference type, but argument passed is a "
				"non-pointer value type");
			return {};
		}
	}

	else if (isPtr)
	{
		if constexpr (!std::is_pointer_v<std::remove_reference_t<T>>)
		{
			if constexpr (!std::is_lvalue_reference_v<T>)
			{
				LOG_ERROR("Lua requested a pointer type, but argument passed is a "
					"a temporary value object");
				return {};
			}
			else // l-value ref
			{
				if constexpr (std::is_const_v<std::remove_reference_t<T>>)
				{
					if (!isConst)
					{
						LOG_ERROR("Lua requested a pointer to a non-const object, "
							"but argument passed is a reference to a const object");
						return {};
					}
					else
					{
						return sol::make_object(state, &value);
					}
				}
				else // non-const l-value ref
				{
					if (isConst)
					{
						const auto* p = &value;

						return sol::make_object(state, p);
					}
					else
					{
						return sol::make_object(state, &value);
					}
				}
			}
		}
		else // ptr
		{
			if constexpr (std::is_const_v<std::remove_pointer_t<std::remove_reference_t<T>>>)
			{
				if (!isConst)
				{
					LOG_ERROR("Lua requested a pointer to a non-const object, "
						"but argument passed is a pointer to a const object");
					return {};
				}
				else
				{
					return sol::make_object(state, value);
				}
			}
			else // non-const ptr
			{
				if (isConst) // non-const ptr, want const ptr
				{
					const auto* p = value;

					return sol::make_object(state, p);
				}
				else // non-const ptr, want non-const ptr
				{
					return sol::make_object(state, value);
				}
			}
		}
	}

	LOG_ERROR("Ill-formed qualifier bitset");

	return {};
}

template <AcceptedLuaTypeQualified T>
inline int LuaFunctionCallHandler::GetArgSimScore(uint64_t parsedType, T&& val)
{
	if (!RawTypesMatch<T>(parsedType))
	{
		return -1;
	}

	const uint32_t parsedQuals = static_cast<uint32_t>(parsedType >> 32);

	const bool isRef = (parsedQuals & LuaTypeQualifiers::Ref) != 0;
	const bool isPtr = (parsedQuals & LuaTypeQualifiers::Ptr) != 0;
	const bool isConst = (parsedQuals & LuaTypeQualifiers::Const) != 0;

	if constexpr (IsNativeLuaType<raw_type_t<T>>())
	{
		return std::numeric_limits<int>::max();
	}

	if (!(isRef || isPtr)) // value type
	{
		int simScore = 0;

		if constexpr (!std::is_pointer_v<std::remove_reference_t<T>>) // not pointer
		{
			++simScore;

			if constexpr (!std::is_lvalue_reference_v<T>) // not l-value ref
			{
				// true value type (const or non const makes no diff)
				++simScore;
			}
			else if constexpr (!std::is_copy_constructible_v<raw_type_t<T>>)
			{
				return -1; // l-value cant become copy, illegal
			}
		}
		else if constexpr (!std::is_copy_constructible_v<raw_type_t<T>>) // ptr that cant be value copied
		{
			return -1;
		}

		return simScore;
	}

	if (isRef)
	{
		int simScore = 0;

		if constexpr (!std::is_pointer_v<std::remove_reference_t<T>>) // not pointer
		{
			++simScore;

			if constexpr (!std::is_lvalue_reference_v<T>) // not l-value ref
			{
				return -1;
			}
			else if constexpr (std::is_const_v<std::remove_reference_t<T>>)
			{
				if (!isConst)
				{
					return -1; // const l-value, illegal
				}
				else
				{
					++simScore;
				}
			}
			else
			{
				++simScore;
			}
		}
		else // ptr
		{
			++simScore;

			if constexpr (std::is_const_v<std::remove_pointer_t<std::remove_reference_t<T>>>)
			{
				if (!isConst)
				{
					return -1; // ptr to const, illegal
				}
				else
				{
					++simScore;
				}
			}
			else // non-const
			{
				if (!isConst)
				{
					++simScore;
				}
			}
		}
		
		return simScore;
	}

	if (isPtr)
	{
		int simScore = 0;

		if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) // is ptr
		{
			++simScore;

			if constexpr (std::is_const_v<std::remove_pointer_t<std::remove_reference_t<T>>>)
			{
				if (!isConst)
				{
					return -1; // ptr to const, illegal
				}
				else
				{
					++simScore;
				}
			}
			else // non-const
			{
				if (!isConst)
				{
					++simScore;
				}
			}
		}
		else // non-ptr
		{
			++simScore;

			if constexpr (!std::is_lvalue_reference_v<T>)
			{
				return -1; // value type, illegal
			}
			else if constexpr (std::is_const_v<std::remove_reference_t<T>>) // const l-value ref
			{
				if (!isConst)
				{
					return -1; // const ref, illegal
				}
				else
				{
					++simScore;
				}
			}
			else // non-const l-value ref
			{
				if (!isConst)
				{
					++simScore;
				}
			}
		}

		return simScore;
	}
		
	return -1;
}

//template <AcceptedLuaTypeQualified T>
//int LuaFunctionCallHandler::GetArugmentSimScore(uint64_t parsedType)
//{
//	if (!RawTypesMatch<T>(parsedType))
//	{
//		return -1;
//	}
//
//	constexpr uint32_t strongTypeQuals = ParseStrongTypeQualifiers<T>();
//	const uint32_t parsedQuals = static_cast<uint32_t>(parsedType >> 32);
//
//	if (strongTypeQuals == parsedQuals)
//	{
//		return std::numeric_limits<int>::max();
//	}
//
//	uint8_t simScore = 0;
//
//	if ((parsedQuals & LuaTypeQualifiers::Ptr) == (strongTypeQuals & LuaTypeQualifiers::Ptr))
//	{
//		++simScore;
//	}
//	else if ((parsedQuals & LuaTypeQualifiers::Ref) == (strongTypeQuals & LuaTypeQualifiers::Ref))
//	{
//		++simScore;
//	}
//
//	if ((parsedQuals & LuaTypeQualifiers::Const) == (strongTypeQuals & LuaTypeQualifiers::Const))
//	{
//		++simScore;
//	}
//
//	return simScore;
//}

template <size_t I, size_t N, typename Tup>
inline size_t LuaFunctionCallHandler::GetArgBestFitIndexImpl(uint64_t parsedType,
	const std::bitset<N>& claimed, Tup&& tup, int simScore, size_t bestIdx)
{
	if constexpr (I >= N)
	{
		return bestIdx;
	}
	else
	{
		if (!claimed.test(I))
		{
			const int newScore = GetArgSimScore(parsedType, std::get<I>(std::forward<Tup>(tup)));
			if (newScore > simScore)
			{
				simScore = newScore;
				bestIdx = I;
			}
		}

		return GetArgBestFitIndexImpl<I + 1>(parsedType, claimed, std::forward<Tup>(tup), 
			simScore, bestIdx);
	}
}

template <size_t N, typename Tup>
inline size_t LuaFunctionCallHandler::GetArgBestFitIndex(uint64_t parsedType, 
	const std::bitset<N>& claimed, Tup&& tup)
{
	return GetArgBestFitIndexImpl<0>(parsedType, claimed, std::forward<Tup>(tup), -1, 
		std::numeric_limits<size_t>::max());
}

template <AcceptedLuaTypeQualified...Args>
inline Result<std::vector<sol::object>> 
LuaFunctionCallHandler::MakeArgumentLuaObjects(sol::state_view state, 
											   std::span<const uint64_t> parsedArgTypes, Args&& ...args)
{
	std::vector<sol::object> results;
	results.reserve(parsedArgTypes.size());

	std::bitset<sizeof...(Args)> claimed;
	size_t failedAtIdx = std::numeric_limits<size_t>::max();

	MakeArgumentLuaObjectsImpl<0>(state, parsedArgTypes, claimed, 0, results, 
		std::forward_as_tuple(std::forward<Args>(args)...), failedAtIdx);

	if (failedAtIdx < parsedArgTypes.size())
	{
		return MAKE_ERROR_FMT("Could not match argument at position '{}'", failedAtIdx);
	}

	return results;
}

template<AcceptedLuaTypeQualified...Args>
inline Result<std::vector<sol::object>> 
LuaFunctionCallHandler::MakeArgumentLuaObjects2(sol::state_view state, 
	std::span<const uint64_t> parsedArgTypes, Args&& ...args)
{
	std::vector<sol::object> results;
	results.reserve(parsedArgTypes.size());

	std::bitset<sizeof...(Args)> claimed;

	TRY(MakeArgumentLuaObjectsImpl2(state, parsedArgTypes, claimed, results,
		std::forward_as_tuple(std::forward<Args>(args)...)));

	return results;
}

template <AcceptedLuaTypeQualified...Args>
inline Result<Void> LuaFunctionCallHandler::CallLuaFunctionQualified(sol::function& fn, 
	std::span<const uint64_t> parsedArgTypes, Args&& ...args)
{
	assert(fn.valid());

	constexpr size_t argCount = sizeof...(Args);

	if (parsedArgTypes.size() > argCount)
	{
		return MAKE_ERROR_FMT("Lua function requires {} arguments, only {} arguments provided",
			parsedArgTypes.size(), argCount);
	}
	if (parsedArgTypes.size() < argCount)
	{
		LOG_WARNING_FMT("Lua function requires only {} arguments, {} arguments provided",
			parsedArgTypes.size(), argCount);
	}

	//TRY(MakeArgumentLuaObjects(fn.lua_state(), parsedArgTypes,
	//	std::forward<Args>(args)...), argObjects);
	TRY(MakeArgumentLuaObjects2(fn.lua_state(), parsedArgTypes,
		std::forward<Args>(args)...), argObjects);

	sol::protected_function_result result = fn(sol::as_args(argObjects));
	if (!result.valid())
	{
		sol::error err = result;
		return MAKE_ERROR_FMT("Failed to call lua function: '{}'", err.what());
	}

	return kVoid;
}

template <size_t I, size_t N, typename Tup>
inline void LuaFunctionCallHandler::MakeArgumentLuaObjectsImpl(sol::state_view state, 
	std::span<const uint64_t> parsedArgTypes, 
	std::bitset<N>& claimed, size_t parsedArgsCurrentIdx, 
	std::vector<sol::object>& results, Tup&& tup, size_t& failedAtIdx)
{
	if (failedAtIdx < parsedArgTypes.size() || parsedArgsCurrentIdx >= parsedArgTypes.size())
	{
		return;
	}

	const uint64_t parsedArgType = parsedArgTypes[parsedArgsCurrentIdx];

	if constexpr (I >= N)
	{
		// couldn't match arg, check if its a pointer that we can pass 'nil' to
		if (IsParsedArgPointer(parsedArgType))
		{
			LOG_WARNING_FMT("Could not match argument at position '{}', passed nil instead",
				parsedArgsCurrentIdx);

			results.emplace_back();
		}
		else
		{
			failedAtIdx = parsedArgsCurrentIdx;

			return;
		}

		if (parsedArgsCurrentIdx + 1 < parsedArgTypes.size())
		{
			MakeArgumentLuaObjectsImpl<0>(state, parsedArgTypes, claimed,
				parsedArgsCurrentIdx + 1, results, std::forward<Tup>(tup), failedAtIdx);
		}

		return;
	}
	else
	{
		using TypeAtI = std::tuple_element_t<I, std::remove_cvref_t<Tup>>;

		if (!claimed.test(I) && RawTypesMatch<TypeAtI>(parsedArgType))
		{
			// argument not already consumed && matches type

			auto obj = TryMakeArgumentLuaObject(state, 
				std::get<I>(std::forward<Tup>(tup)), parsedArgType);
			if (obj.valid())
			{
				results.emplace_back(obj);

				claimed.set(I);

				MakeArgumentLuaObjectsImpl<0>(state, parsedArgTypes, claimed,
					parsedArgsCurrentIdx + 1, results, std::forward<Tup>(tup), failedAtIdx);

				return;
			}
		}

		MakeArgumentLuaObjectsImpl<I + 1>(state, parsedArgTypes, claimed,
			parsedArgsCurrentIdx, results, std::forward<Tup>(tup), failedAtIdx);
	}
}

template <size_t N, typename Tup>
inline Result<Void> LuaFunctionCallHandler::MakeArgumentLuaObjectsImpl2(sol::state_view state, 
	std::span<const uint64_t> parsedArgTypes, std::bitset<N>& claimed,
	std::vector<sol::object>& results, Tup&& tup)
{
	for (size_t i = 0; i < parsedArgTypes.size(); ++i)
	{
		const auto parsedType = parsedArgTypes[i];

		const size_t argBestFixIdx = GetArgBestFitIndex(parsedType, claimed, std::forward<Tup>(tup));
		if (argBestFixIdx > N)
		{
			if (IsParsedArgPointer(parsedType))
			{
				LOG_WARNING_FMT("Could not match argument at position '{}', passed nil instead", i);

				results.emplace_back();
			}
			else
			{
				return MAKE_ERROR_FMT("Could not find suitable match for argument at position '{}'", i);
			}
		}
		else
		{
			const bool success = CallTryMakeArgumentLuaObject(state, parsedType, claimed,
				std::forward<Tup>(tup), results, argBestFixIdx);
			if (!success)
			{
				return MAKE_ERROR_FMT("Failed to create sol object for argument at position '{}'", i);
			}
		}
	}

	return kVoid;
}

template <size_t I, size_t N, typename Tup>
inline bool LuaFunctionCallHandler::CallTryMakeArgumentLuaObject(sol::state_view state,
	uint64_t parsedType, std::bitset<N>& claimed, Tup&& tup, std::vector<sol::object>& results,
	size_t requestedIdx)
{
	if constexpr (I >= N)
	{
		return false;
	}
	else
	{
		if (I == requestedIdx)
		{
			auto obj = TryMakeArgumentLuaObject(state, std::get<I>(std::forward<Tup>(tup)), parsedType);
			if (obj.valid())
			{
				results.emplace_back(obj);

				claimed.set(I);

				return true;
			}

			return false;
		}
		else
		{
			return CallTryMakeArgumentLuaObject<I + 1>(state, parsedType, claimed,
				std::forward<Tup>(tup), results, requestedIdx);
		}
	}
}

template <typename T>
inline bool LuaFunctionCallHandler::RawTypesMatch(uint64_t parsedType)
{
	const uint32_t parsedId = static_cast<uint32_t>(parsedType & 0xFFFFFFFFull);
	uint32_t valId = kInvalidLuaTypeId;

	if constexpr (IsNativeLuaType<raw_type_t<T>>())
	{
		valId = GetNativeLuaTypeId<raw_type_t<T>>();
	}
	else
	{
		valId = TypeInfo<raw_type_t<T>>::hash32;
	}

	return parsedId == valId;
}