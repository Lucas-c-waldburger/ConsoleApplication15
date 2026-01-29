#include "../CatchUtils.h"
#include "../../../scripting/LuaScript.h"
#include "../../../scripting/user_types/EntityUserType.h"
#include "../../../scripting/user_types/TransformLuaUserTypes.h"
#include "../../../scripting/user_types/SDLUsertypes.h"

struct TestPoint { int x, y; };
struct TestLine { TestPoint startPoint, endPoint; };

DEF_LUA_USERTYPE(TestPoint) {
    lua.def_type(sol::constructors<TestPoint(), TestPoint(int, int)>(),
                     "x", &TestPoint::x, "y", &TestPoint::y);
}

DEF_LUA_USERTYPE(TestLine, Dependencies<TestPoint>) {
    lua.def_type(sol::constructors<TestLine(), TestLine(TestPoint, TestPoint)>(),
        "startPoint", &TestLine::startPoint, "endPoint", &TestLine::endPoint);
}

static constexpr std::string_view kPointLineTestScript = R"(
  function ints_to_point(a, b)
     local p = TestPoint.new()
     p.x = a
     p.y = b
     return p
  end
  function points_to_line(p1, p2)
    local l = TestLine.new()
    l.startPoint = p1
    l.endPoint = p2
    return l
  end
)";


TEST_CASE("LuaScript Tests", "[scripting]")
{
    Logger::StartSession();
    // Registering TestLine propogates registration to its dependency (TestPoint)
    auto luaResult = LuaScript::Create<TestLine>(kPointLineTestScript);
    REQUIRE(luaResult.Success());

    auto& lua = luaResult.GetValue();

    auto fnTable = lua.MakeFunctionTable();

    auto intsToPointFn =
        fnTable.RegisterFunction<TestPoint(int, int)>("ints_to_point");
    REQUIRE(intsToPointFn != nullptr);

    int x = 5;
    int y = 3;

    TestPoint pointFnResult = intsToPointFn(5, 3);

    CHECK(pointFnResult.x == 5);
    CHECK(pointFnResult.y == 3);

    auto pointsToLineFn = 
        fnTable.RegisterFunction<TestLine(TestPoint, TestPoint)>("points_to_line");
    REQUIRE(pointsToLineFn != nullptr);
    
    TestPoint p1{ 1, 2 };
    TestPoint p2{ 2 , 5 };

    TestLine lineFnResult = pointsToLineFn(p1, p2);

    CHECK(lineFnResult.startPoint.x == 1);
    CHECK(lineFnResult.startPoint.y == 2);
    CHECK(lineFnResult.endPoint.x == 2);
    CHECK(lineFnResult.endPoint.y == 5);
}


TEST_CASE("LuaFunctionTable Tests", "[scripting]")
{
    Logger::StartSession();

    sol::state lua;
    lua.open_libraries(sol::lib::base);
   
    lua.script(R"(
        function add(a, b)
            return a + b
        end 
        function say_hello(name)
            return "hello " .. name
        end
        function do_nothing()
            return
        end
    )");

    LuaFunctionTable fnTable{ lua };

    auto addFn = fnTable.RegisterFunction<int(int, int)>("add");
    REQUIRE(addFn != nullptr);

    auto sayHelloFn = fnTable.RegisterFunction<std::string(std::string_view)>("say_hello");
    REQUIRE(sayHelloFn != nullptr);

    auto doNothingFn = fnTable.RegisterFunction<void()>("do_nothing");
    REQUIRE(doNothingFn != nullptr);

    int addResult = addFn(3, 5);
    CHECK(addResult == 8);

    std::string sayHelloResult = sayHelloFn("Lucas");
    CHECK(sayHelloResult == "hello Lucas");

    doNothingFn();
}

static constexpr std::string_view kEntityTransformAddScript = R"(
    function add_transform(entity)
        local tf = entity:AddComponentTransform()

        pos = SDL_FPoint.new()
        pos.x = 5.0
        pos.y = 8.0
        tf.position = pos

        return tf
    end
)";

TEST_CASE("Lua Integration with Entity", "[scripting]")
{
    Logger::StartSession();

    auto luaResult = LuaScript::Create<Entity>(kEntityTransformAddScript);
    REQUIRE(luaResult.Success());

    auto& lua = luaResult.GetValue();

    auto fnTable = lua.MakeFunctionTable();

    auto addTfFn = fnTable.RegisterFunction<Transform*(Entity&)>("add_transform");
    REQUIRE(addTfFn != nullptr);

    auto entity = ECS::CreateEntity();
    REQUIRE(entity.IsValid());
    REQUIRE_FALSE(entity.HasComponent<Transform>());

    auto* tfPtr = addTfFn(entity);
    REQUIRE(tfPtr != nullptr);
    CHECK(tfPtr->position.x == 5.0);
    CHECK(tfPtr->position.y == 8.0);
    
    REQUIRE(entity.HasComponent<Transform>());
    auto& tfRef = entity.GetComponent<Transform>();

    CHECK(tfRef.position.x == tfPtr->position.x);
    CHECK(tfRef.position.y == tfPtr->position.y);
}