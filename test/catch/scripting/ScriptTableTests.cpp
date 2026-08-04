#include "../CatchUtils.h"
#include "../../../scripting/ScriptTable.h"
#include <string>

TEST_CASE("ScriptSignature Tests", "[script][a]")
{
	auto sig1 = ScriptSignature::Create<int(float, double)>();
	auto sig2 = ScriptSignature::Create<void(const std::string&)>();
	auto sig3 = ScriptSignature::Create<std::vector<float>()>();
	auto sig4 = ScriptSignature{};

	CHECK(sig1.IsValid());
	CHECK(sig2.IsValid());
	CHECK(sig3.IsValid());
	CHECK_FALSE(sig4.IsValid());

	CHECK(sig1.ReturnType() == GetScriptArgumentId<int>());
	CHECK(sig2.ReturnType() == GetScriptArgumentId<void>());
	CHECK(sig3.ReturnType() == GetScriptArgumentId<std::vector<float>>());

	CHECK(sig1.ArgumentCount() == 2);
	CHECK(sig2.ArgumentCount() == 1);
	CHECK(sig3.ArgumentCount() == 0);

	CHECK(sig1.ContainsArgument<float>());
	CHECK(sig1.ContainsArgument<double>());
	CHECK_FALSE(sig1.ContainsArgument<int>());
	CHECK(sig2.ContainsArgument<const std::string&>());
	CHECK_FALSE(sig2.ContainsArgument<std::string>());
	CHECK_FALSE(sig3.ContainsArgument<int>());

	CHECK(sig1.Argument(0) == GetScriptArgumentId<float>());
	CHECK(sig1.Argument(1) == GetScriptArgumentId<double>());
	CHECK(sig1.Argument(2) == kInvalidScriptArgumentId);
	CHECK(sig2.Argument(0) == GetScriptArgumentId<const std::string&>());
	CHECK(sig2.Argument(1) == kInvalidScriptArgumentId);
	CHECK(sig3.Argument(0) == kInvalidScriptArgumentId);

	CHECK(sig1.IndexOfArgument<float>() == 0);
	CHECK(sig1.IndexOfArgument<double>() == 1);
	CHECK(sig1.IndexOfArgument<float*>() == -1);
	CHECK(sig2.IndexOfArgument<const std::string&>() == 0);
	CHECK(sig2.IndexOfArgument<char>() == -1);
	CHECK(sig3.IndexOfArgument<long>() == -1);

	CHECK(sig1.Matches<int(float, double)>());
	CHECK_FALSE(sig1.Matches<int(const float&, double)>());

	CHECK(sig2.Matches<void(const std::string&)>());
	CHECK_FALSE(sig2.Matches<int(const std::string&)>());
	CHECK(sig3.Matches<std::vector<float>()>());

	CHECK(sig1.MatchesArguments<float, double>());
	CHECK_FALSE(sig1.MatchesArguments<double, float>());
	CHECK(sig2.MatchesArguments<const std::string&>());
	CHECK_FALSE(sig2.MatchesArguments<const std::string>());
	CHECK_FALSE(sig2.MatchesArguments<>());
	CHECK(sig3.MatchesArguments<>());
}

struct StructA { int valueInt = 0; };
struct StructB { std::string valueString; };
struct StructC { double valueDouble = 0.0; uint8_t valueUint = 0; };

TEST_CASE("ScriptTable Tests", "[script][a]")
{
	sol::state state{};

	state.open_libraries(sol::lib::base);

	state.new_usertype<StructA>("StructA", sol::constructors<StructA()>(), 
								"valueInt", &StructA::valueInt);
	state.new_usertype<StructB>("StructB", sol::constructors<StructB()>(),
								"valueString", &StructB::valueString);
	state.new_usertype<StructC>("StructC", sol::constructors<StructC()>(),
								"valueDouble", &StructC::valueDouble,
								"valueUint", &StructC::valueUint);

	static constexpr std::string_view kLuaStr = R"(
		local table = {}

		function table.makeStructA()
		    local a = StructA.new()
			a.valueInt = 5
			return a
		end

		function table.getStructBValue(b)
			return b.valueString
		end
		
		function table.editStructC(c)
		    c.valueDouble = 69.0
			c.valueUint = 65
		end
		
		return table
	)";

	sol::protected_function_result result = state.script(kLuaStr);
	REQUIRE(result.valid());
	REQUIRE(result.get_type() == sol::type::table);

	auto table = ScriptTable::Create(result.get<sol::table>());

	CHECK(table.GetTableId() != std::numeric_limits<ScriptTable::TableId>::max());

	const bool registeredAFn = table.RegisterFunction<StructA()>("makeStructA");
	CHECK(registeredAFn);
	const bool registeredBFn = table.RegisterFunction<std::string(const StructB&)>("getStructBValue");
	CHECK(registeredBFn);
	const bool registeredCFn = table.RegisterFunction<void(StructC&)>("editStructC");
	CHECK(registeredCFn);
	const bool badRegister = table.RegisterFunction<int(float)>("badRegister");
	CHECK_FALSE(badRegister);

	CHECK(table.Contains<StructA()>("makeStructA"));
	CHECK(table.Contains<std::string(const StructB&)>("getStructBValue"));
	CHECK(table.Contains<void(StructC&)>("editStructC"));
	CHECK_FALSE(table.Contains<void(const StructC&)>("editStructC"));

	auto makeStructA = table["makeStructA"];
	CHECK(makeStructA.IsValid());
	CHECK(makeStructA.Matches<StructA()>());
	CHECK(makeStructA.MatchesArguments<>());

	StructA structA = makeStructA();
	CHECK(structA.valueInt == 5);

	auto getStructBValue = table["getStructBValue"];
	CHECK(getStructBValue.IsValid());
	CHECK(getStructBValue.Matches<std::string(const StructB&)>());
	CHECK(getStructBValue.MatchesArguments<const StructB&>());

	StructB structB{ .valueString = "ayyLmao" };
	std::string structBVal = getStructBValue(structB);
	CHECK(structBVal == "ayyLmao");

	auto editStructC = table["editStructC"];
	CHECK(editStructC.IsValid());
	CHECK(editStructC.Matches<void(StructC&)>());
	CHECK(editStructC.MatchesArguments<StructC&>());

	StructC structC{};
	editStructC(structC);
	CHECK(structC.valueDouble == 69.0);
	CHECK(structC.valueUint == 65);

	auto badFn = table["badFn"];
	CHECK_FALSE(badFn.IsValid());
}