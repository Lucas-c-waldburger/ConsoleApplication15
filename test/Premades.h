#pragma once
#include <format>
#include "../ecs/Ecs.h"
#include "../core/Conversions.h"
#include "../scripting/ScriptManager.h"
#include "../sdl/SDLUtils.h"
#include "../sdl/SDLite.h"
#include "../systems/CollisionSystem.h"

static Entity MakeStaticColliderEntity(SDL_FRect rect, SDL_Color color = SDLite::kColorBlack, 
                                       uint8_t profile = (Collider::Solid | Collider::Static))
{
	Entity entity = ECS::CreateEntity();
	assert(entity.IsValid());

	auto [pos, dims] = FromRect(rect);

	auto& spatial = entity.AddComponent(Spatial{ .position = pos, .dimensions = dims });
	auto& collider = entity.AddComponent(Collider{ .position = pos, .dimensions = dims, .profile = profile });

    Renderable::Geometry geo{ .color = color };
    auto& renderable = entity.AddComponent(Renderable{ .renderData = geo, .drawOrder = 1 });

	return entity;
}

static Result<Void> InitSimpleEnvironment(CollisionSystem& collisionSystem,
                                          Dimensions<int> sceneDims = { SDLite::kWindowWidth, SDLite::kWindowHeight })
{
    using namespace SDLite;

    std::vector<Entity> entities;
    entities.reserve(4);

    auto& floor = entities.emplace_back(MakeStaticColliderEntity({
        0.0f, static_cast<float>(sceneDims.h - 20),
        static_cast<float>(sceneDims.w), 20.0f }, kColorBrown));
    auto& leftWall = entities.emplace_back(MakeStaticColliderEntity({
        0.0f, 20.0f, 20.0f, 
        static_cast<float>(sceneDims.h - 40) }, kColorYellow));
    auto& rightWall = entities.emplace_back(MakeStaticColliderEntity({
        static_cast<float>(sceneDims.w - 20), 20.0f, 20.0f,
        static_cast<float>(sceneDims.h - 40) }, kColorYellow));
    auto& ceiling = entities.emplace_back(MakeStaticColliderEntity({
        0.0f, 0.0f,
        static_cast<float>(sceneDims.w), 20.0f }, kColorPurple));

    collisionSystem.RebuildQuadTree(sceneDims, entities);

    return Void{};
}


template <ComponentType...Ts>
class EntityAsWrapper
{
protected:
    using RequiredComponentTypes = TypeList<Ts...>;

    Result<Void> SetEntity(Entity&& ent)
    {
        if (!ent.HasComponents<Ts...>())
        {
            return MAKE_ERROR("Entity did not have all required components for wrapper");
        }

        wrappedEntity_ = std::move(ent);

        return Void{};
    }

    Entity wrappedEntity_;
};

template <typename T>
concept DerivedEntityAsWrapper = requires(T t, Entity&& ent) {
    typename T::RequiredComponentTypes;
    { t.SetEntity(ent) } -> std::same_as<Result<Void>>;
};

