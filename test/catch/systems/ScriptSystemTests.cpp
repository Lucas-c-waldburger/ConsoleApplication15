#include "../CatchUtils.h"
#include "../../../file/FilePathUtility.h"
#include "../../../systems/ScriptSystem.h"
#include "../../../events/EventBus2.h"
#include "../../../scripting/user_types/EntityUserType.h"

namespace {

std::filesystem::path MakeScriptTestPath(std::string_view scriptFilename)
{
	namespace fs = std::filesystem;

	auto path = FilePathUtility::GetRootPath() /
		fs::path("test/catch/test_scripts") / fs::path(scriptFilename);

	return path;
}

constexpr std::pair<uint32_t, uint32_t> DecomposeArgType(uint64_t argType)
{
	return std::make_pair(
		static_cast<uint32_t>(argType >> 32),
		static_cast<uint32_t>(argType & 0xFFFFFFFFull)
	);
}

} // unnamed

TEST_CASE("ScriptSystem Tests", "[sys][script][b]")
{
	ScriptSystem scriptSystem{};

	scriptSystem.GetState().InitWithEngineTypes<Entity>();

	auto entityTestFilepath = MakeScriptTestPath("entity_test.lua");
	REQUIRE(std::filesystem::exists(entityTestFilepath));

	auto addTableResult = scriptSystem.AddFunctionTable(entityTestFilepath.string());
	REQUIRE_RESULT(addTableResult);

	auto tableId = addTableResult.GetValue();
	CHECK(tableId != std::numeric_limits<ScriptTable::TableId>::max());

	CHECK(scriptSystem.GetTableFilepath(tableId) == entityTestFilepath.string());

	auto e = ECS::CreateEntity();
	REQUIRE(e.IsValid());
	e.AddComponent(Name{});
	e.AddComponent(Transform{});

	auto tableView = scriptSystem.GetTableView(tableId);
	CHECK(tableView.IsValid());

	auto& script = e.AddComponent(Script{ .table = tableView });

	CHECK(script.table.Contains("changeName"));
	CHECK(script.table.Contains("changePos"));

	CHECK(script.table.GetTableId() == tableId);

	auto changeNameFn = script.table["changeName"];
	CHECK(changeNameFn);

	changeNameFn(e);

	REQUIRE(e.HasComponent<Name>());
	CHECK(e.GetComponent<Name>() == "changed");

	auto changePosFn = script.table["changePos"];
	CHECK(changePosFn);

	changePosFn(e);

	REQUIRE(e.HasComponent<Transform>());
	CHECK(e.GetComponent<Transform>().position.x == 500.0f);
	CHECK(e.GetComponent<Transform>().position.y == 500.0f);
}

TEST_CASE("ScriptSystem::RemoveTable", "[sys][script][a]")
{
	ScriptSystem scriptSystem{};

	scriptSystem.GetState().InitWithEngineTypes<Entity>();

	auto testFilepath1 = MakeScriptTestPath("entity_test.lua");
	REQUIRE(std::filesystem::exists(testFilepath1));
	auto testFilepath2 = MakeScriptTestPath("empty.lua");
	REQUIRE(std::filesystem::exists(testFilepath2));

	auto addTable1Result = scriptSystem.AddFunctionTable(testFilepath1.string());
	REQUIRE_RESULT(addTable1Result);
	auto addTable2Result = scriptSystem.AddFunctionTable(testFilepath2.string());
	REQUIRE_RESULT(addTable2Result);

	const auto table1Id = addTable1Result.GetValue();
	const auto table2Id = addTable2Result.GetValue();

	auto tableView1 = scriptSystem.GetTableView(table1Id);
	CHECK(tableView1.IsValid());
	auto tableView2 = scriptSystem.GetTableView(table2Id);
	CHECK(tableView2.IsValid());

	auto e1 = ECS::CreateEntity();
	REQUIRE(e1.IsValid());
	auto e2 = ECS::CreateEntity();
	REQUIRE(e2.IsValid());
	auto e3 = ECS::CreateEntity();
	REQUIRE(e3.IsValid());

	e1.AddComponent(Script{ .table = tableView1 });
	e2.AddComponent(Script{ .table = tableView1 });
	e3.AddComponent(Script{ .table = tableView2 });

	CHECK(e1.GetComponent<Script>().table.IsValid());
	CHECK(e2.GetComponent<Script>().table.IsValid());
	CHECK(e3.GetComponent<Script>().table.IsValid());

	const bool removedTable1 = scriptSystem.RemoveTable(table1Id);
	CHECK(removedTable1);

	CHECK_FALSE(e1.GetComponent<Script>().table.IsValid());
	CHECK_FALSE(e2.GetComponent<Script>().table.IsValid());
	CHECK(e3.GetComponent<Script>().table.IsValid());

	const bool removedTable2 = scriptSystem.RemoveTable(table2Id);
	CHECK(removedTable2);

	CHECK_FALSE(e1.GetComponent<Script>().table.IsValid());
	CHECK_FALSE(e2.GetComponent<Script>().table.IsValid());
	CHECK_FALSE(e3.GetComponent<Script>().table.IsValid());
}

