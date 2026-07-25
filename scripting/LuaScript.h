#pragma once
#include "LuaUserType.h"
#include "LuaFunction.h"
#include "LuaTypesRegistry.h"

//namespace detail {

//template <typename T>
//struct lua_script_ret { using type = T; };
//
//template <>
//struct lua_script_ret<void> { using type = Void; };
//
//} // detail
//
//template <typename T>
//using lua_script_ret_t = typename detail::lua_script_ret<T>::type;
//
//class LuaState
//{
//public:
//	template <SomeLuaUserType...Ts>
//	static LuaState Create()
//	{
//		LuaScript lua{};
//
//		lua.state_.open_libraries(sol::lib::base);
//
//		((LuaUserType<Ts>::Register(lua.state_)), ...);
//
//		return lua;
//	}
//
//	template <typename Ret>
//	Result<lua_script_ret_t<Ret>> RunScript(std::string_view script)
//	{
//		sol::protected_function_result result = state_.script(script);
//		if (!result.valid())
//		{
//			sol::error err = result;
//			
//			return MAKE_ERROR(err.what());
//		}
//
//		if constexpr (std::same_as<lua_script_ret_t<Ret>, Void>)
//		{
//			return kVoid;
//		}
//		else
//		{
//			sol::object obj = result;
//			if (obj.is<raw_type_t<Ret>>())
//			{
//				return obj.as<Ret>();
//			}
//			else
//			{
//				return MAKE_ERROR("Script return ")
//			}
//		}
//	}
//
//	template <typename T>
//	decltype(auto) operator[](T&& key)
//	{
//		return state_[std::forward<T>(key)];
//	}
//
//private:
//	sol::state state_;
//};

template <SomeLuaUserType...Ts>
inline sol::state MakeLuaState()
{
	sol::state luaState{};

	luaState.open_libraries(sol::lib::base);

	((LuaUserType<Ts>::Register(luaState)), ...);

	return luaState;
}

class LuaScript
{
public:
	LuaScript() = default;
	~LuaScript() = default;

	LuaScript(const LuaScript&) = delete;
	LuaScript& operator=(const LuaScript&) = delete;

	LuaScript(LuaScript&& other) noexcept : state_(std::move(other.state_)) {}
	LuaScript& operator=(LuaScript&& other) noexcept
	{
		if (this != &other)
		{
			this->state_ = std::move(other.state_);
		}
		return *this;
	}

	template <SomeLuaUserType...Ts>
	static Result<LuaScript> Create(std::string_view script)
	{
		LuaScript lua{};

		lua.state_.open_libraries(sol::lib::base);

		((LuaUserType<Ts>::Register(lua.state_)), ...);

		sol::protected_function_result result = lua.state_.script(script);
		if (!result.valid())
		{
			sol::error err = result;
			return MAKE_ERROR(err.what());
		}

		return lua;
	}

	template <typename T>
	decltype(auto) operator[](T&& key)
	{
		return state_[std::forward<T>(key)];
	}

	LuaFunctionTable MakeFunctionTable()
	{
		return LuaFunctionTable{ state_ };
	}

private:
	sol::state state_;
};