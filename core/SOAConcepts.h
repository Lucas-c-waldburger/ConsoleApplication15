#pragma once
#include <type_traits>
#include <limits>

namespace detail {

template <typename T>
struct member_ptr_traits_impl;

template <typename Class, typename T>
struct member_ptr_traits_impl<T Class::*>
{
    using class_type = Class;
    using value_type = T;
};

} // detail

template <auto MemberPtr> requires std::is_member_object_pointer_v<decltype(MemberPtr)>
struct member_ptr_traits : detail::member_ptr_traits_impl<decltype(MemberPtr)> {};


//namespace detail {

template <auto Ptr1, auto Ptr2>
struct member_ptr_equals : std::false_type {};

template <auto Ptr1, auto Ptr2> 
    requires std::equality_comparable_with<decltype(Ptr1), decltype(Ptr2)>
struct member_ptr_equals<Ptr1, Ptr2>
{
    static constexpr bool value = Ptr1 == Ptr2;
};



//template <auto Ptr1, auto Ptr2>
//struct member_ptr_equals<Ptr1, Ptr2> : 


template <auto PtrArg, size_t Idx, auto...Ptrs>
struct index_of_member_ptr;

template <auto PtrArg, size_t Idx, auto Last>
struct index_of_member_ptr<PtrArg, Idx, Last> {
    static constexpr size_t value = member_ptr_equals<PtrArg, Last>::value
        ? Idx
        : std::numeric_limits<size_t>::max();
};

template <auto PtrArg, size_t Idx, auto Current, auto...Rest>
struct index_of_member_ptr<PtrArg, Idx, Current, Rest...> {
    static constexpr size_t value = member_ptr_equals<PtrArg, Current>::value
        ? Idx
        : index_of_member_ptr<PtrArg, Idx + 1, Rest...>::value;
};

//} // detail

/* SOA COMPATIBLE CONCEPT */
template <typename Class, auto...MemberPtrs>
inline constexpr bool member_ptrs_match_class_v =
    (std::same_as<typename member_ptr_traits<MemberPtrs>::class_type, Class> && ...);

template <typename Class, auto...MemberPtrs>
inline constexpr bool class_constructible_from_member_ptr_values_v =
    (std::is_constructible_v<Class, 
        typename member_ptr_traits<MemberPtrs>::value_type> && ...);


template <typename Class, auto...MemberPtrs>
concept ClassSOACompatible =
std::is_default_constructible_v<Class>; //&&
    //class_constructible_from_member_ptr_values_v<Class, MemberPtrs...>;

template <typename Class, auto...MemberPtrs>
concept MemberPtrsSOACompatible =
    sizeof...(MemberPtrs) > 0 &&
    member_ptrs_match_class_v<Class, MemberPtrs...>;


template <typename Class, auto...MemberPtrs>
concept SOACompatible = ClassSOACompatible<Class, MemberPtrs...>&&
                        MemberPtrsSOACompatible<Class, MemberPtrs...>;