TEST_CASE("LuaFunctionTableParser Tests", "[script][b]")
{
	Logger::StartSession();
	LuaStateManager state{};
	state.InitWithEngineTypes<Entity>();

	struct TestUserStructA {};
	struct TestUserStructB {};
	struct TestUserStructC {};
	enum class TestUserEnum { A = 1, B = 2 };

	CHECK(state.NewUserType<TestUserStructA>("TestUserStructA"));
	CHECK(state.NewUserType<TestUserStructB>("TestUserStructB"));
	CHECK(state.NewUserType<TestUserStructC>("TestUserStructC"));
	CHECK(state.NewEnum<TestUserEnum>("TestUserEnum", "A", TestUserEnum::A, "B", TestUserEnum::B));

	CHECK(state.IsRegistered("TestUserStructA"));
	CHECK(state.IsRegistered("TestUserStructB"));
	CHECK(state.IsRegistered("TestUserStructC"));
	CHECK(state.IsRegistered("TestUserEnum"));

	static const std::string kArgA = "TestUserStructA";
	static const std::string kArgB = "const TestUserStructB";
	static const std::string kArgC = "const TestUserStructC *";
	static const std::string kArgEnum = "TestUserEnum&";

	SECTION("Argument parsing")
	{
		auto parseResultA = LuaFunctionTableParser::ParseFullArgumentType(kArgA, state);
		REQUIRE_RESULT(parseResultA);

		const auto [qualsA, typeA] = DecomposeArgType(parseResultA.GetValue());
		CHECK(qualsA == 0);
		CHECK(typeA == TypeInfo<TestUserStructA>::hash32);

		auto parseResultB = LuaFunctionTableParser::ParseFullArgumentType(kArgB, state);
		REQUIRE_RESULT(parseResultB);

		const auto [qualsB, typeB] = DecomposeArgType(parseResultB.GetValue());
		CHECK(qualsB == 0); // const auto removed since value type
		CHECK(typeB == TypeInfo<TestUserStructB>::hash32);

		auto parseResultC = LuaFunctionTableParser::ParseFullArgumentType(kArgC, state);
		REQUIRE_RESULT(parseResultC);

		const auto [qualsC, typeC] = DecomposeArgType(parseResultC.GetValue());
		CHECK(qualsC == (LuaTypeQualifiers::Const | LuaTypeQualifiers::Ptr)); 
		CHECK(typeC == TypeInfo<TestUserStructC>::hash32);

		auto parseResultEnum = LuaFunctionTableParser::ParseFullArgumentType(kArgEnum, state);
		REQUIRE_RESULT(parseResultEnum);

		const auto [qualsEnum, typeEnum] = DecomposeArgType(parseResultEnum.GetValue());
		CHECK(qualsEnum == LuaTypeQualifiers::Ref);
		CHECK(typeEnum == TypeInfo<TestUserEnum>::hash32);
	}

	SECTION("Table Parsing - Single Arguments")
	{
		static const std::string fnTableLua = R"(
			local table = {}

			function table.A(a)
			end

			function table.B(b)        
			end

			function table.C(c)        
			end

			function table.E(e)        
			end

			local meta = {
			    __signatures = { 
			        A = { "TestUserStructA" },
			        B = { "const TestUserStructB" },
			        C = { "const TestUserStructC *" },
			        E = { "TestUserEnum&" }
			    }
			}
	
			setmetatable(table, meta)
			
			return table
		)";

		auto loadRet = state.LoadScriptString(fnTableLua);
		if (!loadRet.valid())
		{
			sol::error err = loadRet;
			CAPTURE(err.what());
			REQUIRE(false);
		}

		REQUIRE(loadRet.get_type() == sol::type::table);

		auto table = loadRet.get<sol::table>();

		auto tableParseResult = 
			LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state);
		REQUIRE_RESULT(tableParseResult);

		const auto& tableParsed = tableParseResult.GetValue();
		CHECK(tableParsed.size() == 4);

		REQUIRE(tableParsed.contains("A"));
		REQUIRE(tableParsed["A"].size() == 1);
		const auto [qualsA, typeA] = DecomposeArgType(tableParsed["A"].front());
		CHECK(qualsA == 0);
		CHECK(typeA == TypeInfo<TestUserStructA>::hash32);

		REQUIRE(tableParsed.contains("B"));
		REQUIRE(tableParsed["B"].size() == 1);
		const auto [qualsB, typeB] = DecomposeArgType(tableParsed["B"].front());
		CHECK(qualsB == 0); // const auto removed since value type
		CHECK(typeB == TypeInfo<TestUserStructB>::hash32);

		REQUIRE(tableParsed.contains("C"));
		REQUIRE(tableParsed["C"].size() == 1);
		const auto [qualsC, typeC] = DecomposeArgType(tableParsed["C"].front());
		CHECK(qualsC == (LuaTypeQualifiers::Const | LuaTypeQualifiers::Ptr));
		CHECK(typeC == TypeInfo<TestUserStructC>::hash32);

		REQUIRE(tableParsed.contains("E"));
		REQUIRE(tableParsed["E"].size() == 1);
		const auto [qualsEnum, typeEnum] = DecomposeArgType(tableParsed["E"].front());
		CHECK(qualsEnum == LuaTypeQualifiers::Ref);
		CHECK(typeEnum == TypeInfo<TestUserEnum>::hash32);
	}

	SECTION("Table Parsing - Multiple Arguments")
	{
		static const std::string fnTableLua = R"(
			local table = {}

			function table.A_B(a, b)
			end

			function table.B_C(b, c)        
			end

			function table.A_B_C(a, b, c)        
			end

			function table.A_B_C_E(a, b, c, e)        
			end

			local meta = {
			    __signatures = { 
			        A_B = { "TestUserStructA", "const TestUserStructB&" },
			        B_C = { "const TestUserStructB", "TestUserStructC  *" },
			        A_B_C = { "TestUserStructA", "TestUserStructB const &", "const TestUserStructC  *" },
			        A_B_C_E = { "TestUserStructA", "TestUserStructB const ", "TestUserStructC*", "TestUserEnum const *" }
			    }
			}
	
			setmetatable(table, meta)
			
			return table
		)";

		auto loadRet = state.LoadScriptString(fnTableLua);
		if (!loadRet.valid())
		{
			sol::error err = loadRet;
			CAPTURE(err.what());
			REQUIRE(false);
		}

		REQUIRE(loadRet.get_type() == sol::type::table);

		auto table = loadRet.get<sol::table>();

		auto tableParseResult =
			LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state);
		REQUIRE_RESULT(tableParseResult);

		const auto& tableParsed = tableParseResult.GetValue();
		CHECK(tableParsed.size() == 4);

		{
		REQUIRE(tableParsed.contains("A_B"));
		REQUIRE(tableParsed["A_B"].size() == 2);

		const auto [qualsA, typeA] = DecomposeArgType(tableParsed["A_B"][0]);
		CHECK(qualsA == 0);
		CHECK(typeA == TypeInfo<TestUserStructA>::hash32);

		const auto [qualsB, typeB] = DecomposeArgType(tableParsed["A_B"][1]);
		CHECK(qualsB == (LuaTypeQualifiers::Const | LuaTypeQualifiers::Ref));
		CHECK(typeB == TypeInfo<TestUserStructB>::hash32);
		}

		{
		REQUIRE(tableParsed.contains("B_C"));
		REQUIRE(tableParsed["B_C"].size() == 2);

		const auto [qualsB, typeB] = DecomposeArgType(tableParsed["B_C"][0]);
		CHECK(qualsB == 0); // const auto removed since value type
		CHECK(typeB == TypeInfo<TestUserStructB>::hash32);

		const auto [qualsC, typeC] = DecomposeArgType(tableParsed["B_C"][1]);
		CHECK(qualsC == LuaTypeQualifiers::Ptr);
		CHECK(typeC == TypeInfo<TestUserStructC>::hash32);
		}

		{
		REQUIRE(tableParsed.contains("A_B_C"));
		REQUIRE(tableParsed["A_B_C"].size() == 3);

		const auto [qualsA, typeA] = DecomposeArgType(tableParsed["A_B_C"][0]);
		CHECK(qualsA == 0);
		CHECK(typeA == TypeInfo<TestUserStructA>::hash32);

		const auto [qualsB, typeB] = DecomposeArgType(tableParsed["A_B_C"][1]);
		CHECK(qualsB == (LuaTypeQualifiers::Const | LuaTypeQualifiers::Ref));
		CHECK(typeB == TypeInfo<TestUserStructB>::hash32);

		const auto [qualsC, typeC] = DecomposeArgType(tableParsed["A_B_C"][2]);
		CHECK(qualsC == (LuaTypeQualifiers::Const | LuaTypeQualifiers::Ptr));
		CHECK(typeC == TypeInfo<TestUserStructC>::hash32);
		}

		{
		REQUIRE(tableParsed.contains("A_B_C_E"));
		REQUIRE(tableParsed["A_B_C_E"].size() == 4);

		const auto [qualsA, typeA] = DecomposeArgType(tableParsed["A_B_C_E"][0]);
		CHECK(qualsA == 0);
		CHECK(typeA == TypeInfo<TestUserStructA>::hash32);

		const auto [qualsB, typeB] = DecomposeArgType(tableParsed["A_B_C_E"][1]);
		CHECK(qualsB == 0);
		CHECK(typeB == TypeInfo<TestUserStructB>::hash32);

		const auto [qualsC, typeC] = DecomposeArgType(tableParsed["A_B_C_E"][2]);
		CHECK(qualsC == LuaTypeQualifiers::Ptr);
		CHECK(typeC == TypeInfo<TestUserStructC>::hash32);

		const auto [qualsE, typeE] = DecomposeArgType(tableParsed["A_B_C_E"][3]);
		CHECK(qualsE == (LuaTypeQualifiers::Const | LuaTypeQualifiers::Ptr));
		CHECK(typeE == TypeInfo<TestUserEnum>::hash32);
		}
	}
}

