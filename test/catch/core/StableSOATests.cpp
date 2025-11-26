#include "../CatchUtils.h"
#include "../../../core/StableSOA.h"


namespace {

struct TestStruct
{
	std::string name;
	int x;
	float f;
	std::string group;

    bool operator==(const TestStruct&) const = default;
};

using TestSOA = StableSOA<
	TestStruct,
	&TestStruct::name, 
	&TestStruct::x, 
	&TestStruct::f, 
	&TestStruct::group
>;

} // unnamed


TEST_CASE("StableSOA tests", "[core]")
{
	TestSOA soa{};

    REQUIRE(TestSOA::MemberCount == 4);

    TestStruct testStruct1{
        .name = "name1",
        .x = 1,
        .f = 1.1f,
        .group = "group1"
    };
    TestStruct testStruct2{
        .name = "name2",
        .x = 2,
        .f = 2.2f,
        .group = "group2"
    };
    TestStruct testStruct3{
        .name = "name3",
        .x = 3,
        .f = 3.3,
        .group = "group3"
    };
    TestStruct testStruct3Temp = testStruct3;

    // Reserve + Capacity
    soa.Reserve(50);
    CHECK(soa.Capacity() >= 50);

    // PushBack + Size
    const size_t id1 = soa.PushBack(testStruct1);
    REQUIRE(soa.Size() == 1);

    const size_t id2 = soa.PushBack(testStruct2);
    REQUIRE(soa.Size() == 2);

    const size_t id3 = soa.PushBack(std::move(testStruct3Temp)); // can move into it
    REQUIRE(soa.Size() == 3);

    // MakeSlice
    auto slice1 = soa.MakeSlice(id1);
    auto slice2 = soa.MakeSlice(id2);
    auto slice3 = soa.MakeSlice(id3);

    CHECK(slice1 == testStruct1);
    CHECK(slice2 == testStruct2);
    CHECK(slice3 == testStruct3);

    // GetView
    auto [name1, f1] = soa.GetView<&TestStruct::name, &TestStruct::f>(id1);
    CHECK(name1 == testStruct1.name);
    CHECK(f1 == testStruct1.f);

    auto [x2, name2, grp2] = soa.GetView<&TestStruct::x, &TestStruct::name, 
                                         &TestStruct::group>(id2);
    CHECK(x2 == testStruct2.x);
    CHECK(name2 == testStruct2.name);
    CHECK(grp2 == testStruct2.group);

    auto [grp3, f3, x3, name3] = soa.GetView(id3);

    CHECK(grp3 == testStruct3.group);
    CHECK(f3 == testStruct3.f);
    CHECK(x3 == testStruct3.x);
    CHECK(name3 == testStruct3.name);
}