//template <ComponentType T>
//void RegisterLuaComponentDependencies(ScriptManager& scriptManager, ScriptInstance&& instance);
//
//template <>
//void RegisterLuaComponentDependencies<Physics>(ScriptManager& scriptManager, ScriptInstance&& instance)
//{
//    scriptManager.RegisterScript<
//        SDL_FPoint,
//        AccumulatedForces,
//        Physics>(std::move(instance));
//};
//
//class PremadeScript
//{
//public:
//    const std::string& GetScriptKey() const { return scriptKey_; }
//
//protected:
//    PremadeScript(std::string bp, std::string lfn, std::string sk, ScriptInstance::Setup setup) : 
//        boilerplate_(std::move(bp)), luaFileName_(std::move(lfn)), scriptKey_(std::move(sk)) {}
//
//    std::string boilerplate_;
//    std::string luaFileName_;
//    std::string scriptKey_;
//};
//
//class ComponentTestScript : public PremadeScript
//{
//public:
//    ComponentTestScript(std::string bp, std::string lfn, std::string sk, ScriptInstance::Setup setup) :
//        PremadeScript(std::move(bp), std::move(lfn), std::move(sk), std::move(setup)) {}
//
//    template <typename Fn, ComponentType...Ts>
//    Result<Entity> Setup(ScriptManager& scriptManager, std::string_view scriptPath, Fn&& fn)
//    {
//        auto entity = ECS::CreateEntity();
//        ((entity.AddComponent<Ts>()), ...);
//
//        assert(!scriptPath.empty());
//        std::string assembledPath = std::string{ scriptPath } +
//            ((scriptPath.back() != '\\') ? "\\" : "") + luaFileName_;
//
//        ScriptInstance instance{};
//
//        instance.scriptInfo = {
//            .name = scriptKey_,
//            .scriptType = ScriptType::File,
//            .path = assembledPath;
//        };
//
//        assert(fn);
//        instance.setupFn = [&entity](Lua& lua) {
//            fn(lua, ((entity.GetComponent<Ts>())...));
//        };
//
//
//    }
//};

//static constexpr const char* kPhysTestName = "test::physics";
//static constexpr const char* kPhysTestFileFmt = "{}test_physics.lua";
//
//class PhysicsTestScript : public PremadeScript
//{
//public:
//    PhysicsTestScript() : PremadeScript("", "test_physics.lua", "test::physics") {}
//    Result<Void> Setup(ScriptManager& scriptManager, std::string_view scriptPath) override
//    {
//        auto entity = ECS::CreateEntity();
//        auto& phys = entity.AddComponent(Physics{});
//
//        ScriptInstance instance{};
//
//        instance.scriptInfo = {
//            .name = kPhysTestName,
//            .scriptType = ScriptType::File,
//            .path = std::format(kPhysTestFileFmt, scriptPath)
//        };
//
//        instance.setupFn = [&phys](Lua& lua) {
//            lua["physics"] = &phys;
//        };
//
//        scriptManager.RegisterScript<
//            SDL_FPoint,
//            AccumulatedForces,
//            Physics>(std::move(instance));
//    }
//};

static std::string SetupPhysicsTestScript(ScriptManager& scriptManager, 
                                          std::string_view scriptPath, 
                                          Physics defaults = {})
{
    static constexpr const char* kPhysTestName = "test::physics";
    static constexpr const char* kPhysTestFileFmt = "{}test_physics.lua";

    auto entity = ECS::CreateEntity();
    auto& phys = entity.AddComponent(std::move(defaults));

    ScriptInstance instance{};

    instance.scriptInfo = {
        .name = kPhysTestName,
        .scriptType = ScriptType::File,
        .path = std::format(kPhysTestFileFmt, scriptPath)
    };

    instance.setupFn = [&phys](Lua& lua) {
        lua["physics"] = &phys;
    };

    scriptManager.RegisterScript<
        SDL_FPoint,
        AccumulatedForces,
        Physics>(std::move(instance));

    return std::string{kPhysTestName};
}

//class PhysicsTestScriptSetup
//{
//    static constexpr const char* kPhysicsTestName = "test::physics";
//    static constexpr const char* kPhysicsTestFileFmt = "{}test_physics.lua";
//
//    std::string operator()(ScriptManager& scriptManager, std::string_view scriptPath, Physics defaults = {})
//    {
//        auto entity = ECS::CreateEntity();
//        auto& phys = entity.AddComponent(std::move(defaults));
//
//        ScriptInstance instance{};
//
//        instance.scriptInfo = {
//            .name = kPhysTestName,
//            .scriptType = ScriptType::File,
//            .path = std::format(kPhysTestFileFmt, scriptPath)
//        };
//
//        instance.setupFn = [&phys](Lua& lua) {
//            lua["physics"] = &phys;
//        };
//
//        scriptManager.RegisterScript<
//            SDL_FPoint,
//            AccumulatedForces,
//            Physics>(std::move(instance));
//
//        return std::string{kPhysTestName};
//    }
//}