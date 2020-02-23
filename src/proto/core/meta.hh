#pragma once
#include "proto/core/common/types.hh"

//#include <type_traits>

namespace proto {
namespace meta {
     /**********************************************************
     sequence
     */

    template <size_t ...Ns> struct sequence{};
    template <size_t ...Ns> struct sequence_generator;

    template<size_t B, size_t N, size_t ...Ns>
    struct sequence_generator<B, N, Ns...> {
        using type = typename sequence_generator<B, N-1, N-1, Ns...>::type;
    };
    template<size_t B, size_t ...Ns>
    struct sequence_generator<B, B, Ns...> {
        using type = sequence<Ns...>;
    };

    template <size_t B, size_t E>
    using make_sequence = typename sequence_generator<B,E>::type;

    //constant
    template<typename T,T Val>
    struct constant {
        constexpr static T value = Val;
    };

    using true_t = constant<bool, true>;
    using false_t = constant<bool, false>;

    // is_same (add variadic?)
    template<typename A, typename B>
    struct is_same : false_t {};

    template<typename A>
    struct is_same<A, A> : true_t {};

    template<typename A, typename B>
    inline constexpr auto is_same_v = is_same<A,B>::value;

    // identity
    template<typename T>
    struct identity { using type = T; };

    // conditional
    template<bool, typename T, typename>
    struct conditional : identity<T> {};

    template<typename T, typename F>
    struct conditional<false, T, F> : identity<F> {};

    template<bool C, typename T, typename F>
    using conditional_t = typename conditional<C,T,F>::type;

    // conditional_value
    template<typename T, bool, T true_val, T>
    struct conditional_value : constant<T, true_val> {};

    template<typename T, T true_val, T false_val>
    struct conditional_value<T, false, true_val, false_val>
        : constant<T, false_val> {};


    // remove_cv
    
    namespace internal {
        template<typename T> struct remove_const          : identity<T> {};
        template<typename T> struct remove_const<const T> : identity<T> {};

        template<typename T> struct remove_volatile             : identity<T> {};
        template<typename T> struct remove_volatile<volatile T> : identity<T> {};
    }
    
    template<typename T>
    struct remove_cv :
        identity<typename internal::remove_volatile<typename internal::remove_const<T>::type>::type> {};

    //is_const

    template<typename T> struct is_const          : false_t {};
    template<typename T> struct is_const<const T> : true_t {};
    
    // is_reference
    template <typename T> struct is_reference      : false_t {};
    template <typename T> struct is_reference<T&>  : true_t {};
    template <typename T> struct is_reference<T&&> : true_t {};

    // is_function
    // NOTE(kacper): wait, it is a function if it is not a refernce and prefixed with const is not const? bizzare
    // implementation is of course stolen form cpprference
    template<typename T>
    struct is_function : constant<bool, !is_const<const T>::value && !is_reference<T>::value> {};


    // is_array
    template<typename T> struct is_array : false_t {};
    template<typename T> struct is_array<T[]> : true_t {};
    template<typename T, u64 N> struct is_array<T[N]> : true_t {};

    // remove_extent
    template<typename T> struct remove_extent              : identity<T> {};  
    template<typename T> struct remove_extent<T[]>         : identity<T> {}; 
    template<typename T, u64 N> struct remove_extent<T[N]> : identity<T> {};


    template<typename...> using void_t = void;


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


    // typelist
    struct typelist_void;

    template<typename...> struct _typelist;

    template<typename... Ts>
    using typelist = _typelist<typelist_void, Ts...>;

    template<typename ...Ts>
    struct _typelist<typelist_void, Ts...>{

        constexpr static size_t size = sizeof...(Ts);

        template<size_t I, typename T, typename...Sub_Ts> 
        struct at_helper {
            using type = typename at_helper<I-1, Sub_Ts...>::type;
        };

        template<typename T, typename...Sub_Ts> 
        struct at_helper<0, T, Sub_Ts...> {
            using type = T;
        };

        template<size_t I>
        using at = at_helper<I, Ts...>;
       
