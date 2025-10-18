#pragma once
#include "../core/TypeUtils.h"
#include <sol/sol.hpp>
#include <lua.hpp>

//
template <typename...Ts>
using Dependencies = TypeList<Ts...>;

template <typename...Ts>
concept ValidLuaDependenciesMacroArg = sizeof...(Ts) == 0 ||
(sizeof...(Ts) == 1 && is_type_list_v<type_at_index_t<0, TypeList<Ts...>>>);

namespace detail {
template <size_t, typename...Ts> 
struct get_resolved_dependencies_arg_impl
{
	using type = std::tuple_element_t<0, std::tuple<Ts...>>;
};

template <typename...Ts>
struct get_resolved_dependencies_arg_impl<0, Ts...>
{
	using type = TypeList<>;
};

template <typename...Ts>
struct get_resolved_dependencies_arg
{
	using type = typename get_resolved_dependencies_arg_impl<sizeof...(Ts), Ts...>::type;
};
}

//
template <typename T>
struct lua_user_type_dependencies;

template <typename T>
struct lua_user_type_name;

//
template <typename T>
struct LuaUserType;

namespace detail {
template <typename T>
struct lua_helper
{
	explicit lua_helper(sol::state& l) : lua_(l) {}

	template <typename...Args>
	void def_type(Args&&...args) 
	{
		lua_.new_usertype<T>(lua_user_type_name<T>::value, std::forward<Args>(args)...);
	}

	template <typename...Args> requires std::is_enum_v<T>
	void def_enum(Args&&...args) 
	{
		lua_.new_enum(lua_user_type_name<T>::value, std::forward<Args>(args)...);
	}

private:
	sol::state& lua_;
};
}

template <typename T>
struct lua_user_type_register { 
	static void fn(detail::lua_helper<T>&&) { static_assert(false); }
};

template <typename T>
concept SomeLuaUserType = requires() {
	std::same_as<decltype(lua_user_type_name<T>::value), std::string_view>;
	lua_user_type_name<T>::value.size() >= 1;
	std::is_invocable_r_v<void, decltype(lua_user_type_register<T>::fn), detail::lua_helper<T>&&>;
};

template <typename T> 
struct LuaUserType
{
	static constexpr std::string_view name = lua_user_type_name<T>::value;
	using Dependencies = lua_user_type_dependencies<T>::type;

	struct RegisterDeps
	{
		template <typename...Us>
		static void Apply(sol::state& lua)
		{
			((LuaUserType<Us>::Register(lua)), ...);
		}
	};

	static void Register(sol::state& lua)
	{
		Dependencies::template Apply<RegisterDeps>(lua);

		lua_user_type_register<T>::template fn(detail::lua_helper<T>(lua));
	}
};

#define DEF_LUA_USERTYPE(userType, ...)				   \
template <> struct lua_user_type_dependencies<userType> {  \
	using type = typename detail::get_resolved_dependencies_arg<__VA_ARGS__>::type; \
};														   \
template <> struct lua_user_type_name<userType> {		   \
	static constexpr std::string_view value = #userType;   \
};														   \
template <> struct lua_user_type_register<userType> {	   \
	static void fn(detail::lua_helper<userType>&&);        \
};														   \
inline void lua_user_type_register<userType>::fn(detail::lua_helper<userType>&& lua)

