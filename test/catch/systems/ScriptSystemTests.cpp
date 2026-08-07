#include "../CatchUtils.h"
#include "../../../file/FilePathUtility.h"
#include "../../../systems/ScriptSystem.h"
#include "../../../scripting/user_types/EntityUserType.h"

namespace {

std::filesystem::path MakeScriptTestPath(std::string_view scriptFilename)
{
	namespace fs = std::filesystem;

	auto path = FilePathUtility::GetRootPath() /
		fs::path("test/catch/test_scripts") / fs::path(scriptFilename);

	return path;
}

} // unnamed

TEST_CASE("ScriptSystem Tests", "[sys][script]")
{
	ScriptSystem scriptSystem{};

	scriptSystem.GetState().Init<Entity>();

	auto entityTestFilepath = MakeScriptTestPath("entity_test.lua");
	REQUIRE(std::filesystem::exists(entityTestFilepath));

	auto addTableResult = scriptSystem.AddTable(entityTestFilepath);
	REQUIRE_RESULT(addTableResult);

	auto tableId = addTableResult.GetValue();
	CHECK(tableId != std::numeric_limits<ScriptTable::TableId>::max());

	CHECK(scriptSystem.GetTableFilepath(tableId) == entityTestFilepath.string());

	const bool registeredChangeName = 
		scriptSystem.RegisterTableFunction<void(Entity&)>(tableId, "changeName");
	CHECK(registeredChangeName);

	const bool registeredChangePos =
		scriptSystem.RegisterTableFunction<void(Entity&)>(tableId, "changePos");
	CHECK(registeredChangePos);

	auto e = ECS::CreateEntity();
	REQUIRE(e.IsValid());
	e.AddComponent(Name{});
	e.AddComponent(Transform{});

	auto tableView = scriptSystem.GetTableView(tableId);
	CHECK(tableView.IsValid());

	auto& script = e.AddComponent(Script{ .table = std::move(tableView) });

	CHECK(script.table.Contains<void(Entity&)>("changeName"));
	CHECK(script.table.Contains<void(Entity&)>("changePos"));

	CHECK(script.table.GetTableId() == tableId);

	auto changeNameFn = script.table["changeName"];
	CHECK(changeNameFn.IsValid());
	CHECK(changeNameFn.Matches<void(Entity&)>());
	CHECK(changeNameFn.MatchesArguments<Entity&>());

	changeNameFn(e);

	REQUIRE(e.HasComponent<Name>());
	CHECK(e.GetComponent<Name>() == "changed");

	auto changePosFn = script.table["changePos"];
	CHECK(changePosFn.IsValid());
	CHECK(changePosFn.Matches<void(Entity&)>());
	CHECK(changePosFn.MatchesArguments<Entity&>());

	changePosFn(e);

	REQUIRE(e.HasComponent<Transform>());
	CHECK(e.GetComponent<Transform>().position.x == 500.0f);
	CHECK(e.GetComponent<Transform>().position.y == 500.0f);
}

TEST_CASE("ScriptSystem::RemoveTable", "[sys][script][a]")
{
	ScriptSystem scriptSystem{};

	scriptSystem.GetState().Init<Entity>();

	auto testFilepath1 = MakeScriptTestPath("entity_test.lua");
	REQUIRE(std::filesystem::exists(testFilepath1));
	auto testFilepath2 = MakeScriptTestPath("empty.lua");
	REQUIRE(std::filesystem::exists(testFilepath2));

	auto addTable1Result = scriptSystem.AddTable(testFilepath1);
	REQUIRE_RESULT(addTable1Result);
	auto addTable2Result = scriptSystem.AddTable(testFilepath2);
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