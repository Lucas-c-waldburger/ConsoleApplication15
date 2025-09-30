#pragma once
#include "../core/TypeUtils.h"
#include <SDL_rect.h>

//
template <typename...Ts>
struct dependencies_impl { using type = TypeList<Ts...>; };

template <typename...Ts>
struct dependencies_impl<dependencies_impl<Ts...>>
{
	using type = TypeList<Ts...>;
};

template <typename...Ts>
using Dependencies = typename dependencies_impl<Ts...>::type;

//
template <typename T>
struct lua_user_type_dependencies { using type = Dependencies<>; };

template <typename T>
struct lua_user_type_name { static constexpr std::string_view value = ""; };

//
template <typename T>
struct LuaUserType;

namespace detail {
template <typename T>
struct lua_helper
{
	explicit lua_helper(sol::state& l) : lua(l) {}

	template <typename...Args>
	void new_usertype(Args&&...args) {
		lua.new_usertype<T>(lua_user_type_name<T>::value, std::forward<Args>(args)...);
	}

	template <typename...Args> requires std::is_enum_v<T>
	void new_enum(Args&&...args) {
		lua.new_enum(lua_user_type_name<T>::value, std::forward<Args>(args)...);
	}

	sol::state& lua;
};
}

template <typename T>
struct lua_user_type_register { 
	static void fn(detail::lua_helper<T>&&) { static_assert(false); }
};

template <typename T>
concept SomeLuaUserType = requires() {
	std::same_as<decltype(lua_user_type_name<T>::value), std::string_view>;
	lua_user_type_name<T>::value.size() > 1;
	std::is_invocable_r_v<void, decltype(lua_user_type_register<T>::fn), detail::lua_helper<T>&&>;
};

template <typename T> 
struct LuaUserType
{
	static constexpr std::string_view name = lua_user_type_name<T>::value;
	using Dependencies = lua_user_type_dependencies<T>::type;

	static void Register(sol::state& lua)
	{
		if (!lua[name].valid())
		{
			Dependencies::Apply([&lua]<typename U>{
				LuaUserType<U>::Register(lua); 
			});

			lua_user_type_register<T>::fn(detail::lua_helper<T>(lua));
		}
	}
};

#define DEF_LUA_USER_TYPE(userType, ...)				  \
template <> struct lua_user_type_dependencies<userType> { \
	using type = Dependencies<__VA_ARGS__>;				  \
};														  \
template <> struct lua_user_type_name<userType> {		  \
	static constexpr std::string_view value = #userType;   \
};														  \
template <> struct lua_user_type_register<userType> {	  \
	static void fn(detail::lua_helper<userType>&&); \
};														  \
void lua_user_type_register<userType>::fn(detail::lua_helper<userType>&& lua)

struct MyPoint { int x; int y; };

DEF_LUA_USER_TYPE(MyPoint) {
	lua.new_usertype("x", &MyPoint::x);
}

//template <> struct lua_user_type_dependencies<MyPoint> {
//	using type = Dependencies<>;
//}; template <> struct lua_user_type_name<MyPoint> {
//	static constexpr std::string_view name = "MyPoint";
//}; 
//template <> struct lua_user_type_register<MyPoint> {
//	static void fn(detail::lua_helper<MyPoint>&);
//}; 
//void lua_user_type_register<MyPoint>::fn(detail::lua_helper<MyPoint>& lua) {
//	lua.new_usertype("x", &MyPoint::x);
//}

//static_assert(SomeLuaUserType<MyPoint>);

//template <typename T>
//struct LuaUserTypeArgGenerator;
//
//template <typename T>
//struct LuaUserType : public LuaUserTypeArgGenerator<T>
//{
//	static auto GetArgs() { 
//		return LuaUserTypeArgGenerator<T>::GetArgs(); 
//	}
//};
//

//template <typename MemFn>
//inline std::pair<const char*, MemFn> MakeLuaArgPair(const char* nm, MemFn&& fn) {
//	return std::make_pair(nm, std::forward<Fn>(fn));
//}
//
//template <typename... Pairs>
//constexpr auto tuple_from_pairs(Pairs&&... pairs) {
//	return std::tuple_cat(std::make_tuple(pairs.first, pairs.second)...);
//}
//
//
//
//template <typename T>
//struct LuaUserType;
//
//template <typename...Args>
//struct LuaUserTypeArgs;
//
//template <typename...Names, typename...MemPtrs>
//struct LuaUserTypeArgs<std::pair<Names, MemPtrs>...>
//{
//
//};
//
//template <typename T, typename...Names, typename...MemPtrs>
//struct LuaUserType<std::tuple<T, const char*, std::tuple<std::pair<Names, MemPtrs>...>>>
//{
//	template <typename...Pairs>
//	static void Register(sol::state& lua, Pairs&&...pairs) {
//		auto pairTup = std::apply(tuple_from_pairs, std::forward<Pairs>(pairs)...);
//
//	}
//};

//template <typename UserType, typename...Args>
//using LuaNewUserTypeSig = sol::usertype<UserType>(*)(Args&&...);
//
//namespace detail {
//template <typename...Ts>
//inline void register_lua_user_type_dependencies(sol::state& lua)
//{
//	((RegisterLuaUserType<Ts>(lua)), ...);
//}
//template <typename UserType, const char* nm, typename Tup>
//inline void apply_lua_user_type_tuple(sol::state& lua, Tup&& tup)
//{
//	auto args = std::tuple_cat(std::make_tuple(nm), std::forward<Tup>(tup));
//
//	std::apply([&](auto&&... unpacked) { lua.new_usertype<UserType>(unpacked...); }, args);
//}
//}
//
//#define DEF_LUA_TYPE(type, ...) \
//	namespace detail { \
//		static constexpr char kLuaUserTypeName_##type[] = #type; \
//	} \
//	template <> inline void RegisterLuaUserType<type>(sol::state& lua) \
//	{ \
//		detail::register_lua_user_type_dependencies<__VA_ARGS__>(lua); \
//		if (!lua[#type].valid()) \
//		{ \
//			detail::apply_lua_user_type_tuple< \
//				type, detail::kLuaUserTypeName_##type>( \
//					lua, lua_type_##type##_arg_tuple); \
//		} \
//	} \
//	inline constexpr std::tuple lua_type_##type##_arg_tuple=

//inline void register_lua_type_##type##_impl(sol::state& lua) {
//	\
//		ApplyLuaNewUserTypeTup<type>(lua_type_##type##_arg_tuple); \
//} \


//#define DEF_LUA_TYPE(type, ...) \
//	template <> inline void RegisterLuaUserType<type>(sol::state& lua) \
//	{ \
//		RegisterLuaUserTypeDependencies<__VA_ARGS__>(lua); \
//		if (!lua[#type].valid()) \
//		{ \
//			ExpandNewUserTypeArgs<type>(lua, #type, __VA_ARGS__); \
//		} \
//	} \

