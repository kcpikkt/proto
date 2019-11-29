#pragma once
#include <tuple>
#include <utility>
#include <type_traits>
#include <stdint.h>

namespace proto {
namespace meta {
     /**********************************************************
     Sequence
     */

    template <size_t ...Ns> struct Sequence{};
    template <size_t ...Ns> struct SequenceGenerator;

    template<size_t B, size_t N, size_t ...Ns>
    struct SequenceGenerator<B, N, Ns...> {
        using type = typename SequenceGenerator<B, N-1, N-1, Ns...>::type;
    };
    template<size_t B, size_t ...Ns>
    struct SequenceGenerator<B, B, Ns...> {
        using type = Sequence<Ns...>;
    };


    template <size_t B, size_t E>
    using MakeSequence = typename SequenceGenerator<B,E>::type;

    struct typelist_void;

    template<typename...> struct _typelist;

    template<typename... Ts>
    using typelist = _typelist<typelist_void, Ts...>;

    template<typename ...Ts>
    struct _typelist<typelist_void, Ts...>{

        constexpr static size_t size = sizeof...(Ts);

        template<size_t I>
        struct at {
            using type =
                typename std::tuple_element<I, std::tuple<Ts...>>::type;
        };
        // template<size_t I>
        // using at_t = typename at<I>::type;

        template<typename...> struct sub;

        template<size_t ...Is>
        struct sub<Sequence<Is...>> {
            using type = typelist<typename at<Is>::type...>;
        };

        template <size_t N>
        struct prefix {
            static_assert(size >= N);
            using type = typename sub<MakeSequence<0, N>
                                      >::type;
        };

        // template <size_t N>
        // using prefix = typename prefix<N>::type;


        template <size_t N>
        struct suffix {
            static_assert(size >= N);
            using type = typename sub<MakeSequence<size - N, size>
                                      >::type;
        };

        struct first {
            static_assert(size > 0);
            using type = typename at<0>::type;
        };
        // using first_t = typename first::type;

        struct last {
            static_assert(size > 0);
            using type = typename at<size - 1>::type;
        };
        // using last_t = typename last::type;

        template <typename...>
        struct append;

        template <typename T>
        struct append <T> {
            using type = typelist<Ts..., T>;
        };

        template <typename ...Other_Ts,
                  template <typename...> typename List>
        struct append <List<typelist_void, Other_Ts...>> {
            using type = typelist<Ts..., Other_Ts...>;
        };

        // template <typename... Ts>
        // using append_t = typename append<Ts...>::type;

        template <typename...>
        struct prepend;

        template <typename T>
        struct prepend <T> {
            using type = typelist<T, Ts...>;
        };

        template <typename ...Other_Ts,
                  template <typename...> typename List>
        struct prepend <List<typelist_void, Other_Ts...>>
        {
            using type = typelist<Other_Ts..., Ts...>;
        };

        // template <typename... Ts>
        // using prepend_t = typename append<Ts...>::type;

        template <typename...> struct index_of;

        //basecase
        template <typename T,
                  typename ...Recursive_Ts,
                  template <typename...> typename List>
        struct index_of<T, T, List<Recursive_Ts...>>
        {
            constexpr static size_t value = List<Recursive_Ts...>::size;
        };

        template <typename T1,
                  typename T2,
                  typename ...Recursive_Ts,
                  template <typename...> typename List>
        struct index_of<T1, T2, List<Recursive_Ts...>>
        {
            using helper = typelist<Ts...>::prefix<List<Recursive_Ts...>::size - 1>;
            using prefix_typelist = typename helper::type;

            constexpr static size_t value =
                index_of<T1,
                        typename typelist<Recursive_Ts...>::last::type,
                        prefix_typelist>::value;
        };

        template <typename T>
        struct index_of<T>
        {
            using helper = typelist<Ts...>::prefix<size - 1>;
            using prefix_typelist = typename helper::type;

            constexpr static size_t value =
                index_of<T,
                        typename typelist<Ts...>::last::type,
                        prefix_typelist>::value;
        };

