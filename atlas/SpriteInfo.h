#pragma once
#include <string>
#include <vector>
#include "../core/commonObjects.h"
#include "../core/TypeUtils.h"

//template <typename T>
//struct add_lvalue_const_reference { using type = const T&; };
//
//template <typename T>
//using add_lvalue_const_reference_t = add_lvalue_const_reference<T>::type;
//
//template <template <typename> class Wrap>
//class SpriteInfoTemplate
//{
//	Wrap<std::string> spriteName;
//	Wrap<std::string> filepath;
//	Wrap<std::string> seriesName;
//	Wrap<size_t> seriesIndex;
//};
//
//using SpriteInfoSOA = SpriteInfoTemplate<std::vector>;
//using SpriteInfo = SpriteInfoTemplate<add_lvalue_const_reference_t>;


namespace detail {

} // detail

//struct SpriteInfo
//{
//	std::vector<std::string> spriteName;
//	std::vector<std::string> filepath;
//	std::vector<std::string> seriesName;
//	std::vector<size_t> seriesIndex;
//
//    template<auto... MemberPtrs>
//        requires ((std::is_member_object_pointer_v<decltype(MemberPtrs)> && ...))
//    auto GetAt(size_t index)
//    {
//        return std::tuple<decltype((*this).*MemberPtrs[index])...>(
//            ((*this).*MemberPtrs[index])...
//        );
//    }
//
//    template<auto... MemberPtrs>
//        requires ((std::is_member_object_pointer_v<decltype(MemberPtrs)> && ...))
//    auto GetAt(size_t index) const
//    {
//        return std::tuple<decltype(((*this).*MemberPtrs[index]))...>(
//            ((*this).*MemberPtrs[index])...
//        );
//    }
//};

//template <typename Derived>
//struct SOAInterface
//{
//    template<auto... MemberPtrs>
//        requires ((std::is_member_object_pointer_v<decltype(MemberPtrs)> && ...))
//    auto GetAt(size_t index)
//    {
//        auto* self = static_cast<Derived*>(this);
//
//        return std::tuple<
//            decltype((((*self).*MemberPtrs)[index]))...
//        >(
//            (((*self).*MemberPtrs)[index])...
//        );
//    }
//
//    template<auto... MemberPtrs>
//        requires ((std::is_member_object_pointer_v<decltype(MemberPtrs)> && ...))
//    auto GetAt(size_t index) const
//    {
//        const auto* self = static_cast<const Derived*>(this);
//
//        return std::tuple<
//            decltype((((*self).*MemberPtrs)[index]))...
//        >(
//            (((*self).*MemberPtrs)[index])...
//        );
//    }
//};



//using SpriteNameTT = TaggedType<"spriteName", std::string>;
//using FilepathTT = TaggedType<"filepath", std::string>;
//using SeriesNameTT = TaggedType<"seriesName", std::string>;
//using SeriesIndexTT = TaggedType<"seriesIndex", size_t>;


//template <FixedString Name, typename... TTs>
//struct index_of_name;
//
//template <FixedString Name, FixedString HeadName, typename HeadT, typename... Tail>
//struct index_of_name<Name, TaggedType<HeadName, HeadT>, Tail...>
//{
//    static constexpr size_t value =
//        (Name == HeadName) ? 0 : (1 + index_of_name<Name, Tail...>::value);
//};
//
//template <FixedString Name, typename... TTs>
//static constexpr size_t index_of_name_v = index_of_name<Name, TTs...>::value;

template <FixedString VarName, typename T>
struct SOAMember {};

//template <FixedString Name, size_t Idx, FixedString...Rest>
//struct index_of_name_impl
//{
//    static constexpr size_t value = index_of_name_impl<Name, Idx + 1>;
//};
//
//// base case when pack exhausted
//template <FixedString Name, size_t Idx>
//struct index_of_name_impl<Name, Idx> {
//    static_assert(Idx != Idx, "Name not found in index_of_name_impl!");
//};
//
//template <FixedString Name, size_t Idx, FixedString Last>
//struct index_of_name_impl<Name, Idx, Last> {
//    static constexpr size_t value = (Name == Current)
//        ? Idx
//        : index_of_name_impl<Name, Idx + 1>::value;
//};
//
//template <FixedString Name, size_t Idx, FixedString Current, FixedString...Rest>
//struct index_of_name_impl<Name, Idx, Current, Rest...> {
//    static constexpr size_t value = (Name == Current)
//        ? Idx
//        : index_of_name_impl<Name, Idx + 1, Rest...>::value;
//};

template <FixedString Name, FixedString... Names>
    //requires (Name == Names || ...)
consteval std::size_t index_of_name_eval() {
    std::size_t idx = 0;
    bool found = ((Name == Names ? true : (++idx, false)) || ...);
    if (!found) return std::numeric_limits<size_t>::max();
    return idx;
}


static_assert(index_of_name_eval<"AD", "BD", "AD", "CD">() == 1);

template <typename Tup, FixedString Name, FixedString...Names> 
    //requires (Name == Names || ...)
struct type_at_name
{
    static constexpr size_t idx = index_of_name_eval<Name, Names...>();
    using type = std::tuple_element_t<idx, Tup>;
};

template <typename Tup, FixedString Name, FixedString...Names>
using type_at_name_t = typename type_at_name<Tup, Name, Names...>::type;

static_assert(std::same_as<type_at_name_t<std::tuple<int, float, double>,
    "Bx", "Ax", "Bx", "Cx">, float>);

//template <typename T>
//struct strip_wrapper_first_type { using type = T; };
//
//template <template <typename...> class Wrap, typename T, typename...Ts>
//struct strip_wrapper_first_type<Wrap<T, Ts...>> { using type = T; };
//
//template <typename T>
//using strip_wrapper_first_type_t = strip_wrapper_first_type<T>::type;


