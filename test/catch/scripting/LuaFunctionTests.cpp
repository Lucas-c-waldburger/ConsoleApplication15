#include "../CatchMain.cpp"
#include "../../../scripting/LuaFunction.h"


TEST_CASE("LuaFunctionTable Tests", "[scripting]")
{
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

    bool registeredAdd = fnTable.RegisterFunction<int(int, int)>("add");
    REQUIRE(registeredAdd);

    bool registeredSayHello = fnTable.RegisterFunction<std::string(std::string_view)>("say_hello");
    REQUIRE(registeredSayHello);

    bool registeredDoNothing = fnTable.RegisterFunction<void()>("do_nothing");
    REQUIRE(registeredDoNothing);

    auto addFn = fnTable.GetFunction<int(int, int)>("add");
    REQUIRE(addFn != nullptr);

    int addResult = addFn(3, 5);
    CHECK(addResult == 8);

    auto sayHelloFn = fnTable.GetFunction<std::string(std::string_view)>("say_hello");
    REQUIRE(sayHelloFn != nullptr);

    std::string sayHelloResult = sayHelloFn("Lucas");
    CHECK(sayHelloResult == "hello Lucas");

    auto doNothingFn = fnTable.GetFunction<void()>("do_nothing");
    doNothingFn();
}