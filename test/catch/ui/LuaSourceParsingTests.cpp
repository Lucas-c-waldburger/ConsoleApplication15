#include "../CatchUtils.h"
#include "../../ui/lua_text_editor/LuaSourceParser.h"
#include "../../ui/lua_text_editor/LuaSourceDocumentSymbol.h"
#include "../../ui/lua_text_editor/LuaSourceDocumentParser.h"
#include "../../ui/lua_text_editor/LuaSourceDocumentParseSignaturesTable.h"
#include "../../ui/lua_text_editor/LuaSourceDocumentParseTableInfo.h"
#include "../../../scripting/LuaStateManager.h"
#include "../../../file/FilePathUtility.h"

enum class InstrumentType
{
	Guitar,
	Bass,
	Drums,
	Keyboard
};

enum class VenueSize
{
	Small,
	Medium,
	Large
};

struct Instrument
{
	Instrument() = default;
	Instrument(InstrumentType t, int a, float p) : type(t), age(a), price(p) {}

	int age = 0;
	float price = 0.0f;
	InstrumentType type = InstrumentType::Guitar;

	void SetPrice(float newPrice) { price = newPrice; }
	float GetPrice() const { return price; }
};

struct Venue
{
	Venue() = default;
	explicit Venue(VenueSize s) : size(s) {}

	std::string name;
	VenueSize size = VenueSize::Small;
	bool allowsSmoking = false;

	void SetSize(VenueSize newSize) { size = newSize; }
	void SetSize(const std::string& sizeStr)
	{
		if (sizeStr == "Medium") { size = VenueSize::Medium; }
		else if (sizeStr == "Large") { size = VenueSize::Large; }
		else { size = VenueSize::Small; }
	}
};

struct Gig
{
	Venue venue;
	std::vector<Instrument> instruments;

	void AddInstrument(const Instrument& instrument) { instruments.push_back(instrument); }
};

namespace {

Result<std::string> LoadTestScript(std::string_view scriptFilename)
{
	namespace fs = std::filesystem;

	auto path = FilePathUtility::GetRootPath() /
		fs::path("test/catch/test_scripts") / fs::path(scriptFilename);

	REQUIRE(fs::exists(path));
	REQUIRE(fs::is_regular_file(path));

	std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		return MAKE_ERROR_FMT("Could not open file at path '{}'", path.string());
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string content;
	content.resize(size);

	if (!file.read(content.data(), size))
	{
		return MAKE_ERROR_FMT("Error reading file at path '{}'", path.string());
	}

	return content;
}

void RegisterTestTypes(LuaStateManager& stateManager)
{
	CHECK(stateManager.AutoRegister<InstrumentType>("InstrumentType"));
	CHECK(stateManager.AutoRegister<VenueSize>("VenueSize"));

	CHECK(stateManager.NewUserType<Instrument>("Instrument",
		sol::constructors<Instrument(), Instrument(InstrumentType, int, float)>(),
		"age", &Instrument::age,
		"type", &Instrument::type,
		"price", sol::property(&Instrument::GetPrice, &Instrument::SetPrice)));

	CHECK(stateManager.NewUserType<Venue>("Venue",
		sol::constructors<Venue(), Venue(VenueSize)>(),
		"name", &Venue::name,
		"size", &Venue::size,
		"allowsSmoking", &Venue::allowsSmoking,
		"setSize", sol::overload(
			static_cast<void (Venue::*)(VenueSize)>(&Venue::SetSize),
			static_cast<void (Venue::*)(const std::string&)>(&Venue::SetSize))));

	CHECK(stateManager.NewUserType<Gig>("Gig",
		"venue", &Gig::venue,
		"instruments", &Gig::instruments,
		"addInstrument", &Gig::AddInstrument,
		"removeInstrument", [](Gig& gig, size_t idx) {
			if (idx < gig.instruments.size()) {
				gig.instruments.erase(gig.instruments.begin() + idx);
			}
		}));
}


} // unnamed