template <typename...Members> requires (sizeof...(Members) > 0)
class StableSOA;

template <FixedString...Names, typename...Ts>
class StableSOA<SOAMember<Names, Ts>...>
{
private:
    using TupleType = std::tuple<std::vector<Ts>...>;

    TupleType variables_;

    /* Helper traits */
    //template <FixedString Name> requires (Name == Names || ...)
    //static constexpr size_t IndexOfName = index_of_name_v<Name, Names...>;
    //template <FixedString Name>
    //static constexpr size_t IndexOfName() {
    //    constexpr size_t value = index_of_name_v<Name, Names...>;
    //    return value;
    //}

    //template <FixedString Name> requires (Names == Names || ...)
    //static consteval size_t IndexOfName() {
    //    return index_of_name_eval<Name, Names...>();
    //}

    //template <FixedString Name> requires (Name == Names || ...)
    //static constexpr size_t IndexOfName = index_of_name_eval<Name, Names...>();

    //template <FixedString Name> requires (Name == Names || ...)
    //using VecTypeAtName = std::tuple_element_t<IndexOfName<Name>, TupleType>;

    //template <FixedString Name> requires (Name == Names || ...)
    //using VecTypeAtName = std::tuple_element_t<index_of_name_eval<Name, Names...>(), TupleType>;

    //template <FixedString Name>// requires (Name == Names || ...)
    //using ElemTypeAtName = typename VecTypeAtName<Name>::template value_type;

    /* Internal helper methods */
    template <FixedString Name>// requires (Name == Names || ...)
    decltype(auto) GetVector() 
    {
        static constexpr size_t idx = index_of_name_eval<Name, Names...>();
        return std::get<idx>(variables_);
    }

    template <FixedString Name>// requires (Name == Names || ...)
    decltype(auto) GetVector() const
    {
        static constexpr size_t idx = index_of_name_eval<Name, Names...>();
        return std::get<idx>(variables_);
    }

    template <FixedString Name>// requires (Name == Names || ...)
    decltype(auto) GetSliceImpl(size_t pos)
    {
        auto& vec = GetVector<Name>();
        assert(vec.size() > pos);

        return (vec[pos]);
    }

    template <FixedString Name>// requires (Name == Names || ...)
    decltype(auto) GetSliceImpl(size_t pos) const
    {
        auto& vec = GetVector<Name>();
        assert(vec.size() > pos);

        return (vec[pos]);
    }

public:
    StableSOA() = default;
    ~StableSOA() = default;

    template <FixedString...NameArgs> 
    auto GetSlice(size_t pos)
    { 
        using Type = std::tuple<std::add_lvalue_reference_t<
            typename type_at_name_t<TupleType, NameArgs, Names...>::value_type
        >...>;

        return Type(GetSliceImpl<NameArgs>(pos)...);
    }

    template <FixedString...NameArgs>
    auto GetSlice(size_t pos) const
    {
        using Type = std::tuple<std::add_const_t<std::add_lvalue_reference_t<
            typename type_at_name_t<TupleType, NameArgs, Names...>::value_type
        >>...>;

        return Type(GetSliceImpl<NameArgs>(pos)...);
    }

    template <typename... Args> requires ((sizeof...(Args) == sizeof...(Ts))
                                          && (std::convertible_to<Args, Ts> && ...))
    size_t PushBack(Args&&... args)
    {
        size_t newIdx = std::get<0>(variables_).size();

        auto& vecs = variables_;

        [&]<size_t...Is>(std::index_sequence<Is...>) {
            ((std::get<Is>(vecs).emplace_back(std::forward<Args>(args))), ...);
        }(std::index_sequence_for<Ts...>{});

        return newIdx;
    }

    template <typename Fn> requires std::invocable<Fn, Ts&...>
    void ForEach(Fn&& fn) &
        noexcept(noexcept(std::invoke(std::forward<Fn>(fn), std::declval<Ts&>()...)))
    {
        auto& vecs = variables_;

        [&]<size_t...Is>(std::index_sequence<Is...>) {
            for (size_t i = 0; i < Size(); i++)
            {
                assert((std::get<Is>(vecs).size() > i) && ...);

                std::invoke(fn, std::get<Is>(vecs)[i]...);
            }
        }(std::index_sequence_for<Ts...>{});
    }

    template <typename Fn> requires std::invocable<Fn, const Ts&...>
    void ForEach(Fn&& fn) const &
        noexcept(noexcept(std::invoke(std::forward<Fn>(fn), std::declval<const Ts&>()...)))
    {
        const auto& vecs = variables_;

        [&]<size_t...Is>(std::index_sequence<Is...>) {
            for (size_t i = 0; i < Size(); i++)
            {
                assert((std::get<Is>(vecs).size() > i) && ...);

                std::invoke(fn, std::get<Is>(vecs)[i]...);
            }
        }(std::index_sequence_for<Ts...>{});
    }

    size_t Size() const
    {
        return std::get<0>(variables_).size();
    }
};



//struct SpriteInfo : SOAInterface<SpriteInfo>
//{
//    std::vector<std::string> spriteName;
//    std::vector<std::string> filepath;
//    std::vector<std::string> seriesName;
//    std::vector<size_t> seriesInde;
//};
//
//auto f()
//{
//    SpriteInfo infos{};
//    auto [name, idx] = infos.GetAt<&SpriteInfo::seriesName, &SpriteInfo::seriesIndex>(0);
//}


//return Type(std::get<
//    index_of_name_v<NameArgs, TaggedType<Names, Ts>>>(
//variables)...);