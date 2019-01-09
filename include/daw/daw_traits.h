// The MIT License (MIT)
//
// Copyright (c) 2014-2019 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cpp_17.h"
#include "impl/daw_traits_concepts.h"
#include "impl/daw_traits_impl.h"

namespace daw {
	namespace traits {
		template<typename T>
		using root_type_t = std::decay_t<std::remove_reference_t<T>>;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Return the largest sizeof in list of types.  Used to pad
		///				unions and small value optimization
		///
		template<typename...>
		struct max_sizeof;

		template<typename First>
		struct max_sizeof<First> {
			using type = First;
			static const size_t value = sizeof( type );
		};

		// the biggest of everything in Args and First
		template<typename First, typename... Args>
		struct max_sizeof<First, Args...> {
			using next = typename max_sizeof<Args...>::type;
			using type =
			  typename std::conditional<( sizeof( First ) >= sizeof( next ) ), First,
			                            next>::type;
			static const size_t value = sizeof( type );
		};

		template<typename... Types>
		using max_sizeof_t = typename max_sizeof<Types...>::type;

		template<typename... Types>
		constexpr size_t max_sizeof_v = max_sizeof<Types...>::value;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Returns true if all values passed are true
		///
		template<typename BoolType,
		         std::enable_if_t<is_convertible_v<BoolType, bool>,
		                          std::nullptr_t> = nullptr>
		constexpr bool are_true( BoolType b1 ) noexcept {
			return static_cast<bool>( b1 );
		}

		template<typename BoolType1, typename BoolType2>
		constexpr bool are_true( BoolType1 b1, BoolType2 b2 ) noexcept {
			return are_true( b1 ) and are_true( b2 );
		}

		template<typename BoolType1, typename BoolType2, typename... Booleans>
		constexpr bool are_true( BoolType1 b1, BoolType2 b2,
		                         Booleans... others ) noexcept {
			return are_true( b1, b2 ) and are_true( others... );
		}

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Are all template parameters the same type
		///
		template<typename T, typename... Rest>
		struct are_same_types : std::false_type {};

		template<typename T, typename First>
		struct are_same_types<T, First> : std::is_same<T, First> {};

		template<typename T, typename First, typename... Rest>
		struct are_same_types<T, First, Rest...>
		  : std::integral_constant<bool, is_same_v<T, First> or
		                                   are_same_types<T, Rest...>::value> {};

		template<typename T, typename... Rest>
		CXINLINE bool are_same_types_v = are_same_types<T, Rest...>::value;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	A sequence of bool values
		///
		template<bool...>
		struct bool_sequence {};

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Integral constant with result of and'ing all bool's supplied
		///
		template<bool... Bools>
		using bool_and = std::is_same<bool_sequence<Bools...>,
		                              bool_sequence<( Bools or true )...>>;

		template<bool... Bools>
		CXINLINE bool bool_and_v = bool_and<Bools...>::value;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Integral constant with result of or'ing all bool's supplied
		///
		template<bool... Bools>
		using bool_or =
		  std::integral_constant<bool, ( !bool_and<!Bools...>::value )>;

		template<bool... Bools>
		CXINLINE bool bool_or_v = bool_or<Bools...>::value;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Similar to enable_if but enabled when any of the
		///				parameters is true
		///
		template<typename R, bool... Bs>
		using enable_if_any = std::enable_if<bool_or_v<Bs...>, R>;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Similar to enable_if but enabled when all of the
		///				parameters is true
		///
		template<typename R, bool... Bs>
		using enable_if_all = std::enable_if<bool_and_v<Bs...>, R>;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Is type T on of the other types
		///
		template<typename T, typename... Types>
		struct is_one_of : std::false_type {};

		template<typename T, typename Type>
		struct is_one_of<T, Type> : std::is_same<T, Type> {};

		template<typename T, typename Type, typename... Types>
		struct is_one_of<T, Type, Types...>
		  : daw::disjunction<std::is_same<T, Type>, is_one_of<T, Types...>> {};

		template<typename T, typename... Types>
		using is_one_of_t = typename is_one_of<T, Types...>::type;

		template<typename T, typename... Types>
		CXINLINE bool is_one_of_v = is_one_of<T, Types...>::value;

		template<typename...>
		struct can_convert_from : std::false_type {};

		template<typename To, typename From>
		struct can_convert_from<To, From> : std::is_convertible<From, To> {};