        template<size_t I>
        using at_t = typename at<I>::type;


        template<typename...> struct sub;

        template<size_t ...Is>
        struct sub<sequence<Is...>> {
            using type = typelist<typename at<Is>::type...>;
        };

        template <size_t N>
        struct prefix {
            static_assert(size >= N);
            using type = typename sub<make_sequence<0, N>
                                      >::type;
        };

        template <size_t N>
        using prefix_t = typename prefix<N>::type;

               template <size_t N>
        struct suffix {
            static_assert(size >= N);
            using type = typename sub<make_sequence<size - N, size>
                                      >::type;
        };

        struct first {
            static_assert(size > 0);
            using type = typename at<0>::type;
        };
        using first_t = typename first::type;

        struct last {
            static_assert(size > 0);
            using type = typename at<size - 1>::type;
        };
        using last_t = typename last::type;

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

        // it won't compile if type you of index you are after is not in the typelist
        template<typename...> struct index_of;

        template<typename L, typename...Sub_Ts>
        struct index_of<L, L, Sub_Ts...> {
            constexpr static auto value = (size - sizeof...(Sub_Ts) - 1);
        };

        template<typename L, typename T, typename...Sub_Ts>
        struct index_of<L, T, Sub_Ts...> {
            static_assert( sizeof...(Sub_Ts) > 0, "Type is not present in the typelist"); // no match, no left - not present
            constexpr static size_t value = index_of<L, Sub_Ts...>::value;
        };
        
        template<typename T>
        struct index_of<T> {
            constexpr static auto value = index_of<T, Ts...>::value;
        };
        
        template <typename T>
        constexpr static auto index_of_v = index_of<T>::value;


        template<typename...> struct contains;

        template<typename L, typename...Sub_Ts>
        struct contains<L, L, Sub_Ts...> : true_t {};

        template<typename L, typename T, typename...Sub_Ts>
        struct contains<L, T, Sub_Ts...> :
            conditional_t<(sizeof...(Sub_Ts) > 0), contains<L, Sub_Ts...>, false_t> {};
        
        template<typename T>
        struct contains<T> : contains<T, Ts...> {};
        
        template <typename T>
        constexpr static auto contains_v = contains<T>::value;
    };


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

    // is_convertible
    namespace internal {
        template<typename To>
        true_t is_convertible_test(To);

        template<typename>
        false_t is_convertible_test(...);

        template<typename From, typename To>
        using is_convertible = decltype(is_convertible_test<To>(declval<From>()));
    }

    template<typename A, typename B>
    using is_convertible = internal::is_convertible<A, B>;

    template<typename A, typename B>
    inline static constexpr auto is_convertible_v = is_convertible<A, B>::value;


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

    template<typename T> using remove_ref = internal::remove_ref<T>;

    template<typename T>
    using remove_ref_t = typename remove_ref<T>::type;

    template<typename T>
    constexpr static inline remove_ref_t<T>&& move(T&& x) {
        return static_cast<remove_ref_t<T>&&>(x);
    }

    template<typename T>
    constexpr static inline T&& forward(remove_ref_t<T>& arg) {
        return static_cast<T&&>(arg);
    }


    namespace internal {
        // NOTE(kacper): hmmm...
        template <typename T> auto try_add_pointer(int) -> identity<typename remove_ref<T>::type*>;
        template <typename T> auto try_add_pointer(...) -> identity<T>;

    } 
    template <typename T> struct add_pointer : decltype(internal::try_add_pointer<T>(0)) {};
    template <typename T> using add_pointer_t = typename add_pointer<T>::type;

    //decay
    template<typename T>
    struct decay {
        using U = remove_ref_t<T>;

        using type = conditional_t< 
            is_array<U>::value, typename remove_extent<U>::type*,
            conditional_t< 
                is_function<U>::value,
                typename add_pointer<U>::type,
                typename remove_cv<U>::type
            >
        >;
    };
    template<typename T>
    using decay_t = typename decay<T>::type;

} // namespace meta
} // namespace proto
