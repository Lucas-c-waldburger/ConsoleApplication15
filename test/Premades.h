#pragma once
#include <format>
#include "../ecs/Ecs.h"
#include "../core/Conversions.h"
#include "../scripting/ScriptManager.h"

static Entity MakeColliderEntity(SDL_FRect rect, Collider::Profile profile)
{
	Entity entity = ECS::CreateEntity();
	assert(entity.IsValid());

	auto [pos, dims] = FromRect(rect);

	auto& spatial = entity.AddComponent(Spatial{ .position = pos, .dimensions = dims });
	auto& collider = entity.AddComponent(Collider{ .position = pos, .dimensions = dims });
	collider.profile = Collider::Profile::Static;

	return entity;
}

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