TEST_CASE("LuaFunctionCallHandler Tests", "[scripting][b]")
{
	Logger::StartSession();
	LuaStateManager state{};
	state.InitWithEngineTypes<Entity>();

	struct TestUserStructA {};
	struct TestUserStructB {};
	struct TestUserStructC {};
	enum class TestUserEnum { A = 1, B = 2 };

	CHECK(state.NewUserType<TestUserStructA>("TestUserStructA"));
	CHECK(state.NewUserType<TestUserStructB>("TestUserStructB"));
	CHECK(state.NewUserType<TestUserStructC>("TestUserStructC"));
	CHECK(state.NewEnum<TestUserEnum>("TestUserEnum", "A", TestUserEnum::A, "B", TestUserEnum::B));

	CHECK(state.IsRegistered("TestUserStructA"));
	CHECK(state.IsRegistered("TestUserStructB"));
	CHECK(state.IsRegistered("TestUserStructC"));
	CHECK(state.IsRegistered("TestUserEnum"));

	static const std::string kArgA = "TestUserStructA";
	static const std::string kArgB = "const TestUserStructB";
	static const std::string kArgC = "const TestUserStructC *";
	static const std::string kArgEnum = "TestUserEnum&";

	SECTION("TryMakeArgumentLuaObject")
	{
		TestUserStructA structAValue{};
		const TestUserStructA structAValueConst{};
		TestUserStructA* structAPtr = &structAValue;
		const TestUserStructA* structAPtrConst = &structAValueConst;
		TestUserStructA* const structAConstPtr = &structAValue;
		TestUserStructA& structARef = structAValue;
		const TestUserStructA& structARefConst = structAValueConst;
		TestUserStructA* structAPtrNull = nullptr;
		const TestUserStructA* structAPtrConstNull = nullptr;
		TestUserStructA** structADoublePtr = &structAPtr;

		constexpr uint64_t parsedTypeValue = static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32);
		constexpr uint64_t parsedTypeConstValue = { 
			static_cast<uint64_t>(LuaTypeQualifiers::Const) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32) 
		};
		constexpr uint64_t parsedTypeRef = {
			static_cast<uint64_t>(LuaTypeQualifiers::Ref) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32)
		};
		constexpr uint64_t parsedTypeRefConst = {
			static_cast<uint64_t>(LuaTypeQualifiers::Ref | LuaTypeQualifiers::Const) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32)
		};
		constexpr uint64_t parsedTypePtr = {
			static_cast<uint64_t>(LuaTypeQualifiers::Ptr) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32)
		};
		constexpr uint64_t parsedTypePtrConst = {
			static_cast<uint64_t>(LuaTypeQualifiers::Ptr | LuaTypeQualifiers::Const) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32)
		};

		STATIC_CHECK_FALSE(AcceptedLuaTypeQualified<int&&>);
		STATIC_CHECK_FALSE(AcceptedLuaTypeQualified<const int&&>);
		STATIC_CHECK_FALSE(AcceptedLuaTypeQualified<int**>);
		STATIC_CHECK_FALSE(AcceptedLuaTypeQualified<int***>);
		STATIC_CHECK_FALSE(AcceptedLuaTypeQualified<int**&>);
		STATIC_CHECK_FALSE(AcceptedLuaTypeQualified<int*&&>);

		SECTION("Lua requests value type")
		{
		// T value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), TestUserStructA{}, parsedTypeValue).valid());
		// const T value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), static_cast<const TestUserStructA>(TestUserStructA{}), parsedTypeValue).valid());
		// T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARef, parsedTypeValue).valid());
		// const T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARefConst, parsedTypeValue).valid());
		// T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtr, parsedTypeValue).valid());
		// const T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrConst, parsedTypeValue).valid());
		// T const ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAConstPtr, parsedTypeValue).valid());
		// T ptr null - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrNull, parsedTypeValue).valid());
		}

		SECTION("Lua requests const value type")
		{
		// T value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), TestUserStructA{}, parsedTypeConstValue).valid());
		// const T value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), static_cast<const TestUserStructA>(TestUserStructA{}), parsedTypeConstValue).valid());
		// T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARef, parsedTypeConstValue).valid());
		// const T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARefConst, parsedTypeConstValue).valid());
		// T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtr, parsedTypeConstValue).valid());
		// const T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrConst, parsedTypeConstValue).valid());
		// T const ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAConstPtr, parsedTypeConstValue).valid());
		// T ptr null - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrNull, parsedTypeConstValue).valid());
		}

		SECTION("Lua requests reference type")
		{
		// T value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), TestUserStructA{}, parsedTypeRef).valid());
		// const T value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), static_cast<const TestUserStructA>(TestUserStructA{}), parsedTypeRef).valid());
		// T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARef, parsedTypeRef).valid());
		// const T ref value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARefConst, parsedTypeRef).valid());
		// T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtr, parsedTypeRef).valid());
		// const T ptr value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrConst, parsedTypeRef).valid());
		// T const ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAConstPtr, parsedTypeRef).valid());
		// T ptr null - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrNull, parsedTypeRef).valid());
		}

		SECTION("Lua requests const reference type")
		{
		// T value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), TestUserStructA{}, parsedTypeRefConst).valid());
		// const T value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), static_cast<const TestUserStructA>(TestUserStructA{}), parsedTypeRefConst).valid());
		// T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARef, parsedTypeRefConst).valid());
		// const T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARefConst, parsedTypeRefConst).valid());
		// T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtr, parsedTypeRefConst).valid());
		// const T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrConst, parsedTypeRefConst).valid());
		// T const ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAConstPtr, parsedTypeRefConst).valid());
		// T ptr null - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrNull, parsedTypeRefConst).valid());
		}

		SECTION("Lua requests pointer type")
		{
		// T value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), TestUserStructA{}, parsedTypePtr).valid());
		// const T value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), static_cast<const TestUserStructA>(TestUserStructA{}), parsedTypePtr).valid());
		// T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARef, parsedTypePtr).valid());
		// const T ref value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARefConst, parsedTypePtr).valid());
		// T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtr, parsedTypePtr).valid());
		// const T ptr value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrConst, parsedTypePtr).valid());
		// T const ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAConstPtr, parsedTypePtr).valid());
		// T ptr null - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrNull, parsedTypePtr).valid());
		}

		SECTION("Lua requests pointer to const type")
		{
		// T value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), TestUserStructA{}, parsedTypePtrConst).valid());
		// const T value - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), static_cast<const TestUserStructA>(TestUserStructA{}), parsedTypePtrConst).valid());
		// T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARef, parsedTypePtrConst).valid());
		// const T ref value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structARefConst, parsedTypePtrConst).valid());
		// T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtr, parsedTypePtrConst).valid());
		// const T ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrConst, parsedTypePtrConst).valid());
		// T const ptr value - value
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAConstPtr, parsedTypePtrConst).valid());
		// T ptr null - value FAIL
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), structAPtrNull, parsedTypePtrConst).valid());
		}
	}

	SECTION("CallLuaFunctionQualified")
	{
		constexpr uint64_t userStructARefType = {
			static_cast<uint64_t>(LuaTypeQualifiers::Ref) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32)
		};
		constexpr uint64_t userStructBConstValueType = {
			static_cast<uint64_t>(LuaTypeQualifiers::Const) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructB>::hash32)
		};
		constexpr uint64_t userStructCPtrConstType = {
			static_cast<uint64_t>(LuaTypeQualifiers::Const | LuaTypeQualifiers::Ptr) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructC>::hash32)
		};

		std::vector<uint64_t> argTypes = { 
			userStructARefType, userStructBConstValueType, userStructCPtrConstType 
		};

		sol::state_view stateView{ state.Data() };

		std::string fnOutput;
		stateView["fn"] = 
			[&fnOutput](TestUserStructA& a, const TestUserStructB b, const TestUserStructC* c) {
				fnOutput = "called";
			};

		sol::function fn = stateView["fn"];
		REQUIRE(fn.valid());

		TestUserStructA userStructA{};
		TestUserStructB userStructB{};
		TestUserStructC userStructC{};

		SECTION("Different argument type permutations")
		{
			{
			auto callResult = LuaFunctionCallHandler::CallLuaFunctionQualified(
				fn, argTypes, userStructA, static_cast<const TestUserStructB>(TestUserStructB{}),
				&userStructC
			);
			REQUIRE_RESULT(callResult);

			CHECK(fnOutput == "called");
			fnOutput.clear();
			}

			{
			auto callResult = LuaFunctionCallHandler::CallLuaFunctionQualified(
				fn, argTypes, userStructA, userStructB, &userStructC
			);
			REQUIRE_RESULT(callResult);

			CHECK(fnOutput == "called");
			fnOutput.clear();
			}

			{
			const auto* cPtr = &userStructC;

			auto callResult = LuaFunctionCallHandler::CallLuaFunctionQualified(
				fn, argTypes, userStructA, userStructB, cPtr
			);
			REQUIRE_RESULT(callResult);

			CHECK(fnOutput == "called");
			fnOutput.clear();
			}
		}

		SECTION("Different argument ordering")
		{
			{
			auto callResult = LuaFunctionCallHandler::CallLuaFunctionQualified(
				fn, argTypes, userStructA, &userStructC, userStructB
			);
			REQUIRE_RESULT(callResult);

			CHECK(fnOutput == "called");
			fnOutput.clear();
			}

			{
			auto callResult = LuaFunctionCallHandler::CallLuaFunctionQualified(
				fn, argTypes, userStructB, userStructA, &userStructC
			);
			REQUIRE_RESULT(callResult);

			CHECK(fnOutput == "called");
			fnOutput.clear();
			}

			{
			auto callResult = LuaFunctionCallHandler::CallLuaFunctionQualified(
				fn, argTypes, &userStructC, userStructB, userStructA
			);
			REQUIRE_RESULT(callResult);

			CHECK(fnOutput == "called");
			fnOutput.clear();
			}
		}

		SECTION("Non-unique argument types")
		{
			std::string firstCall;
			std::string secondCall;

			struct TestUserStructD {
				std::string id;
			};

			CHECK(state.NewUserType<TestUserStructD>("TestUserStructD", "id", &TestUserStructD::id));
			CHECK(state.IsRegistered("TestUserStructD"));

			stateView["repeatArgFn1"] = [&firstCall, &secondCall]
			(TestUserStructD& d1, const TestUserStructB* b, const TestUserStructD d2) {
				firstCall = d1.id;
				secondCall = d2.id;
			};
			stateView["repeatArgFn2"] = [&firstCall, &secondCall]
			(const TestUserStructD d1, const TestUserStructB* b, TestUserStructD& d2) {
				firstCall = d1.id;
				secondCall = d2.id;
			};

			constexpr uint64_t structDRefType = {
				static_cast<uint64_t>(LuaTypeQualifiers::Ref) << 32 |
				static_cast<uint64_t>(TypeInfo<TestUserStructD>::hash32)
			};
			constexpr uint64_t structDConstValType = {
				static_cast<uint64_t>(LuaTypeQualifiers::Const) << 32 |
				static_cast<uint64_t>(TypeInfo<TestUserStructD>::hash32)
			};
			constexpr uint64_t structBConstPtrType = {
				static_cast<uint64_t>(LuaTypeQualifiers::Const | LuaTypeQualifiers::Ptr) << 32 |
				static_cast<uint64_t>(TypeInfo<TestUserStructB>::hash32)
			};

			const std::vector<uint64_t> repeatArgFn1Types = {
				structDRefType, structBConstPtrType, structDConstValType
			};
			const std::vector<uint64_t> repeatArgFn2Types = {
				structDConstValType, structBConstPtrType, structDRefType
			}; 

			sol::function repeatArgFn1 = stateView["repeatArgFn1"];
			REQUIRE(repeatArgFn1.valid());
			sol::function repeatArgFn2 = stateView["repeatArgFn2"];
			REQUIRE(repeatArgFn2.valid());

			TestUserStructD structDInstance{ .id = "ref instance" };
			TestUserStructB structBInstance{};

			{
			auto callResult = LuaFunctionCallHandler::CallLuaFunctionQualified(
				repeatArgFn1, repeatArgFn1Types, structDInstance, &structBInstance, 
				TestUserStructD{ .id = "value instance" }
			);
			REQUIRE_RESULT(callResult);

			CHECK(firstCall == "ref instance");
			CHECK(secondCall == "value instance");
			firstCall.clear();
			secondCall.clear();
			}

			{
			auto callResult = LuaFunctionCallHandler::CallLuaFunctionQualified(
				repeatArgFn2, repeatArgFn2Types, structDInstance, &structBInstance,
				TestUserStructD{ .id = "value instance" }
			);
			REQUIRE_RESULT(callResult);

			CHECK(firstCall == "value instance");
			CHECK(secondCall == "ref instance");
			firstCall.clear();
			secondCall.clear();
			}
		}
	}
}