		template<typename To, typename From, typename... Froms>
		struct can_convert_from<To, From, Froms...>
		  : daw::disjunction<std::is_convertible<From, To>,
		                     can_convert_from<To, Froms...>> {};

		template<typename To, typename... Froms>
		using can_convert_from_t = typename can_convert_from<To, Froms...>::type;

		template<typename To, typename... Froms>
		CXINLINE bool can_convert_from_v = can_convert_from<To, Froms...>::value;

		namespace details {
			template<typename>
			struct type_sink {
				using type = void;
			}; // consumes a type, and makes it `void`
			template<typename T>
			using type_sink_t = typename type_sink<T>::type;
		} // namespace details

// Build a check for the precence of a member
// Can be used like METHOD_CHECKER_ANY( has_begin_method, begin, ( ) ) and the
// check will look for begin( ) If you want to check args you could do the
// following METHOD_CHECKER_ANY( has_index_operation, operator[], (
// std::declval<size_t>( ) ) )
#define METHOD_CHECKER_ANY( name, fn )                                         \
	namespace impl {                                                             \
		template<typename T, typename... Args>                                     \
		using name =                                                               \
		  decltype( std::declval<T>( ).fn( std::declval<Args>( )... ) );           \
	}                                                                            \
	template<typename T, typename... Args>                                       \
	constexpr bool name##_v = is_detected_v<impl::name, T, Args...>;             \
	// END METHOD_CHECKER_ANY

		// decltype( std::declval<typename T::MemberName>( ) );
#define HAS_STATIC_TYPE_MEMBER( MemberName )                                   \
	namespace detectors {                                                        \
		template<typename T>                                                       \
		using MemberName##_member = typename T::MemberName;                        \
	}                                                                            \
	template<typename T>                                                         \
	constexpr bool has_##MemberName##_member_v =                                 \
	  is_detected_v<detectors::MemberName##_member, T>;

		HAS_STATIC_TYPE_MEMBER( type );
		HAS_STATIC_TYPE_MEMBER( value_type );
		HAS_STATIC_TYPE_MEMBER( mapped_type );
		HAS_STATIC_TYPE_MEMBER( iterator );

		METHOD_CHECKER_ANY( has_begin_member, begin );
		METHOD_CHECKER_ANY( has_end_member, end );

		namespace detectors {
			template<typename T>
			using has_substr_member = decltype( std::declval<T>( ).substr(
			  std::declval<size_t>( ), std::declval<size_t>( ) ) );
		}

		template<typename T>
		CXINLINE bool has_substr_member_v =
		  is_detected_v<detectors::has_substr_member, T>;

		template<typename T>
		CXINLINE bool is_string_v = all_true_v<is_convertible_v<T, std::string> or
		                                       is_convertible_v<T, std::wstring>>;

		template<typename T>
		CXINLINE bool isnt_string_v = !is_string_v<T>;

		template<typename T>
		CXINLINE bool is_container_not_string_v =
		  all_true_v<isnt_string_v<T>, is_container_like_v<T>>;

		template<typename T>
		CXINLINE bool is_map_like_v =
		  all_true_v<is_container_like_v<T>, has_mapped_type_member_v<T>>;

		template<typename T>
		CXINLINE bool isnt_map_like_v = !is_map_like_v<T>;

		template<typename T>
		CXINLINE bool is_vector_like_not_string_v =
		  all_true_v<is_container_not_string_v<T>, isnt_map_like_v<T>>;

		template<typename T>
		using static_not =
		  std::conditional<T::value, std::false_type, std::true_type>;

		template<typename T>
		using static_not_t = typename static_not<T>::type;

		template<typename T>
		CXINLINE bool static_not_v = static_not<T>::value;

#define GENERATE_IS_STD_CONTAINER1( ContainerName )                            \
	template<typename T>                                                         \
	constexpr bool is_##ContainerName##_v =                                      \
	  is_same_v<T, std::ContainerName<typename T::value_type>>;

		GENERATE_IS_STD_CONTAINER1( vector );
		GENERATE_IS_STD_CONTAINER1( list );
		GENERATE_IS_STD_CONTAINER1( set );
		GENERATE_IS_STD_CONTAINER1( unordered_set );
		GENERATE_IS_STD_CONTAINER1( deque );

#undef GENERATE_IS_STD_CONTAINER1

#define GENERATE_IS_STD_CONTAINER2( ContainerName )                            \
	template<typename T>                                                         \
	constexpr bool is_##ContainerName##_v = is_same_v<                           \
	  T, std::ContainerName<typename T::key_type, typename T::mapped_type>>;

