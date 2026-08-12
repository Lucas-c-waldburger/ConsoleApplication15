#include "FixtureLua.h"

namespace {

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

} // unnamed

void ExtendSpriteRenderable(StateWrapper& state, SpriteAtlas& atlas)
{
	if (auto userType = state.GetUserType<SpriteRenderableComponent>())
	{
		userType["setSprite"] = [&atlas](SpriteRenderableComponent& r, std::string name) {
			r.sprite = atlas.GetSprite(name);
			if (!r.sprite.resourceHandle.IsValid())
			{
				LOG_ERROR_FMT("sprite '{}' not found");
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
				LOG_ERROR_FMT("Font '{}' not found");
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

void SetUpFixtureLuaState(SceneFixture::SharedPtr& fx)
{
	if (!fx->IsSystemRegistered<ScriptSystem>())
	{
		return;
	}

	auto& scriptSys = fx->GetSystem<ScriptSystem>();
	scriptSys.InitState<Entity, TextureRepository, Camera, AudioBank>();

	auto& state = scriptSys.GetState();
	auto wrap = StateWrapper{ .state = state };

	auto& camera = fx->GetCamera();
	auto& textureRepo = fx->GetTextureRepository();
	auto& audioBank = fx->GetAudioBank();
	
	sol::table fxLua = state.create_named_table(kFixtureObjectName);
	fxLua[kCameraObjectName] = &camera;
	fxLua[kTextureRepositoryObjectName] = &textureRepo;
	fxLua[kAudioBankObjectName] = &audioBank;

	ExtendSpriteRenderable(wrap, textureRepo.GetSpriteAtlas());
	ExtendTextRenderable(wrap, textureRepo.GetFontAtlas());
	ExtendAudioRequest(wrap, audioBank);
}