TEST_CASE("Native Lua Type identification", "[scripting]")
{
	SECTION("Number")
	{
		STATIC_CHECK(IsNativeLuaType<int>());
		STATIC_CHECK(IsNativeLuaType<double>());
		STATIC_CHECK(IsNativeLuaType<float>());
		STATIC_CHECK(IsNativeLuaType<const uint8_t>());
		STATIC_CHECK(IsNativeLuaType<uint32_t>());
		STATIC_CHECK(IsNativeLuaType<const uint64_t*>());

		STATIC_CHECK(GetNativeLuaTypeId<int>() == kNativeNumberLuaTypeId);
		STATIC_CHECK(GetNativeLuaTypeId<double>() == kNativeNumberLuaTypeId);
		STATIC_CHECK(GetNativeLuaTypeId<float>() == kNativeNumberLuaTypeId);
		STATIC_CHECK(GetNativeLuaTypeId<const uint8_t>() == kNativeNumberLuaTypeId);
		STATIC_CHECK(GetNativeLuaTypeId<uint32_t&>() == kNativeNumberLuaTypeId);
		STATIC_CHECK(GetNativeLuaTypeId<const uint64_t*>() == kNativeNumberLuaTypeId);

		STATIC_CHECK(GetNativeLuaTypeName<int>() == "number");
		STATIC_CHECK(GetNativeLuaTypeName<uint32_t&>() == "number");
		STATIC_CHECK(GetNativeLuaTypeName<const uint64_t*>() == "number");
	}

	SECTION("Boolean")
	{
		STATIC_CHECK(IsNativeLuaType<bool>());
		STATIC_CHECK(IsNativeLuaType<bool*&>());

		STATIC_CHECK(GetNativeLuaTypeId<bool>() == kNativeBooleanLuaTypeId);
		STATIC_CHECK_FALSE(GetNativeLuaTypeId<bool>() == kNativeNumberLuaTypeId);

		STATIC_CHECK(GetNativeLuaTypeName<bool>() == "boolean");
		STATIC_CHECK(GetNativeLuaTypeName<bool*&>() == "boolean");
	}

	SECTION("String")
	{
		STATIC_CHECK(IsNativeLuaType<const char*>());
		STATIC_CHECK(IsNativeLuaType<std::string>());
		STATIC_CHECK(IsNativeLuaType<std::string_view>());
		STATIC_CHECK(IsNativeLuaType<char[8]>());

		STATIC_CHECK(GetNativeLuaTypeId<const char*>() == kNativeStringLuaTypeId);
		STATIC_CHECK(GetNativeLuaTypeId<std::string>() == kNativeStringLuaTypeId);
		STATIC_CHECK(GetNativeLuaTypeId<std::string_view>() == kNativeStringLuaTypeId);
		STATIC_CHECK(GetNativeLuaTypeId<char[8]>() == kNativeStringLuaTypeId);

		STATIC_CHECK(GetNativeLuaTypeName<const char*>() == "string");
		STATIC_CHECK(GetNativeLuaTypeName<std::string>() == "string");
		STATIC_CHECK(GetNativeLuaTypeName<std::string_view>() == "string");
		STATIC_CHECK(GetNativeLuaTypeName<char[8]>() == "string");
	}
}