		GENERATE_IS_STD_CONTAINER2( map );
		GENERATE_IS_STD_CONTAINER2( unordered_map );

#undef GENERATE_IS_STD_CONTAINER2

		template<typename T>
		CXINLINE bool is_single_item_container_v =
		  any_true_v<is_vector_v<T>, is_list_v<T>, is_set_v<T>, is_deque_v<T>,
		             is_unordered_set_v<T>>;

		template<typename T>
		CXINLINE bool is_container_v =
		  any_true_v<is_vector_v<T>, is_list_v<T>, is_set_v<T>, is_deque_v<T>,
		             is_unordered_set_v<T>, is_map_v<T>, is_unordered_map_v<T>>;

		template<typename T>
		CXINLINE bool is_map_type_v =
		  any_true_v<is_map_v<T>, is_unordered_map_v<T>>;

		template<typename T>
		CXINLINE bool is_numberic_v =
		  any_true_v<is_floating_point_v<T>, is_integral_v<T>>;

		template<typename T>
		CXINLINE bool is_container_or_array_v =
		  any_true_v<is_container_v<T>, is_array_v<T>>;

		namespace detectors {
			template<typename OutStream, typename T>
			using streamable =
			  decltype( std::declval<OutStream>( ) << std::declval<T>( ) );
		}
		template<typename OutStream, typename T>
		CXINLINE bool is_streamable_v =
		  daw::is_detected_v<detectors::streamable, OutStream &, T>;

		template<template<class> class Base, typename Derived>
		CXINLINE bool is_mixed_from_v = is_base_of_v<Base<Derived>, Derived>;

		template<std::size_t I, typename T, typename... Ts>
		struct nth_element_impl {
			using type = typename nth_element_impl<I - 1, Ts...>::type;
		};

		template<typename T, typename... Ts>
		struct nth_element_impl<0, T, Ts...> {
			using type = T;
		};

		template<std::size_t I, typename... Ts>
		using nth_element = typename nth_element_impl<I, Ts...>::type;

		namespace impl {
			template<typename T>
			auto has_to_string( T const &, long ) -> std::false_type;

			template<typename T>
			auto has_to_string( T const &lhs, int )
			  -> std::is_convertible<decltype( lhs.to_string( ) ), std::string>;
		} // namespace impl

		template<typename T>
		CXINLINE bool has_to_string_v =
		  decltype( impl::has_to_string( std::declval<T>( ), 1 ) )::value;

		namespace operators {
			namespace impl {
				template<typename L, typename R>
				auto has_op_eq_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_eq_impl( L const &lhs, R const &rhs, int )
				  -> std::is_convertible<decltype( lhs == rhs ), bool>;

				template<typename L, typename R>
				auto has_op_ne_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_ne_impl( L const &lhs, R const &rhs, int )
				  -> std::is_convertible<decltype( lhs != rhs ), bool>;

				template<typename L, typename R>
				auto has_op_lt_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_lt_impl( L const &lhs, R const &rhs, int )
				  -> std::is_convertible<decltype( lhs < rhs ), bool>;

				template<typename L, typename R>
				auto has_op_le_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_le_impl( L const &lhs, R const &rhs, int )
				  -> std::is_convertible<decltype( lhs <= rhs ), bool>;

				template<typename L, typename R>
				auto has_op_gt_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_gt_impl( L const &lhs, R const &rhs, int )
				  -> std::is_convertible<decltype( lhs > rhs ), bool>;

				template<typename L, typename R>
				auto has_op_ge_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_ge_impl( L const &lhs, R const &rhs, int )
				  -> std::is_convertible<decltype( lhs >= rhs ), bool>;
			} // namespace impl

