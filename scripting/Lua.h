#pragma once
#include "LuaTypesRegistry.h"
#include "../core/Result.h"

class Lua
{
public:
	Lua() = default;
	~Lua() = default;

	Lua(const Lua&) = delete;
	Lua& operator=(const Lua&) = delete;

	Lua(Lua&& other) noexcept : lua_(std::move(other.lua_)), scriptInfo_(std::move(other.scriptInfo_)) {}
	Lua& operator=(Lua&& other) noexcept
	{
		if (this != &other)
		{
			this->lua_ = std::move(other.lua_);
			this->scriptInfo_ = std::move(other.scriptInfo_);
		}
		return *this;
	}

	template <typename...Ts>
	void RegisterUserTypes()
	{
		((RegisterLuaUserType<Ts>(lua_)), ...);
	}
	template <typename T>
	void RegisterUserTypeList()
	{
		RegisterUserTypeListDetail<T>::Call(lua_);
	}

	void SetScript(ScriptInfo scriptInfo) { scriptInfo_ = std::move(scriptInfo); }
	const ScriptInfo& GetScript() const { return scriptInfo_; }

	Result<Void> Run();

	sol::state& GetSol() { return lua_; }
	const sol::state& GetSol() const { return lua_; }

	template <typename T>
	auto operator[](T&& key)
	{
		return lua_[std::forward<T>(key)];
	}

	template <typename...Ts, typename...Libs>
	static Lua GetInstance(Libs&&...libs)
	{
		Lua luaInstance{};

		if constexpr (sizeof...(Libs) == 0)
		{
			luaInstance.lua_.open_libraries(sol::lib::base);
		}
		else
		{
			luaInstance.lua_.open_libraries(std::forward<Libs>(libs)...);
		}

		luaInstance.RegisterUserTypes<Ts...>();

		return luaInstance;
	}

private:
	template <typename> struct RegisterUserTypeListDetail;
	template <typename...Ts> struct RegisterUserTypeListDetail<TypeList<Ts...>>
	{
		static void Call(sol::state& lua) { ((RegisterLuaUserType<Ts>(lua)), ...); }
	};

	sol::state lua_;
	ScriptInfo scriptInfo_;
};