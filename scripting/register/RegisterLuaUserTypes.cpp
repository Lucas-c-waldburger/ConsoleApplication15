#include "RegisterLuaUserTypes.h"
#include "UserTypeIncludes.h"
#include "../../components/ComponentConcepts.h"

namespace {

template <typename T>
concept HasLuaUserTypeRegistration = requires(LuaStateManager & m) {
	{ register_lua_usertype<T>::call(m) } -> std::same_as<void>;
};

template <typename T> struct ComponentLuaUserTypePred :
	std::bool_constant<(HasLuaUserTypeRegistration<T> && public_mutable_component_v<T>)> {
};

namespace detail {

template <typename> struct register_component_ids_on_table;

template <template <typename...> class TList, typename...Ts>
struct register_component_ids_on_table<TList<Ts...>> {
	static void call(sol::table& cmpIdTable) {
		static constexpr auto impl = []<typename T>(sol::table & cmpIdTable) {
			cmpIdTable[register_lua_usertype<T>::name] = MakeComponentId<T, TList<Ts...>>();
		};
		((impl.template operator()<Ts> (cmpIdTable)), ...);
	}
};

} // detail

template <HasLuaUserTypeRegistration T>
inline void RegisterLuaUserType(LuaStateManager& m)
{
	register_lua_usertype<T>::call(m);
}

void RegisterComponentIds(LuaStateManager& m)
{
	RegisterLuaUserType<ComponentId>(m);

	sol::table cmpIdTable = m.Data().create_named_table("ComponentId");

	detail::register_component_ids_on_table<filter_types_t<
		ComponentTypeList, ComponentLuaUserTypePred>>::call(cmpIdTable);
}

void RegisterCoreUserTypes(LuaStateManager& m)
{
	RegisterLuaUserType<Dimensions<int>>(m);
	RegisterLuaUserType<Dimensions<float>>(m);

	RegisterLuaUserType<SDL_Point>(m);
	RegisterLuaUserType<SDL_FPoint>(m);
	RegisterLuaUserType<SDL_Rect>(m);
	RegisterLuaUserType<SDL_FRect>(m);
	RegisterLuaUserType<SDL_Color>(m);
}

void RegisterBasicComponentUserTypes(LuaStateManager& m)
{
	RegisterLuaUserType<Transform>(m);
	RegisterLuaUserType<Name>(m);
	RegisterLuaUserType<CameraTarget>(m);

	RegisterLuaUserType<Timer::Flag>(m);
	RegisterLuaUserType<Timer>(m);
}

void RegisterPhysicsUserTypes(LuaStateManager& m)
{
	RegisterLuaUserType<Handle<B2Body>>(m);
	RegisterLuaUserType<Handle<B2Shape>>(m);
}

void RegisterInputUserTypes(LuaStateManager& m)
{
	RegisterLuaUserType<InputState>(m);
	RegisterLuaUserType<GameControllerInputSource>(m);
	RegisterLuaUserType<GameControllerInputFieldValue>(m);
	RegisterLuaUserType<GameControllerInputField>(m);
	RegisterLuaUserType<GameControllerState>(m);
}

void RegisterRenderableUserTypes(LuaStateManager& m)
{
	RegisterLuaUserType<Handle<TextureResource>>(m);

	RegisterLuaUserType<AtlasPlot>(m);
	RegisterLuaUserType<SDL_RendererFlip>(m);
	RegisterLuaUserType<SDL_BlendMode>(m);
	RegisterLuaUserType<RGB>(m);
	RegisterLuaUserType<TextureMods>(m);
	RegisterLuaUserType<DebugDraw>(m);
	RegisterLuaUserType<DebugDrawSet>(m);
	RegisterLuaUserType<Anchor>(m);
	RegisterLuaUserType<RenderProfile::Anchors>(m);
	RegisterLuaUserType<RenderProfile>(m);

	RegisterLuaUserType<TextAlign>(m);
	RegisterLuaUserType<Glyph>(m);
	RegisterLuaUserType<GlyphCacheData>(m);
	RegisterLuaUserType<TextFormatting>(m);
	RegisterLuaUserType<GlyphTextWriter>(m);
	RegisterLuaUserType<TextRenderableComponent>(m);

	RegisterLuaUserType<Sprite>(m);
	RegisterLuaUserType<SpriteRenderableComponent>(m);
	RegisterLuaUserType<NeedsAnimationUpdate>(m);
	RegisterLuaUserType<SpriteSeriesIndex>(m);
	RegisterLuaUserType<SpriteAnimationComponent>(m);
}

void RegisterAudioUserTypes(LuaStateManager& m)
{
	RegisterLuaUserType<Handle<Audio>>(m);
	RegisterLuaUserType<AudioInstanceID>(m);
	RegisterLuaUserType<AudioFadeMs>(m);
	RegisterLuaUserType<HandedPair<uint8_t>>(m);
	RegisterLuaUserType<AudioSpatialData>(m);
	RegisterLuaUserType<AudioChannelSettings>(m);
	RegisterLuaUserType<AudioUpdateSettings>(m);
	RegisterLuaUserType<AudioPlayCommand>(m);
	RegisterLuaUserType<AudioStatus>(m);
	RegisterLuaUserType<NewAudioRequest>(m);
	RegisterLuaUserType<ActiveAudio>(m);
	RegisterLuaUserType<AudioBank>(m);
}

void RegisterEventUserTypes(LuaStateManager& m)
{
	RegisterLuaUserType<CollisionData>(m);
	RegisterLuaUserType<events::ContactCollisionBegin>(m);
	RegisterLuaUserType<events::ContactCollisionEnd>(m);
	RegisterLuaUserType<events::SensorCollisionBegin>(m);
	RegisterLuaUserType<events::SensorCollisionEnd>(m);
	RegisterLuaUserType<events::HitCollision>(m);

	RegisterLuaUserType<events::GameControllerConnected>(m);
	RegisterLuaUserType<events::GameControllerDisconnected>(m);
	RegisterLuaUserType<events::GameControllerInput>(m);

	RegisterLuaUserType<events::TimerFired>(m);
}

void RegisterEngineObjectUserTypes(LuaStateManager& m)
{
	RegisterLuaUserType<Camera>(m);
	RegisterLuaUserType<TextureRepository>(m);
}

} // unnamed

void RegisterLuaUserTypes(LuaStateManager& m)
{
	RegisterComponentIds(m);
	RegisterCoreUserTypes(m);
	RegisterBasicComponentUserTypes(m);
	RegisterPhysicsUserTypes(m);
	RegisterInputUserTypes(m);
	RegisterRenderableUserTypes(m);
	RegisterAudioUserTypes(m);
	RegisterEventUserTypes(m);
	RegisterLuaUserType<Entity>(m);
	RegisterEngineObjectUserTypes(m);
}

