#include "../CatchUtils.h"
#include "../../../scripting/ParsedLuaUserTypeInfo.h"
#include <string>

namespace {

enum TestEnumA
{
	One,
	Two,
	Three
};

struct TestStructA
{
	int i = 0;
	float f = 0.0f;
	std::string s;

	bool operator==(const TestStructA&) const = default;

	void Increment() { ++i; }

	void SetString(const std::string& str) { s = str; }
	void SetString(int x) { s = std::to_string(x); }
};


} // unnamed

TEST_CASE("Parsing UserTypes", "[script][k]")
{
	sol::state state;
	state.open_libraries(sol::lib::base);

	state.new_usertype<TestStructA>("TestStructA", 
		"i", &TestStructA::i, 
		"f", sol::property([](TestStructA& a) -> float& { return a.f; },
						   [](TestStructA& a, float newF) { a.f = newF; }),
		"increment", &TestStructA::Increment,
		"set_string", sol::overload([](TestStructA& a, std::string newS) { a.s = std::move(newS); },
									[](TestStructA& a) { a.s = "overload";}),
		sol::meta_function::equal_to, &TestStructA::operator==);

	REQUIRE(state["TestStructA"] != sol::type::nil);

	ParsedLuaUserTypeInfo userTypeInfo;

	CHECK_FALSE(userTypeInfo.IsCommitted());

	userTypeInfo.ParseUserTypeArgs<TestStructA>("TestStructA",
		"i", &TestStructA::i,
		"f", sol::property([](TestStructA& a) -> float& { return a.f; },
						   [](TestStructA& a, float newF) { a.f = newF; }),
		"increment", &TestStructA::Increment,
		"set_string", sol::overload([](TestStructA& a, std::string newS) { a.s = std::move(newS); },
									[](TestStructA& a) { a.s = "overload"; }),
		sol::meta_function::equal_to, &TestStructA::operator==);

	userTypeInfo.Commit();
	CHECK(userTypeInfo.IsCommitted());

	const auto dataIdx = userTypeInfo.GetDataIndex("TestStructA");
	REQUIRE(dataIdx.IsValid());

	const auto dataIdx2 = userTypeInfo.GetDataIndex<TestStructA>();
	CHECK(dataIdx.value == dataIdx2.value);

	CHECK(userTypeInfo.GetUserTypeNames()[dataIdx] == "TestStructA");
	CHECK(userTypeInfo.GetUserTypeIds()[dataIdx] == TypeInfo<TestStructA>::hash32);

	const auto& fieldNames = userTypeInfo.GetUserTypeFieldNames()[dataIdx];
	const auto& memberKinds = userTypeInfo.GetUserTypeMemberKinds()[dataIdx];

	REQUIRE(fieldNames.size() == 4); // metafunction skipped
	REQUIRE(fieldNames.size() == memberKinds.size());

	using Kind = LuaUsertypeMemberKind;

	CHECK(fieldNames[0] == "i");
	CHECK(memberKinds[0] == Kind::Field);

	CHECK(fieldNames[1] == "f");
	CHECK(memberKinds[1] == Kind::Property);

	CHECK(fieldNames[2] == "increment");
	CHECK(memberKinds[2] == Kind::Method);

	CHECK(fieldNames[3] == "set_string");
	CHECK(memberKinds[3] == Kind::Overload);
}

TEST_CASE("Parsing Enums", "[script][k]")
{
	sol::state state;
	state.open_libraries(sol::lib::base);

	state.new_enum("TestEnumA",
		"One", TestEnumA::One,
		"Two", TestEnumA::Two,
		"Three", TestEnumA::Three);

	ParsedLuaUserTypeInfo userTypeInfo;

	CHECK_FALSE(userTypeInfo.IsCommitted());

	userTypeInfo.ParseUserTypeArgs<TestEnumA>("TestEnumA",
		"One", TestEnumA::One,
		"Two", TestEnumA::Two,
		"Three", TestEnumA::Three);

	userTypeInfo.Commit();
	CHECK(userTypeInfo.IsCommitted());

	const auto dataIdx = userTypeInfo.GetDataIndex("TestEnumA");
	REQUIRE(dataIdx.IsValid());

	const auto dataIdx2 = userTypeInfo.GetDataIndex<TestEnumA>();
	CHECK(dataIdx.value == dataIdx2.value);

	CHECK(userTypeInfo.GetUserTypeNames()[dataIdx] == "TestEnumA");
	CHECK(userTypeInfo.GetUserTypeIds()[dataIdx] == TypeInfo<TestEnumA>::hash32);

	const auto& fieldNames = userTypeInfo.GetUserTypeFieldNames()[dataIdx];
	const auto& memberKinds = userTypeInfo.GetUserTypeMemberKinds()[dataIdx];

	REQUIRE(fieldNames.size() == 3);
	REQUIRE(fieldNames.size() == memberKinds.size());

	using Kind = LuaUsertypeMemberKind;

	CHECK(fieldNames[0] == "One");
	CHECK(memberKinds[0] == Kind::Field);

	CHECK(fieldNames[1] == "Two");
	CHECK(memberKinds[1] == Kind::Field);

	CHECK(fieldNames[2] == "Three");
	CHECK(memberKinds[2] == Kind::Field);
}