#pragma once
#include <sol/sol.hpp>
#include "LuaNativeTypeIdUtils.h"
#include "../core/Dictionary.h"
#include "../core/TypeInfo.h"
#include "../core/TypeUtils.h"
#include "../core/FuncTraits.h"
#include "../core/SOAConcepts.h"

namespace detail {

template <typename> struct is_sol_property;

template <typename F, typename G>
struct is_sol_property<sol::property_wrapper<F, G>> : std::true_type {};

template <typename> struct is_sol_overload;

template <typename...Fs>
struct is_sol_overload<sol::overload_set<Fs...>> : std::true_type {};

template <typename T, typename Fn, typename = void>
struct is_lambda_based_method : std::false_type {};

template <typename T, typename Fn>
struct is_lambda_based_method<T, Fn, std::void_t<decltype(&Fn::operator())>>
{
	using traits = func_traits<Fn>;

	template <typename F, size_t I, typename U> 
	static constexpr bool arg_is_v = std::same_as<
		std::remove_cvref_t<typename traits::template arg_at<I>>, 
		std::remove_cvref_t<U>
	>;

	static constexpr bool value = [] {
		if constexpr (traits::argCount > 0)
		{
			if constexpr (arg_is_v<Fn, 0, sol::this_state>)
			{
				if constexpr (traits::argCount > 1)
				{
					return arg_is_v<Fn, 1, T>;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return arg_is_v<Fn, 0, T>;
			}
		}
		else
		{
			return false;
		}
	}();
};

} // detail

template <typename V> concept SomeSolProperty =
detail::is_sol_property<std::remove_cvref_t<V>>::value;

template <typename V> concept SomeSolOverload =
detail::is_sol_overload<std::remove_cvref_t<V>>::value;

template <typename V> concept ConvertibleToStringView = 
std::convertible_to<std::remove_cvref_t<V>, std::string_view>;

template <typename T, typename V> concept SomeLambdaBasedMethod = 
detail::is_lambda_based_method<T, V>::value;

enum class LuaUsertypeMemberKind
{
	Field,
	Method,
	Property,
	Overload
};

template <typename T, typename V>
inline constexpr LuaUsertypeMemberKind GetLuaUsertypeMemberKind()
{
	using ObjType = std::remove_cvref_t<T>;
	using ValueType = std::remove_cvref_t<V>;

	if constexpr (std::is_member_object_pointer_v<ValueType>)
	{
		return LuaUsertypeMemberKind::Field;
	}
	else if constexpr (std::is_member_function_pointer_v<ValueType>)
	{
		return LuaUsertypeMemberKind::Method;
	}
	else if constexpr (SomeSolProperty<ValueType>)
	{
		return LuaUsertypeMemberKind::Property;
	}
	else if constexpr (SomeLambdaBasedMethod<ObjType, ValueType>)
	{
		return LuaUsertypeMemberKind::Method;
	}
	else if constexpr (SomeSolOverload<ValueType>)
	{
		return LuaUsertypeMemberKind::Overload;
	}
	else
	{
		return LuaUsertypeMemberKind::Field;
	}
}

//template <typename V>
//inline constexpr uint32_t TryGetLuaUsertypeFieldReturnType()
//{
//	using ValueType = std::remove_cvref_t<V>;
//
//	if constexpr (std::is_member_object_pointer_v<ValueType>)
//	{
//		using MemberType = std::remove_cvref_t<
//			typename member_ptr_traits<ValueType>::value_type>;
//
//		return TypeInfo<MemberType>::hash32;
//	}
//	else if constexpr (HasFuncTraits<V>)
//	{
//		using RetType = typename func_traits<std::remove_cvref_t<V>>::return_type;
//
//		if constexpr (IsNativeLuaType<RetType>())
//		{
//			return GetNativeLuaTypeId<RetType>();
//		}
//		else
//		{
//			return TypeInfo<RetType>::hash32;
//		}
//	}
//	else
//	{
//		return kInvalidLuaTypeId;
//	}
//}

namespace detail {

template <typename T, typename...Args> struct parse_lua_usertype_members;

template <typename T>
struct parse_lua_usertype_members<T>
{
	static void call(std::vector<std::string>&,
					 std::vector<LuaUsertypeMemberKind>&) {}
};

template <typename T, typename K>
struct parse_lua_usertype_members<T, K>
{
	static void call(std::vector<std::string>&,
					 std::vector<LuaUsertypeMemberKind>&, const K&) 
	{
		static_assert(!ConvertibleToStringView<K>);
	}
};

template <typename T, typename K, typename V, typename...Rest>
struct parse_lua_usertype_members<T, K, V, Rest...>
{
	static void call(std::vector<std::string>& fieldNames,
		std::vector<LuaUsertypeMemberKind>& memberKinds,
		const K& k, const V& v, const Rest&...rest)
	{
		if constexpr (ConvertibleToStringView<K>)
		{
			static_assert(!ConvertibleToStringView<V>);

			fieldNames.emplace_back(k);
			memberKinds.emplace_back(GetLuaUsertypeMemberKind<T, V>());
		}

		if constexpr (ConvertibleToStringView<V>)
		{
			parse_lua_usertype_members<T, V, Rest...>::call(
				fieldNames, memberKinds, v, rest...);
		}
		else
		{
			parse_lua_usertype_members<T, Rest...>::call(
				fieldNames, memberKinds, rest...);
		}
	}
};

} // detail

class ParsedLuaUserTypeInfo
{
public:
	struct DataIndex
	{
		size_t value;

