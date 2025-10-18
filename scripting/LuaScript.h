#pragma once
#include "LuaUserType.h"
#include "LuaFunction.h"
#include "LuaTypesRegistry.h"


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