        // template <typename ...Ts>
        // using index_of_v = typename index_of<Ts...>::value;
    };

    template<typename T,T Val>
    struct constant {
        constexpr static T value = Val;
    };

    using true_t = constant<bool, true>;
    using false_t = constant<bool, false>;

    // what a shame that and is a keyword

    template<typename...> using void_t = void;

    // identity
    template<typename T>
    struct identity { using type = T; };

    // negation
    template<typename T>
    struct negation : constant<bool, !T::value> {};

    template<typename T>
    inline constexpr auto negation_v = negation<T>::value;

    // conjunction
    template<typename ...> struct conjunction;

    template<typename T>
    struct conjunction<T> : constant<bool, T::value> {};

    template<typename T, typename ...Ts>
    struct conjunction<T, Ts...> :
        constant<bool, T::value && conjunction<Ts...>::value> {};

    template<typename ...Ts>
    inline static constexpr auto conjunction_v = conjunction<Ts...>::value;

    // is_same (add variadic?)
    template<typename A, typename B>
    struct is_same : false_t {};

    template<typename A>
    struct is_same<A, A> : true_t {};

    template<typename A, typename B>
    inline constexpr auto is_same_v = is_same<A,B>::value;

    // is_floating_point
    namespace internal {
        template<typename T>
        struct is_floating_point : false_t {};

        template<>
        struct is_floating_point<float> : true_t {};

        template<>
        struct is_floating_point<double> : true_t {};

        template<>
        struct is_floating_point<long double> : true_t {};
    }

    template<typename T>
    using is_floating_point = internal::is_floating_point<T>;

    template<typename T>
    inline static constexpr auto is_floating_point_v = is_floating_point<T>::value;

    // is_integer
    namespace internal {
        template<typename T>
        struct is_integer : false_t {};

        template<>
        struct is_integer<bool> : true_t {};

        template<>
        struct is_integer<char> : true_t {};

        template<>
        struct is_integer<signed char> : true_t {};

        template<>
        struct is_integer<unsigned char> : true_t {};

        template<>
        struct is_integer<wchar_t> : true_t {};

        // cannot find it
        // template<>
        // struct is_integer<char8_t> : true_t {};

        template<>
        struct is_integer<char16_t> : true_t {};

        template<>
        struct is_integer<char32_t> : true_t {};

        template<>
        struct is_integer<short> : true_t {};

        template<>
        struct is_integer<unsigned short> : true_t {};

        template<>
        struct is_integer<int> : true_t {};

        template<>
        struct is_integer<unsigned int> : true_t {};

        template<>
        struct is_integer<long> : true_t {};

        template<>
        struct is_integer<unsigned long> : true_t {};

        template<>
        struct is_integer<long long> : true_t {};

        template<>
        struct is_integer<unsigned long long> : true_t {};
    }

    template<typename T>
    using is_integer = internal::is_integer<T>;

    template<typename T>
    inline static constexpr auto is_integer_v = is_integer<T>::value;

    template<bool, typename T, typename>
    struct conditional : identity<T> {};

    template<typename T, typename F>
    struct conditional<false, T, F> : identity<F> {};
    namespace internal {
        template<bool, typename T = void>
        struct enable_if {};

        template<typename T>
        struct enable_if<true,T> {
            using type = T;
        };
    }

    template<bool B, typename T = void>
    using enable_if = internal::enable_if<B, T>;

    template<bool B, typename T = void>
    using enable_if_t = typename enable_if<B, T>::type;

    //hmmm
    template<typename T>
    struct is_allocator : false_t
    {};

    // NOTE(kacper): stl uses some additional
    //               universal reference binding wizardry
    //               of which purpose I do not really understand
    //               and therefore deemed unnecessary.

    template<typename T>
    struct lvalue_ref : identity<T&> {};

    template<typename T>
    struct rvalue_ref : identity<T&&> {};

    template<typename T>
    typename rvalue_ref<T>::type declval();

    namespace internal {
        // we do not provide an argument, but expression is ill formed,
        // if T is not a class, struct or union
        template<typename T>
        true_t is_aggregate_res(int T::* ); // feels like cheating