			template<typename L, typename R = L>
			CXINLINE bool has_op_eq_v = decltype( impl::has_op_eq_impl(
			  std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			CXINLINE bool has_op_ne_v = decltype( impl::has_op_ne_impl(
			  std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			CXINLINE bool has_op_lt_v = decltype( impl::has_op_lt_impl(
			  std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			CXINLINE bool has_op_le_v = decltype( impl::has_op_le_impl(
			  std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			CXINLINE bool has_op_gt_v = decltype( impl::has_op_gt_impl(
			  std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			CXINLINE bool has_op_ge_v = decltype( impl::has_op_ge_impl(
			  std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

		} // namespace operators

		template<typename T, typename U>
		constexpr inline bool not_self( ) {
			using decayed_t = typename std::decay_t<T>;
			return !is_same_v<decayed_t, U> and !is_base_of_v<U, decayed_t>;
		}

		template<typename To, typename... From>
		CXINLINE bool are_convertible_to_v =
		  all_true_v<is_convertible_v<From, To>...>;

		template<typename String>
		CXINLINE bool is_not_array_array_v =
		  !daw::is_detected_v<detectors::is_array_array, String>;

		template<typename String>
		CXINLINE bool is_string_view_like_v =
		  is_container_like_v<String const> and has_integer_subscript_v<
		    String const> and has_size_memberfn_v<String const>
		    and has_empty_memberfn_v<String const> and is_not_array_array_v<String>;

		template<typename String>
		CXINLINE bool is_string_like_v = is_string_view_like_v<String>
		  and has_append_operator_v<String> and has_append_memberfn_v<String>
		    and is_container_like_v<String> and has_integer_subscript_v<String>;

		template<typename Container, size_t ExpectedSize>
		CXINLINE bool is_value_size_equal_v =
		  sizeof( *std::cbegin( std::declval<Container>( ) ) ) == ExpectedSize;

		namespace impl {
			template<typename... Ts>
			struct are_unique;

			template<typename T1>
			struct are_unique<T1> : std::true_type {};

			template<typename T1, typename T2>
			struct are_unique<T1, T2>
			  : std::integral_constant<bool, !daw::is_same_v<T1, T2>> {};

			template<typename T1, typename T2, typename... Ts>
			struct are_unique<T1, T2, Ts...>
			  : std::integral_constant<bool, are_unique<T1, T2>::value and
			                                   are_unique<T1, Ts...>::value and
			                                   are_unique<T2, Ts...>::value> {};
		} // namespace impl

		template<typename... Ts>
		using are_unique_t = impl::are_unique<Ts...>;
		template<typename... Ts>
		CXINLINE bool are_unique_v = are_unique_t<Ts...>::value;

		namespace impl {
			template<typename...>
			struct isnt_cv_ref;

			template<>
			struct isnt_cv_ref<> : std::true_type {};

			template<typename T, typename... Ts>
			struct isnt_cv_ref<T, Ts...>
			  : std::integral_constant<
			      bool,
			      ( !any_true_v<is_const_v<T>, is_reference_v<T>,
			                    is_volatile_v<T>> and
			        is_same_v<std::true_type, typename isnt_cv_ref<Ts...>::type> )> {
			};
		} // namespace impl

		template<typename... Ts>
		CXINLINE bool isnt_cv_ref_v = impl::isnt_cv_ref<Ts...>::value;

		template<typename T>
		using deref_t = decltype( *std::declval<T>( ) );

		template<typename Function, typename... Args>
		using result_of_t =
		  decltype( std::declval<Function>( )( std::declval<Args>( )... ) );

		template<typename Function, typename... Args>
		using invoke_result_t =
		  decltype( std::declval<Function>( )( std::declval<Args>( )... ) );


		namespace impl {
			template<typename T, typename... Args>
			struct first_type_impl {
				using type = T;
			};
		} // namespace impl

		template<typename... Args>
		using first_type = typename impl::first_type_impl<Args...>::type;

		namespace impl {
			template<typename... Ts>
			constexpr void tuple_test( std::tuple<Ts...> const & ) noexcept {}

			template<typename... Ts>
			using detect_is_tuple = decltype( tuple_test( std::declval<Ts>( )... ) );
		} // namespace impl

		template<typename...>
		struct is_first_type;

		template<typename T>
		struct is_first_type<T> : std::false_type {};

		template<typename T, typename Arg, typename... Args>
		struct is_first_type<T, Arg, Args...> : std::is_same<T, std::decay_t<Arg>> {
		};

		// Tests if type T is the same as the first argument in Args or if args is
		// empty it is false
		template<typename T, typename... Args>
		CXINLINE bool is_first_type_v = is_first_type<T, Args...>::value;

		template<typename... Ts>
		CXINLINE bool is_tuple_v = is_detected_v<impl::detect_is_tuple, Ts...>;

		namespace impl {}
		template<typename T, typename... Args>
		CXINLINE bool is_init_list_constructible_v = all_true_v<
		  are_same_types_v<Args...>,
		  daw::is_constructible_v<T, std::initializer_list<first_type<Args...>>>>;

		namespace ostream_detectors {
			template<typename T>
			using has_char_type_detect = typename T::char_type;

			template<typename T>
			using has_adjustfield_detect = decltype( T::adjustfield );

			template<typename T>
			using has_left_detect = decltype( T::left );

			template<typename T>
			using has_fill_member_detect = decltype( std::declval<T>( ).fill( ) );

			template<typename T>
			using has_good_member_detect = decltype( std::declval<T>( ).good( ) );

			template<typename T, typename CharT>
			using has_write_member_detect = decltype( std::declval<T>( ).write(
			  std::declval<CharT const *>( ), std::declval<int>( ) ) );

			template<typename T>
			using has_width_member_detect = decltype( std::declval<T>( ).width( ) );

			template<typename T>
			using has_flags_member_detect = decltype( std::declval<T>( ).flags( ) );

			template<typename T>
			CXINLINE bool has_adjustfield_v =
			  daw::is_detected_v<has_adjustfield_detect, T>;

			template<typename T>
			CXINLINE bool has_left_v = daw::is_detected_v<has_left_detect, T>;

			template<typename T>
			CXINLINE bool has_char_type_v =
			  daw::is_detected_v<has_char_type_detect, T>;

			template<typename T>
			CXINLINE bool has_fill_member_v =
			  daw::is_detected_v<has_fill_member_detect, T>;

			template<typename T>
			CXINLINE bool has_good_member_v =
			  daw::is_detected_v<has_good_member_detect, T>;

			template<typename T, typename CharT>
			CXINLINE bool has_write_member_v =
			  daw::is_detected_v<has_write_member_detect, T, CharT>;

			template<typename T>
			CXINLINE bool has_width_member_v =
			  daw::is_detected_v<has_width_member_detect, T>;

			template<typename T>
			CXINLINE bool has_flags_member_v =
			  daw::is_detected_v<has_flags_member_detect, T>;

		} // namespace ostream_detectors

		template<typename T>
		CXINLINE bool is_ostream_like_lite_v =
		  all_true_v<ostream_detectors::has_char_type_v<T>,
		             ostream_detectors::has_adjustfield_v<T>,
		             ostream_detectors::has_fill_member_v<T>,
		             ostream_detectors::has_good_member_v<T>,
		             ostream_detectors::has_width_member_v<T>,
		             ostream_detectors::has_flags_member_v<T>>;

		template<typename T, typename CharT>
		CXINLINE bool is_ostream_like_v =
		  all_true_v<is_ostream_like_lite_v<T>,
		             ostream_detectors::has_write_member_v<T, CharT>>;

		template<typename T>
		CXINLINE bool is_character_v = is_one_of_v<T, char, wchar_t>;

		template<typename Arg, typename... Args>
		using last_type_t =
		  std::tuple_element_t<sizeof...( Args ), std::tuple<Arg, Args...>>;

		template<size_t N, typename... Args>
		using nth_type = std::tuple_element_t<N, std::tuple<Args...>>;

		template<typename A, typename B, typename... C>
		struct pack_index_of
		  : std::integral_constant<int, std::is_same<A, B>{}
		                                  ? 0
		                                  : ( pack_index_of<A, C...>{} == -1
		                                        ? -1
		                                        : 1 + pack_index_of<A, C...>{} )> {
		};

		template<typename... Args>
		struct pack_list {
			static constexpr size_t const size = sizeof...( Args );
		};

		template<typename T, typename... Args>
		struct pack_list_front {
			using front = T;
			using back = pack_list<Args...>;
		};

		template<typename... Args>
		using pop_list_front = pack_list_front<Args...>;

		template<typename T, typename... Args>
		using push_list_back = pack_list<Args..., T>;

		template<typename T, typename... Args>
		using push_list_front = pack_list<T, Args...>;

		template<typename A, typename B>
		struct pack_index_of<A, B>
		  : std::integral_constant<int, std::is_same<A, B>{} - 1> {};

		template<typename T, typename... Pack>
		struct pack_index_of<T, pack_list<Pack...>> : pack_index_of<T, Pack...> {};

		template<typename T, typename... Pack>
		CXINLINE auto pack_index_of_v = pack_index_of<T, Pack...>::value;

		template<typename T, typename... Pack>
		CXINLINE bool pack_exits_v = pack_index_of_v<T, Pack...> >= 0;

	} // namespace traits
} // namespace daw
