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
        .f = 3.3f,
        .group = "group3"
    };
    TestStruct testStruct3Temp = testStruct3;

    /* Reserve + Capacity */
    soa.Reserve(50);
    CHECK(soa.Capacity() >= 50);

    /* PushBack + Size */
    const size_t id1 = soa.PushBack(testStruct1);
    REQUIRE(soa.Size() == 1);

    const size_t id2 = soa.PushBack(testStruct2);
    REQUIRE(soa.Size() == 2);

    const size_t id3 = soa.PushBack(std::move(testStruct3Temp)); // can move into it
    REQUIRE(soa.Size() == 3);

    /* MakeSlice */
    // joins all the members for a particular Id into a ne
    auto slice1 = soa.MakeSlice(id1);
    auto slice2 = soa.MakeSlice(id2);
    auto slice3 = soa.MakeSlice(id3);

    CHECK(slice1 == testStruct1);
    CHECK(slice2 == testStruct2);
    CHECK(slice3 == testStruct3);

    /* GetView */
    // can select arbitrary amount of members
    auto [name1, f1] = soa.GetView<&TestStruct::name, &TestStruct::f>(id1);
    CHECK(name1 == testStruct1.name);
    CHECK(f1 == testStruct1.f);

    // don't have to get the members in struct order
    auto [x2, name2, grp2] = soa.GetView<&TestStruct::x, &TestStruct::name,
        &TestStruct::group>(id2);
    CHECK(x2 == testStruct2.x);
    CHECK(name2 == testStruct2.name);
    CHECK(grp2 == testStruct2.group);

    // providing no template args will provide all members in struct order
    auto [name3, x3, f3, grp3] = soa.GetView(id3);

    CHECK(grp3 == testStruct3.group);
    CHECK(f3 == testStruct3.f);
    CHECK(x3 == testStruct3.x);
    CHECK(name3 == testStruct3.name);

    /* GetView with 1 template argument gives direct reference to that member (mutable) */
    auto& name3Ref = soa.GetView<&TestStruct::name>(id3);

    // make sure its not a tuple with 1 string element and actually is a ref to the string itself
    static_assert(std::same_as<std::string&, decltype(name3Ref)>);

    // can mutate the ref
    name3Ref = "new name";
    CHECK(soa.GetView<&TestStruct::name>(id3) == "new name");
    name3Ref = testStruct3.name;
    CHECK(soa.GetView<&TestStruct::name>(id3) == testStruct3.name);

    /* Const version */
    const auto& soaConst = soa;
    const auto& name2Ref = soaConst.GetView<&TestStruct::name>(id2);

    // make sure its not a tuple with 1 string element and is actually a const ref to the string itself
    static_assert(std::same_as<const std::string&, decltype(name2Ref)>);

    CHECK(name2Ref == testStruct2.name);

    /* TryGetView */
    auto view1Optional = soa.TryGetView<&TestStruct::name, &TestStruct::f>(id1);
    REQUIRE(view1Optional.has_value());

    auto [view1Name, view1F] = *view1Optional;
    CHECK(view1Name == testStruct1.name);
    CHECK(view1F == testStruct1.f);

    // can mutate the contents of the optional
    static_assert(std::same_as<float&, decltype(view1F)>);
    view1F = 10.0f;
    CHECK(soa.GetView<&TestStruct::f>(id1) == 10.0f);
    view1F = testStruct1.f;
    CHECK(soa.GetView<&TestStruct::f>(id1) == testStruct1.f);

    // empty optional if bad id
    constexpr size_t badId = 10000;
    REQUIRE(badId > soa.Size());
    auto badView = soa.TryGetView<&TestStruct::x, &TestStruct::group>(10000);
    REQUIRE(!badView.has_value());

    /* ForEach */
    // providing no template args will provide all members in struct order
    size_t count;
    for (auto [nm, x, f, grp] : soa.ForEach())
    {
        ++count;
        CHECK(!nm.empty());
        CHECK(x > 0);
        CHECK(f > 0.0f);
        CHECK(!grp.empty());
    }

    CHECK(count == soa.Size());
    count = 0;

    // can specify which members to iterate over in any order with template args
    for (auto [f, nm, x] : soa.ForEach<&TestStruct::f, &TestStruct::name, &TestStruct::x>())
    {
        ++count;
        CHECK(f > 0.0f);
        CHECK(!nm.empty());
        CHECK(x > 0);
    }

    CHECK(count == soa.Size());
}