        template<typename T>
        false_t is_aggregate_res(...);
    }

    template<typename T>
    struct is_aggregate : 
        decltype(internal::is_aggregate_res<T>(nullptr)) {};

    // is_union
    // WARNING(kacper): compiler dependant
    template<typename T>
    struct is_union : constant<bool, __is_union(T)> {};

    template<typename T>
    inline static constexpr auto is_union_v = is_union<T>::value;

    // is_class
    template<typename T>
    struct is_class : conjunction<is_aggregate<T>,
                                  negation<is_union<T>>> {};
        
    template<typename T>
    inline static constexpr auto is_class_v = is_class<T>::value;

    // is_base_of
    namespace internal {
        // Base == Derived case
        // as well as when argument type is convertable to Base*
        template<typename Base>
        true_t is_base_of_test(const volatile Base*);

        template<typename Base>
        false_t is_base_of_test(const volatile void*);

        // quite nice trick is going on here.
        // if Base is private then this type is ill-formed
        template<typename Base, typename Derived>
        using is_base_of_test_result =
            decltype(is_base_of_test<Base>(declval<Derived*>()));

        // base is private case
        template<typename Base, typename Derived, typename = void>
        struct is_base_of_private_guard : true_t {};

        template<typename Base, typename Derived>
        struct is_base_of_private_guard<Base, Derived,
                                        // if is_base_of_test is ill-formed,
                                        // expression below is too and so
                                        // base template is choosen.
                                        void_t<is_base_of_test_result<Base,
                                                                     Derived>>>
        // if base is not private just use the result
            : public is_base_of_test_result<Base, Derived> {};

        template<typename Base, typename Derived>
        struct is_base_of
            : conditional<is_class_v<Base> && is_class_v<Derived>,
                          is_base_of_private_guard<Base, Derived>,
                          false_t>::type {};
    }

    template<typename Base, typename Derived>
    using is_base_of = internal::is_base_of<Base, Derived>;

    template<typename Base, typename Derived>
    inline static constexpr auto is_base_of_v = is_base_of<Base, Derived>::value;

    // has_operator_*

#define PROTO_META_HAS_OPERATOR_DEF(OP,OPNAME)                          \
    namespace internal {                                                \
        template<typename T>                                            \
        static auto has_operator_##OPNAME##_test(T* t)                  \
            -> decltype(*t OP *t, true_t{});                            \
                                                                        \
        static false_t has_operator_##OPNAME##_test(void*);             \
    }                                                                   \
    template<typename T>                                                \
    struct has_operator_##OPNAME :                                      \
        decltype(internal::has_operator_##OPNAME##_test(declval<T*>())) \
    {};                                                                 \
                                                                        \
    template<typename T>                                                \
    inline static constexpr auto has_operator_##OPNAME##_v =            \
        has_operator_##OPNAME<T>::value;

    PROTO_META_HAS_OPERATOR_DEF(==,eq);
    // does not work yet
    PROTO_META_HAS_OPERATOR_DEF(!=,neq);
    PROTO_META_HAS_OPERATOR_DEF(>=,geq);
    PROTO_META_HAS_OPERATOR_DEF(>,gr);
    PROTO_META_HAS_OPERATOR_DEF(<=,leq);
    PROTO_META_HAS_OPERATOR_DEF(>,le);

    namespace internal {
        template<typename T> struct remove_ref      : identity<T> {};
        template<typename T> struct remove_ref<T&>  : identity<T> {};
        template<typename T> struct remove_ref<T&&> : identity<T> {};
    }

    template<typename T>
    using remove_ref = internal::remove_ref<T>;

    template<typename T>
    using remove_ref_t = typename remove_ref<T>::type;

    template<typename T>
    inline remove_ref_t<T>&& move(T&& arg) {
        return static_cast<remove_ref_t<T>&&>(arg);
    }

    template<typename T>
    inline T&& forward(remove_ref_t<T>& arg) {
        return static_cast<T&&>(arg);
    }

} // namespace meta
} // namespace proto