TEST_CASE("Lua Source Parsing Test", "[ui][script]")
{
	LuaStateManager stateManager;
	RegisterTestTypes(stateManager);

	const auto& userTypeInfo = stateManager.GetLuaUserTypeInfo();

	const auto instrumentTypeIdx = userTypeInfo.GetDataIndex("InstrumentType");
	REQUIRE(instrumentTypeIdx.IsValid());

	const auto venueSizeIdx = userTypeInfo.GetDataIndex("VenueSize");
	REQUIRE(venueSizeIdx.IsValid());
	const auto instrumentIdx = userTypeInfo.GetDataIndex("Instrument");
	REQUIRE(instrumentIdx.IsValid());
	const auto venueIdx = userTypeInfo.GetDataIndex("Venue");
	REQUIRE(venueIdx.IsValid());
	const auto gigIdx = userTypeInfo.GetDataIndex("Gig");
	REQUIRE(gigIdx.IsValid());
	auto scriptStrResult = LoadTestScript("source_parser_test.lua");
	REQUIRE(scriptStrResult.Success());
	
	const auto& source = scriptStrResult.GetValue();

	auto parser = ui::CreateUniqueTSParserPtr();
	REQUIRE(parser);
	const auto* language = tree_sitter_lua();
	REQUIRE(ts_parser_set_language(parser.get(), language));

	auto tree = ui::CreateUniqueTSTreePtr(
		parser.get(),
		nullptr,
		source);
	REQUIRE(tree);

	auto cursor = ui::TreeCursor(ts_tree_root_node(tree.get()), source);
	REQUIRE(cursor);

	auto sourceDoc = ui::LuaSourceDocument::Parse(cursor, userTypeInfo);

	SECTION("Check Table Info")
	{
		CHECK(sourceDoc.tableInfo.functionTableName == "music_table");
		CHECK(sourceDoc.tableInfo.metaTableName == "meta_stuff");
	}

	SECTION("Check Function Blocks")
	{
		REQUIRE(sourceDoc.functionBlocks.size() == 3);

		const auto& block1 = sourceDoc.functionBlocks[0];
		CHECK(block1.functionName == "isGuitar");
		REQUIRE(block1.parameterNames.size() == 1);
		CHECK(block1.parameterNames[0] == "instrument");
		REQUIRE(block1.parameterTypes.size() == 1);
		CHECK(block1.parameterTypes[0] == "const Instrument&");

		const auto& block2 = sourceDoc.functionBlocks[1];
		CHECK(block2.functionName == "addKeyboard");
		REQUIRE(block2.parameterNames.size() == 1);
		CHECK(block2.parameterNames[0] == "gig");
		REQUIRE(block2.parameterTypes.size() == 1);
		CHECK(block2.parameterTypes[0] == "Gig&");

		const auto& block3 = sourceDoc.functionBlocks[2];
		CHECK(block3.functionName == "setSizeForSmoking");
		REQUIRE(block3.parameterNames.size() == 2);
		CHECK(block3.parameterNames[0] == "venue");
		CHECK(block3.parameterNames[1] == "size");
		REQUIRE(block3.parameterTypes.size() == 2);
		CHECK(block3.parameterTypes[0] == "Venue*");
		CHECK(block3.parameterTypes[1] == "VenueSize");
	}

	SECTION("Check Index Expressions")
	{
		using enum ui::LuaSourceDocument::IndexExpressionType;

		REQUIRE(sourceDoc.indexExpressions.size() == 11);

		const auto& expr1 = sourceDoc.indexExpressions[0];
		CHECK(expr1.lhsText == "music_table");
		CHECK(expr1.rhsText == "isGuitar");
		CHECK(expr1.type == Dot);

		const auto& expr2 = sourceDoc.indexExpressions[1];
		CHECK(expr2.lhsText == "instrument");
		CHECK(expr2.rhsText == "type");
		CHECK(expr2.type == Dot);
		CHECK(expr2.location > expr1.location);

		const auto& expr3 = sourceDoc.indexExpressions[2];
		CHECK(expr3.lhsText == "InstrumentType");
		CHECK(expr3.rhsText == "Guitar");
		CHECK(expr3.type == Dot);
		CHECK(expr3.location > expr2.location);

		const auto& expr4 = sourceDoc.indexExpressions[3];
		CHECK(expr4.lhsText == "music_table");
		CHECK(expr4.rhsText == "addKeyboard");
		CHECK(expr4.type == Dot);
		CHECK(expr4.location > expr3.location);

		const auto& expr5 = sourceDoc.indexExpressions[4];
		CHECK(expr5.lhsText == "InstrumentType");
		CHECK(expr5.rhsText == "Keyboard");
		CHECK(expr5.type == Dot);
		CHECK(expr5.location > expr4.location);

		const auto& expr6 = sourceDoc.indexExpressions[5];
		CHECK(expr6.lhsText == "gig");
		CHECK(expr6.rhsText == "addInstrument");
		CHECK(expr6.type == Method);
		CHECK(expr6.location > expr5.location);

		const auto& expr7 = sourceDoc.indexExpressions[6];
		CHECK(expr7.lhsText == "music_table");
		CHECK(expr7.rhsText == "setSizeForSmoking");
		CHECK(expr7.type == Dot);
		CHECK(expr7.location > expr6.location); 

		const auto& expr8 = sourceDoc.indexExpressions[7];
		CHECK(expr8.lhsText == "venue");
		CHECK(expr8.rhsText == "allowsSmoking");
		CHECK(expr8.type == Dot);
		CHECK(expr8.location > expr7.location);

		const auto& expr9 = sourceDoc.indexExpressions[8];
		CHECK(expr9.lhsText == "venue");
		CHECK(expr9.rhsText == "setSize");
		CHECK(expr9.type == Method);
		CHECK(expr9.location > expr8.location);

		const auto& expr10 = sourceDoc.indexExpressions[9];
		CHECK(expr10.lhsText == "venue");
		CHECK(expr10.rhsText == "setSize");
		CHECK(expr10.type == Method);
		CHECK(expr10.location > expr9.location);

		const auto& expr11 = sourceDoc.indexExpressions[10];
		CHECK(expr11.lhsText == "VenueSize");
		CHECK(expr11.rhsText == "Medium");
		CHECK(expr11.type == Dot);
		CHECK(expr11.location > expr10.location);
	}

	SECTION("Check Errors")
	{
		CHECK(sourceDoc.errors.empty());
	}
}