TEST_CASE("Lua function parsing/call with Native lua types", "[scripting]")
{
	Logger::StartSession();
	LuaStateManager state{};
	state.InitWithEngineTypes<>();

	struct TestUserStructA {};
	enum class TestUserEnum { A = 1, B = 2 };

	CHECK(state.NewUserType<TestUserStructA>("TestUserStructA"));
	CHECK(state.NewEnum<TestUserEnum>("TestUserEnum", "A", TestUserEnum::A, "B", TestUserEnum::B));

	CHECK(state.IsRegistered("TestUserStructA"));
	CHECK(state.IsRegistered("TestUserEnum"));

	CHECK(state.IsRegistered("number"));
	CHECK(state.IsRegistered("boolean"));
	CHECK(state.IsRegistered("string"));

	SECTION("Qualifiers get stripped for native lua types when string-parsed")
	{
		static const std::string numberRefTypeStr = "number&";
		static const std::string booleanPtrTypeStr = "boolean *";
		static const std::string stringConstRefTypeStr = "const  string&";

		auto parseResultNum = LuaFunctionTableParser::ParseFullArgumentType(numberRefTypeStr, state);
		REQUIRE_RESULT(parseResultNum);

		const auto [qualsNum, typeNum] = DecomposeArgType(parseResultNum.GetValue());
		CHECK(qualsNum == 0);
		CHECK(typeNum == kNativeNumberLuaTypeId);

		auto parseResultBool = LuaFunctionTableParser::ParseFullArgumentType(booleanPtrTypeStr, state);
		REQUIRE_RESULT(parseResultBool);

		const auto [qualsBool, typeBool] = DecomposeArgType(parseResultBool.GetValue());
		CHECK(qualsBool == 0);
		CHECK(typeBool == kNativeBooleanLuaTypeId);

		auto parseResultString = LuaFunctionTableParser::ParseFullArgumentType(stringConstRefTypeStr, state);
		REQUIRE_RESULT(parseResultString);

		const auto [qualsString, typeString] = DecomposeArgType(parseResultString.GetValue());
		CHECK(qualsString == 0);
		CHECK(typeString == kNativeStringLuaTypeId);
	}

	SECTION("Correct argument matching for native lua types")
	{
		constexpr uint64_t numberRefType = {
			static_cast<uint64_t>(LuaTypeQualifiers::Ref) << 32 |
			static_cast<uint64_t>(kNativeNumberLuaTypeId)
		};
		constexpr uint64_t booleanPtrType = {
			static_cast<uint64_t>(LuaTypeQualifiers::Ptr) << 32 |
			static_cast<uint64_t>(kNativeBooleanLuaTypeId)
		};
		constexpr uint64_t stringConstRefType = {
			static_cast<uint64_t>(LuaTypeQualifiers::Ref | LuaTypeQualifiers::Const) << 32 |
			static_cast<uint64_t>(kNativeStringLuaTypeId)
		};

		int kNumberValue = 12;
		int& kNumberRef = kNumberValue;
		const int* kNumberPtrConst = &kNumberValue;
		int* kNumberPtrNull = nullptr;

		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), 12.3f, numberRefType).valid());
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kNumberRef, numberRefType).valid());
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kNumberPtrConst, numberRefType).valid());
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kNumberPtrNull, numberRefType).valid());

		bool kBoolValue = true;
		const bool& kBoolConstRef = kBoolValue;
		bool* kBoolPtr = &kBoolValue;
		bool* kBoolPtrNull = nullptr;

		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), true, booleanPtrType).valid());
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kBoolConstRef, booleanPtrType).valid());
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kBoolPtr, booleanPtrType).valid());
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kBoolPtrNull, booleanPtrType).valid());

		std::string kStrValue = "yo";
		std::string_view kStrViewValue = kStrValue;
		std::string* kStringPtr = &kStrValue;
		const std::string_view& kStringViewRefConst = kStrViewValue;
		const char* kConstCharPtrNull = nullptr;

		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), "bro", stringConstRefType).valid());
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), 'b', stringConstRefType).valid());
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kStrViewValue, stringConstRefType).valid());
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kStringPtr, stringConstRefType).valid());
		CHECK(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kStringViewRefConst, stringConstRefType).valid());
		CHECK_FALSE(LuaFunctionCallHandler::TryMakeArgumentLuaObject(
			state.Data(), kConstCharPtrNull, stringConstRefType).valid());
	}

	SECTION("Mixing native and user types in function call")
	{
		sol::state_view stateView{ state.Data() };

		int intOutput = 0;
		double doubleOutput = 0.0;
		bool boolOutput = false;
		std::string stringOutput;

		stateView["intFnMix"] = [&intOutput](int i, const TestUserStructA& a) {
			intOutput = i;
		};
		stateView["doubleBoolFnMix"] = [&doubleOutput, &boolOutput]
		(double d, const TestUserStructA* a, bool b) {
			doubleOutput = d;
			boolOutput = b;
		};
		stateView["allFnMix"] = [&](TestUserStructA a, double d, int i, std::string s, bool b) {
			intOutput = i;
			doubleOutput = d;
			stringOutput = s;
			boolOutput = b;
		};

		constexpr uint64_t structAValueType = {
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32)
		};
		constexpr uint64_t structAPtrConstType = {
			static_cast<uint64_t>(LuaTypeQualifiers::Const | LuaTypeQualifiers::Ptr) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32)
		};
		constexpr uint64_t structARefConstType = {
			static_cast<uint64_t>(LuaTypeQualifiers::Const | LuaTypeQualifiers::Ref) << 32 |
			static_cast<uint64_t>(TypeInfo<TestUserStructA>::hash32)
		};
		constexpr uint64_t numberType = static_cast<uint64_t>(kNativeNumberLuaTypeId);
		constexpr uint64_t booleanType = static_cast<uint64_t>(kNativeBooleanLuaTypeId);
		constexpr uint64_t stringType = static_cast<uint64_t>(kNativeStringLuaTypeId);

		const std::vector<uint64_t> intFnMixTypes = {
			numberType, structARefConstType
		};
		const std::vector<uint64_t> doubleBoolFnMixTypes = {
			numberType, structAPtrConstType, booleanType
		};
		const std::vector<uint64_t> allFnMixTypes = {
			structAValueType, numberType, numberType, stringType, booleanType
		};

		TestUserStructA structAInstance{};

		{
		sol::function intFnMix = stateView["intFnMix"];
		REQUIRE(intFnMix.valid());

		CHECK(LuaFunctionCallHandler::CallLuaFunctionQualified(
			intFnMix, intFnMixTypes, 354, structAInstance
		).Success());
		CHECK(intOutput == 354);
		intOutput = 0;
		}

		{
		sol::function doubleBoolFnMix = stateView["doubleBoolFnMix"];
		REQUIRE(doubleBoolFnMix.valid());

		CHECK(LuaFunctionCallHandler::CallLuaFunctionQualified(
			doubleBoolFnMix, doubleBoolFnMixTypes, 80.5, true, &structAInstance
		).Success());
		CHECK(doubleOutput == 80.5);
		CHECK(boolOutput == true);
		doubleOutput = 0.0;
		boolOutput = false;
		}

		{
		sol::function allFnMix = stateView["allFnMix"];
		REQUIRE(allFnMix.valid());

		CHECK(LuaFunctionCallHandler::CallLuaFunctionQualified(
			allFnMix, allFnMixTypes, 69.69, TestUserStructA{}, true, 67, "hello"
		).Success());
		CHECK(doubleOutput == 69.69);
		CHECK(intOutput == 67);
		CHECK(boolOutput == true);
		CHECK(stringOutput == "hello");
		}
	}
}