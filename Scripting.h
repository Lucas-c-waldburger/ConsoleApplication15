#pragma once
#include <sol/sol.hpp>
#include <lua.hpp>
#include <filesystem>
#include "Core.h"
#include "Components.h"
#include <SDL.h>

//template <typename...Ts>
//struct LuaVariantWrapper
//{
//	std::string ToString() const 
//	{
//		return std::visit([](const auto& val) -> std::string 
//			{
//			if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::string>)
//				return val;
//			else
//				return std::to_string(val);
//			}, value);
//	}
//
//	std::variant<Ts...> value;
//};

template <typename T>
class LuaProperty
{
public:
	using ValueType = std::remove_cvref_t<T>;

	const ValueType& GetValue() { return value_; }
	void SetValue(ValueType newVal) { value_ = std::move(newVal); }

	static auto GetBinding() requires (!std::is_const_v<T>)
	{
		return sol::property(&LuaProperty<T>::GetValue, &LuaProperty<T>::SetValue);
	}
	static auto GetBinding() requires std::is_const_v<T>
	{
		return sol::property(&LuaProperty<T>::GetValue, nullptr);
	}

private:
	ValueType value_;
};



template <typename>
static void RegisterLuaUserType(sol::state& lua);
// specialized for different types

class Lua
{
public:
	enum class ScriptType 
	{ 
		Unknown, 
		String, 
		File 
	};

	struct ScriptInfo
	{		
		std::string name;
		ScriptType scriptType = ScriptType::Unknown;
	};

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

	Result<Void> Run()
	{
		sol::protected_function_result ret;

		switch (scriptInfo_.scriptType)
		{
		case ScriptType::String:
			ret = lua_.script(scriptInfo_.name, sol::script_pass_on_error);
			break;
		case ScriptType::File:
			ret = lua_.script_file(scriptInfo_.name, sol::script_pass_on_error);
			break;
		case ScriptType::Unknown: default:
			return MAKE_ERROR("No script type specified");
		}

		if (!ret.valid())
		{
			sol::error err = ret;

			return MAKE_ERROR(std::string{err.what()});
		}
		return Void{};
	}

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

// SDL TYPES
template <> static void RegisterLuaUserType<SDL_FPoint>(sol::state& lua)
{
	try {
		lua.new_usertype<SDL_FPoint>("SDL_FPoint", "x", &SDL_FPoint::x, "y", &SDL_FPoint::y);
	} catch (sol::error& e) { std::cout << e.what() << '\n'; }
}
template <> static void RegisterLuaUserType<SDL_Point>(sol::state& lua)
{
	lua.new_usertype<SDL_Point>("SDL_Point", "x", &SDL_Point::x, "y", &SDL_Point::y);
}
template <> static void RegisterLuaUserType<SDL_Rect>(sol::state& lua)
{
	lua.new_usertype<SDL_Rect>("SDL_Rect", "x", &SDL_Rect::x, "y", &SDL_Rect::y,
										   "w", &SDL_Rect::w, "h", &SDL_Rect::h);
}
template <> static void RegisterLuaUserType<SDL_FRect>(sol::state& lua)
{
	lua.new_usertype<SDL_FRect>("SDL_FRect", "x", &SDL_FRect::x, "y", &SDL_FRect::y,
					 						 "w", &SDL_FRect::w, "h", &SDL_FRect::h);
}

// my core types
template <> static void RegisterLuaUserType<Dimensions<float>>(sol::state& lua)
{
	try {
		lua.new_usertype<Dimensions<float>>("Dimensions<float>", 
			"w", &Dimensions<float>::w, "h", &Dimensions<float>::h);
	} catch (sol::error& e) { std::cout << e.what() << '\n'; }
}


// HANDLES
template <> static void RegisterLuaUserType<Handle<GlyphAtlas>>(sol::state& lua)
{
	lua.new_usertype<Handle<GlyphAtlas>>("Handle<GlyphAtlas>",
		sol::meta_function::equal_to, &Handle<GlyphAtlas>::operator==);

	lua["Handle<GlyphAtlas>"]["__ne"] = [](const Handle<GlyphAtlas>& lhs, const Handle<GlyphAtlas>& rhs) {
		return lhs != rhs;
	};
}
template <> static void RegisterLuaUserType<Handle<SpriteSeriesAtlas>>(sol::state& lua)
{
	lua.new_usertype<Handle<SpriteSeriesAtlas>>("Handle<SpriteSeriesAtlas>",
		sol::meta_function::equal_to, &Handle<SpriteSeriesAtlas>::operator==);

	lua["Handle<SpriteSeriesAtlas>"]["__ne"] = 
		[](const Handle<SpriteSeriesAtlas>& lhs, const Handle<SpriteSeriesAtlas>& rhs) {
			return lhs != rhs;
	};
}\\


// COMPONENTS
template <> static void RegisterLuaUserType<Physics>(sol::state& lua)
{
	lua.new_usertype<Physics>("Physics", "velocity", &Physics::velocity,
		"acceleration", &Physics::acceleration, "mass", &Physics::mass, "drag", &Physics::drag);
}
template <> static void RegisterLuaUserType<Spatial>(sol::state& lua)
{
	try {
		lua.new_usertype<Spatial>("Spatial",
			"position", &Spatial::position,
			"dimensions", &Spatial::dimensions);
	} catch (sol::error& e) { std::cout << e.what() << '\n'; }
}
template <> static void RegisterLuaUserType<Transform>(sol::state& lua)
{
	lua.new_usertype<Transform>("Transform", "scale", &Transform::scale,
		"rotation", &Transform::rotation, "offset", &Transform::offset);
}

template <> static void RegisterLuaUserType<Parent>(sol::state& lua)
{
	lua.new_usertype<Parent>("Parent", "parentEntity", &Parent::parentEntity);
}
template <> static void RegisterLuaUserType<Children>(sol::state& lua)
{
	lua.new_usertype<Children>("Children", "childEntities", &Children::childEntities);
}

// RENDERABLE
template <> static void RegisterLuaUserType<Renderable::Text::Alignment>(sol::state& lua)
{
	using Alignment = Renderable::Text::Alignment;
	lua.new_enum("Renderable::Text::Alignment", "Left", Alignment::Left, 
		"Right", Alignment::Right, "Center", Alignment::Center);
}
template <> static void RegisterLuaUserType<Renderable::Text>(sol::state& lua)
{
	using Text = Renderable::Text;
	lua.new_usertype<Text>("Renderable::Text", "sourceAtlas", &Renderable::Text::sourceAtlas,
		"text", &Text::text, "align", &Text::align, "scaleToFit", &Text::scaleToFit);
}
template <> static void RegisterLuaUserType<Renderable::Sprite>(sol::state& lua)
{
	using Sprite = Renderable::Sprite;
	lua.new_usertype<Sprite>("Renderable::Sprite", "sourceAtlas", &Renderable::Sprite::sourceAtlas,
		"seriesName", &Sprite::seriesName, "currentIndex", &Sprite::currentIndex);
}


//// LUA KEY
//template <typename T> struct LuaKeyTemplate;
//template <typename T> static constexpr const char* LuaKey = LuaKeyTemplate<T>::value;
//
//template <> struct LuaKeyTemplate<Spatial> { static constexpr const char* value = };