TEST_CASE("LuaSourceDocumentSymbol Tests", "[ui][script][o]")
{
	LuaStateManager stateManager;
	RegisterTestTypes(stateManager);

	const auto& userTypeInfo = stateManager.GetLuaUserTypeInfo();

	const auto instrumentTypeIdx = userTypeInfo.GetDataIndex("InstrumentType");
	REQUIRE(instrumentTypeIdx.IsValid());

	const auto venueSizeIdx = userTypeInfo.GetDataIndex("VenueSize");
	REQUIRE(venueSizeIdx.IsValid());
	const auto instrumentIdx = userTypeInfo.GetDataIndex("Instrument");
	REQUIRE(instrumentIdx.IsValid());
	const auto venueIdx = userTypeInfo.GetDataIndex("Venue");
	REQUIRE(venueIdx.IsValid());
	const auto gigIdx = userTypeInfo.GetDataIndex("Gig");
	REQUIRE(gigIdx.IsValid());
	auto scriptStrResult = LoadTestScript("symbol_parser_test.lua");
	REQUIRE(scriptStrResult.Success());

	const auto& source = scriptStrResult.GetValue();

	auto parser = ui::CreateUniqueTSParserPtr();
	REQUIRE(parser);
	const auto* language = tree_sitter_lua();
	REQUIRE(ts_parser_set_language(parser.get(), language));

	auto tree = ui::CreateUniqueTSTreePtr(
		parser.get(),
		nullptr,
		source);
	REQUIRE(tree);

	auto cursor = ui::TreeCursor(ts_tree_root_node(tree.get()), source);
	REQUIRE(cursor);

	ui::LuaSourceDocumentSymbolParseContext ctx{
		.userTypeInfo = userTypeInfo
	};

	const bool tableInfoSuccess = ui::ParseTableInfo(cursor, ctx);
	CHECK(tableInfoSuccess);

	CHECK(ctx.tableInfo.functionTableName == "music_table");
	CHECK(ctx.tableInfo.metaTableName == "meta_stuff");

	cursor.Reset();
	const bool sigTableSuccess = ui::ParseSignaturesTable(cursor, ctx);
	CHECK(sigTableSuccess);

	const auto& sigTable = ctx.signaturesTable;
	REQUIRE(sigTable.functionNames.size() == 3);
	CHECK(sigTable.functionNames[0] == "isGuitar");
	CHECK(sigTable.functionNames[1] == "addKeyboard");
	CHECK(sigTable.functionNames[2] == "setSizeForSmoking");

	REQUIRE(sigTable.parameterTypes.size() == 3);
	const auto& isGuitarParams = sigTable.parameterTypes[0];
	REQUIRE(isGuitarParams.size() == 1);
	CHECK(isGuitarParams[0] == TypeInfo<Instrument>::hash32);

	const auto& addKeyboardParams = sigTable.parameterTypes[1];
	REQUIRE(addKeyboardParams.size() == 1);
	CHECK(addKeyboardParams[0] == TypeInfo<Gig>::hash32);

	const auto& setSizeForSmokingParams = sigTable.parameterTypes[2];
	REQUIRE(setSizeForSmokingParams.size() == 2);
	CHECK(setSizeForSmokingParams[0] == TypeInfo<Venue>::hash32);
	CHECK(setSizeForSmokingParams[1] == TypeInfo<VenueSize>::hash32);

	cursor.Reset();
	const bool symbolsSuccess = ui::ParseLuaSourceDocumentSymbols(cursor, ctx);
	CHECK(symbolsSuccess);
}