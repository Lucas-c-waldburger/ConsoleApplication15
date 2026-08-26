#include "FixtureLua.h"
#include "Fixtures.h"
#include "../scripting/user_types/LuaUserTypeIncludes.h"

namespace {

static constexpr std::string_view kEcsObjectName = "ecs";
static constexpr std::string_view kComponentIdObjectName = "ComponentId";
static constexpr std::string_view kFixtureObjectName = "fixture";
static constexpr std::string_view kCameraObjectName = "camera";
static constexpr std::string_view kTextureRepositoryObjectName = "textures";
static constexpr std::string_view kAudioBankObjectName = "audio";

struct GetUserTypeWrapper
{
	explicit operator bool() const { return userTypeTable.valid(); }
	bool operator!() const { return !userTypeTable.valid(); }
	auto operator[](std::string_view name) { return userTypeTable[name]; }

	sol::table userTypeTable;
};

struct StateWrapper
{
	template <SomeLuaUserType T>
	GetUserTypeWrapper GetUserType() 
	{ 
		return { .userTypeTable = state[lua_user_type_name<T>::value] };
	}

	sol::state_view state;
};

namespace detail {

template <typename> struct register_component_ids_on_table;
template <template <typename...> class TList, typename...Ts>
struct register_component_ids_on_table<TList<Ts...>> {
	static void call(sol::table& cmpIdTable) {
		static constexpr auto impl = []<typename T>(sol::table & cmpIdTable) {
			cmpIdTable[lua_user_type_name<T>::value] = MakeComponentId<T, LuaComponentTypeList>();
		};
		((impl.template operator()<Ts>(cmpIdTable)), ...);
	}
};

} // detail

void RegisterComponentIdsOnTable(sol::table& cmpIdTable)
{
	detail::register_component_ids_on_table<LuaComponentTypeList>::call(cmpIdTable);
}

} // unnamed

void ExtendSpriteRenderable(StateWrapper& state, SpriteAtlas& atlas)
{
	if (auto userType = state.GetUserType<SpriteRenderableComponent>())
	{
		userType["setSprite"] = [&atlas](SpriteRenderableComponent& r, std::string name) {
			r.sprite = atlas.GetSprite(name);
			if (!r.sprite.resourceHandle.IsValid())
			{
				LOG_ERROR_FMT("sprite '{}' not found", name);
			}
		};
	}
}

void ExtendTextRenderable(StateWrapper& state, FontAtlas& atlas)
{
	if (auto userType = state.GetUserType<TextRenderableComponent>())
	{
		userType["setFont"] = [&atlas](TextRenderableComponent& r, std::string name) {
			std::string tempText = std::move(r.writer.text);
			r.writer = atlas.GetTextWriter(name);
			r.writer.text = std::move(tempText);

			if (!r.writer.resourceHandle.IsValid())
			{
				LOG_ERROR_FMT("Font '{}' not found", name);
			}
		};
	}
}

void ExtendAudioRequest(StateWrapper& state, AudioBank& bank)
{
	if (auto userType = state.GetUserType<NewAudioRequest>())
	{
		userType["setAudio"] = [&bank](NewAudioRequest& r, std::string name) {
			r.audioHandle = bank.GetAudio(name);
			if (!r.audioHandle.IsValid())
			{
				LOG_ERROR_FMT("Audio '{}' not found");
			}
		};
	}
}

void AddComponentIdTable(sol::state_view state)
{
	sol::table cmpIdLua = state.create_named_table(kComponentIdObjectName);

	RegisterComponentIdsOnTable(cmpIdLua);
}

void AddEcsTable(sol::state_view state)
{
	static constexpr auto getEntitiesWith = +[](sol::variadic_args args) {
		ComponentSignature sig = 0;

		for (const sol::object& arg : args)
		{
			if (!arg.is<ComponentId>())
			{
				throw sol::error("getEntitiesWith expects ComponentId arguments");
			}

			sig |= static_cast<ComponentSignature>(arg.as<ComponentId>().bit);
		}

		return sol::as_table(ECS::GetAllEntitiesWithSignature(sig));
	};

	sol::table ecsLua = state.create_named_table(kEcsObjectName);

	ecsLua["createEntity"] = [] -> Entity { return ECS::CreateEntity(); };
	ecsLua["getEntityById"] = [](Entity_t id) { return ECS::GetEntityByID(id); };
	ecsLua["getEntitiesWith"] = getEntitiesWith;
}

void SetUpFixtureLuaState(SceneFixture& fx)
{
	if (!fx.IsSystemRegistered<ScriptSystem>())
	{
		return;
	}

	auto& scriptSys = fx.GetSystem<ScriptSystem>();
	scriptSys.GetState().InitWithEngineTypes<
		Entity, 
		TextureRepository, 
		Camera, 
		AudioBank,
		EventLuaUserTypeList
	>();

	auto& state = scriptSys.GetState();

	AddComponentIdTable(state.Data());
	AddEcsTable(state.Data());

	auto wrap = StateWrapper{ .state = state.Data()};

	auto& camera = fx.GetCamera();
	auto& textureRepo = fx.GetTextureRepository();
	auto& audioBank = fx.GetAudioBank();
	
	sol::table fxLua = sol::state_view{ state.Data() }.create_named_table(kFixtureObjectName);
	fxLua[kCameraObjectName] = &camera;
	fxLua[kTextureRepositoryObjectName] = &textureRepo;
	fxLua[kAudioBankObjectName] = &audioBank;

	ExtendSpriteRenderable(wrap, textureRepo.GetSpriteAtlas());
	ExtendTextRenderable(wrap, textureRepo.GetFontAtlas());
	ExtendAudioRequest(wrap, audioBank);
}
