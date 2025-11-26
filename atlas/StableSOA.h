//#pragma once
//#include <string>
//#include <vector>
//#include "../core/commonObjects.h"
//#include "../core/TypeUtils.h"
//
//template <FixedString VarName, typename T>
//struct SOAMember {};
//
//template <FixedString Name, FixedString... Names>
//consteval std::size_t index_of_name_eval() {
//    std::size_t idx = 0;
//    bool found = ((Name == Names ? true : (++idx, false)) || ...);
//    if (!found) return std::numeric_limits<size_t>::max();
//    return idx;
//}
//
//template <typename Tup, FixedString Name, FixedString...Names>
//struct type_at_name
//{
//    static constexpr size_t idx = index_of_name_eval<Name, Names...>();
//    using type = std::tuple_element_t<idx, Tup>;
//};
//
//template <typename Tup, FixedString Name, FixedString...Names>
//using type_at_name_t = typename type_at_name<Tup, Name, Names...>::type;
//
//template <typename...Members> requires (sizeof...(Members) > 0)
//class StableSOA;
//
//template <FixedString...Names, typename...Ts>
//class StableSOA<SOAMember<Names, Ts>...>
//{
//private:
//    using TupleType = std::tuple<std::vector<Ts>...>;
//
//    TupleType variables_;
//
//    /* Internal helper methods */
//    template <FixedString Name>// requires (Name == Names || ...)
//    decltype(auto) GetVector() 
//    {
//        static constexpr size_t idx = index_of_name_eval<Name, Names...>();
//        return std::get<idx>(variables_);
//    }
//
//    template <FixedString Name>// requires (Name == Names || ...)
//    decltype(auto) GetVector() const
//    {
//        static constexpr size_t idx = index_of_name_eval<Name, Names...>();
//        return std::get<idx>(variables_);
//    }
//
//    template <FixedString Name>// requires (Name == Names || ...)
//    decltype(auto) GetSliceImpl(size_t pos)
//    {
//        auto& vec = GetVector<Name>();
//        assert(vec.size() > pos);
//
//        return (vec[pos]);
//    }
//
//    template <FixedString Name>// requires (Name == Names || ...)
//    decltype(auto) GetSliceImpl(size_t pos) const
//    {
//        auto& vec = GetVector<Name>();
//        assert(vec.size() > pos);
//
//        return (vec[pos]);
//    }
//
//public:
//    StableSOA() = default;
//    ~StableSOA() = default;
//
//    template <FixedString...NameArgs> 
//    auto GetSlice(size_t pos)
//    { 
//        using Type = std::tuple<std::add_lvalue_reference_t<
//            typename type_at_name_t<TupleType, NameArgs, Names...>::value_type
//        >...>;
//
//        return Type(GetSliceImpl<NameArgs>(pos)...);
//    }
//
//    template <FixedString...NameArgs>
//    auto GetSlice(size_t pos) const
//    {
//        using Type = std::tuple<std::add_const_t<std::add_lvalue_reference_t<
//            typename type_at_name_t<TupleType, NameArgs, Names...>::value_type
//        >>...>;
//
//        return Type(GetSliceImpl<NameArgs>(pos)...);
//    }
//
//    template <typename... Args> requires ((sizeof...(Args) == sizeof...(Ts))
//                                          && (std::convertible_to<Args, Ts> && ...))
//    size_t PushBack(Args&&... args)
//    {
//        size_t newIdx = std::get<0>(variables_).size();
//
//        auto& vecs = variables_;
//
//        [&]<size_t...Is>(std::index_sequence<Is...>) {
//            ((std::get<Is>(vecs).emplace_back(std::forward<Args>(args))), ...);
//        }(std::index_sequence_for<Ts...>{});
//
//        return newIdx;
//    }
//
//    template <typename Fn> requires std::invocable<Fn, Ts&...>
//    void ForEach(Fn&& fn) &
//        noexcept(noexcept(std::invoke(std::forward<Fn>(fn), std::declval<Ts&>()...)))
//    {
//        auto& vecs = variables_;
//
//        [&]<size_t...Is>(std::index_sequence<Is...>) {
//            for (size_t i = 0; i < Size(); i++)
//            {
//                assert((std::get<Is>(vecs).size() > i) && ...);
//
//                std::invoke(fn, std::get<Is>(vecs)[i]...);
//            }
//        }(std::index_sequence_for<Ts...>{});
//    }
//
//    template <typename Fn> requires std::invocable<Fn, const Ts&...>
//    void ForEach(Fn&& fn) const &
//        noexcept(noexcept(std::invoke(std::forward<Fn>(fn), std::declval<const Ts&>()...)))
//    {
//        const auto& vecs = variables_;
//
//        [&]<size_t...Is>(std::index_sequence<Is...>) {
//            for (size_t i = 0; i < Size(); i++)
//            {
//                assert((std::get<Is>(vecs).size() > i) && ...);
//
//                std::invoke(fn, std::get<Is>(vecs)[i]...);
//            }
//        }(std::index_sequence_for<Ts...>{});
//    }
//
//    size_t Size() const
//    {
//        return std::get<0>(variables_).size();
//    }
//};