		constexpr bool IsValid() const noexcept
		{
			return value != std::numeric_limits<size_t>::max();
		}
		constexpr operator size_t() const noexcept
		{
			return value;
		}
	};

	ParsedLuaUserTypeInfo() = default;
	~ParsedLuaUserTypeInfo() = default;
	ParsedLuaUserTypeInfo(const ParsedLuaUserTypeInfo&) = delete;
	ParsedLuaUserTypeInfo& operator=(const ParsedLuaUserTypeInfo&) = delete;
	ParsedLuaUserTypeInfo(ParsedLuaUserTypeInfo&& other) noexcept :
		userTypeNames_(std::move(other.userTypeNames_)),
		userTypeIds_(std::move(other.userTypeIds_)),
		userTypeFieldNames_(std::move(other.userTypeFieldNames_)),
		userTypeMemberKinds_(std::move(other.userTypeMemberKinds_))
	{
		other.userTypeNameToDataIndex_.clear();
		other.userTypeIdToDataIndex_.clear();

		Commit();
	}
	ParsedLuaUserTypeInfo& operator=(ParsedLuaUserTypeInfo&& other) noexcept
	{
		if (this == &other) { return *this; }

		userTypeNames_ = std::move(other.userTypeNames_);
		userTypeIds_ = std::move(other.userTypeIds_);
		userTypeFieldNames_ = std::move(other.userTypeFieldNames_);
		userTypeMemberKinds_ = std::move(other.userTypeMemberKinds_);

		other.userTypeNameToDataIndex_.clear();
		other.userTypeIdToDataIndex_.clear();

		Commit();

		return *this;
	}

	DataIndex GetDataIndex(std::string_view userTypeName) const
	{
		auto it = userTypeNameToDataIndex_.find(userTypeName);

		return (it != userTypeNameToDataIndex_.end())
			? DataIndex{ it->second }
			: DataIndex{ std::numeric_limits<size_t>::max() };
	}

	const std::vector<std::string>&
	GetUserTypeNames() const { return userTypeNames_; }

	const std::vector<uint32_t>&
	GetUserTypeIds() const { return userTypeIds_; }

	const std::vector<std::vector<std::string>>&
	GetUserTypeFieldNames() const { return userTypeFieldNames_; }

	const std::vector<std::vector<LuaUsertypeMemberKind>>&
	GetUserTypeMemberKinds() const { return userTypeMemberKinds_; }

	template <typename T> requires std::same_as<raw_type_t<T>, T>
	DataIndex GetDataIndex() const
	{
		auto it = userTypeIdToDataIndex_.find(TypeInfo<T>::hash32);

		return (it != userTypeIdToDataIndex_.end())
			? DataIndex{ it->second }
			: DataIndex{ std::numeric_limits<size_t>::max() };
	}

	void Commit()
	{
		userTypeNameToDataIndex_.clear();
		userTypeIdToDataIndex_.clear();

		assert(userTypeNames_.size() == userTypeIds_.size());

		for (size_t i = 0; i < userTypeNames_.size(); ++i)
		{
			[[maybe_unused]] auto [_1, nameInserted] =
				userTypeNameToDataIndex_.try_emplace(userTypeNames_[i], i);
			assert(nameInserted);

			[[maybe_unused]] auto [_2, typeIdInserted] =
				userTypeIdToDataIndex_.try_emplace(userTypeIds_[i], i);
			assert(typeIdInserted);
		}

		committed_ = true;
	}

	template <typename T, typename...Args>
		requires (std::same_as<raw_type_t<T>, T> && 
				 (std::is_class_v<T> || std::is_enum_v<T>))
	void ParseUserTypeArgs(std::string_view name, const Args&...args)
	{
		committed_ = false;

		AddEntry();

		userTypeNames_.back() = std::string{ name };
		userTypeIds_.back() = TypeInfo<T>::hash32;

		auto& fieldNames = userTypeFieldNames_.back();
		auto& memberKinds = userTypeMemberKinds_.back();

		detail::parse_lua_usertype_members<T, Args...>::call(fieldNames, memberKinds, args...);
	}

	bool IsCommitted() const { return committed_; }

private:
	size_t AddEntry()
	{
		const size_t idx = userTypeNames_.size();

		userTypeNames_.emplace_back();
		userTypeIds_.emplace_back();
		userTypeFieldNames_.emplace_back();
		userTypeMemberKinds_.emplace_back();

		return idx;
	}

	std::vector<std::string> userTypeNames_;
	std::vector<uint32_t> userTypeIds_;
	std::vector<std::vector<std::string>> userTypeFieldNames_;
	std::vector<std::vector<LuaUsertypeMemberKind>> userTypeMemberKinds_;
	//std::vector<uint32_t> userTypeFieldRawReturnTypeIds_;

	std::unordered_map<std::string_view, size_t> userTypeNameToDataIndex_;
	std::unordered_map<uint32_t, size_t> userTypeIdToDataIndex_;

	bool committed_ = false;
};