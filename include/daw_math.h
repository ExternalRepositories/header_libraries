// The MIT License (MIT)
//
// Copyright (c) 2014-2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#ifdef max
#undef max
#endif // max

#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "daw_traits.h"

namespace daw {
	namespace math {
		template<typename T>
		constexpr T const PI = T( 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899 );

		template<typename Result = intmax_t>
		constexpr Result round( double d ) noexcept {
			static_assert( daw::is_integral_v<Result>, "Result type must be integral" );
			return static_cast<Result>( d + 0.5 );
		}

		template<typename Result = intmax_t>
		constexpr Result floor( double d ) noexcept {
			static_assert( daw::is_integral_v<Result>, "Result type must be integral" );
			return static_cast<Result>( d );
		}

		template<typename Result = intmax_t>
		constexpr Result ceil( double d ) noexcept {
			static_assert( daw::is_integral_v<Result>, "Result type must be integral" );
			return static_cast<Result>( d + 1.0 );
		}

		template<typename T>
		constexpr T round_by( T const value, double const rnd_by ) noexcept {
			static_assert( is_arithmetic_v<T>, "value must be an arithmetic type" );
			auto const rnd = static_cast<double>( round( static_cast<double>( value ) / rnd_by ) );
			return static_cast<T>( rnd * rnd_by );
		}

		template<typename T>
		constexpr T floor_by( T const value, double const rnd_by ) noexcept {
			static_assert( is_arithmetic_v<T>, "value must be an arithmetic type" );
			auto const rnd = static_cast<double>( floor( static_cast<double>( value ) / rnd_by ) );
			return static_cast<T>( rnd * rnd_by );
		}

		template<typename T>
		constexpr T ceil_by( T const value, double const rnd_by ) noexcept {
			static_assert( is_arithmetic_v<T>, "value must be an arithmetic type" );
			auto const rnd = static_cast<double>( ceil( static_cast<double>( value ) / rnd_by ) );
			return static_cast<T>( rnd * rnd_by );
		}

		template<typename T>
		constexpr T abs( T t ) noexcept {
			return t >= 0 ? t : -t;
		}

		template<typename T>
		constexpr uintmax_t factorial( T t ) noexcept {
			t = abs( t );
			uintmax_t result = 1;
			for( ; t > 0; --t ) {
				result *= static_cast<uintmax_t>( t );
			}
			return result;
		}

		namespace impl {
			template<typename T>
			constexpr T pow_impl( T base, size_t exponent ) noexcept {
				for( size_t k = 0; k < exponent; ++k ) {
					base *= base;
				}
				return base;
			}

		} // namespace impl

		template<typename T>
		constexpr T pow( T base, size_t exponent ) noexcept {
			return base == 0 ? 0 : exponent > 0 ? impl::pow_impl( base, exponent ) : 1;
		}

		namespace impl {
			struct constants {
				constexpr static double const tol = 0.001;
			};

			template<typename T>
			constexpr auto cube( T x ) noexcept {
				return x * x * x;
			}

			// Based on the triple-angle formula: sin 3x = 3 sin x - 4 sin ^3 x
			template<typename R>
			constexpr R sin_helper( R x ) noexcept {
				return x < constants::tol ? x
				                          : static_cast<R>( 3.0 ) * ( sin_helper( x / static_cast<R>( 3.0 ) ) ) -
				                              static_cast<R>( 4.0 ) * cube( sin_helper( x / static_cast<R>( 3.0 ) ) );
			}
		} // namespace impl

		template<typename R>
		constexpr R sin( R x ) noexcept {
			return impl::sin_helper( x < 0 ? PI<R> - x : x );
		}

		template<typename R>
		constexpr R cos( R x ) noexcept {
			if( 0.0 == x ) {
				return 1.0;
			}
			return sin( ( PI<R> / 2.0 ) - x );
		}

		template<typename T>
		constexpr auto sqr( T const &value ) noexcept {
			static_assert( is_arithmetic_v<T>, "Template parameter must be an arithmetic type" );
			return value * value;
		}

		///
		/// <summary>Set the minimum value allowed</summary>
		///
		template<typename T, typename U>
		constexpr auto value_or_min( T const &value, U const &min_value ) noexcept {
			if( min_value > value ) {
				return min_value;
			}
			return value;
		}

		template<typename T, std::enable_if_t<!is_floating_point_v<T>, std::nullptr_t> = nullptr>
		constexpr bool nearly_equal( T const &a, T const &b ) noexcept {
			return a == b;
		}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		template<typename T, std::enable_if_t<is_floating_point_v<T>, std::nullptr_t> = nullptr>
		constexpr bool nearly_equal( T const &a, T const &b ) noexcept {
			// Code from http://floating-point-gui.de/errors/comparison/
			auto absA = std::abs( a );
			auto absB = std::abs( b );
			auto diff = std::abs( a - b );

			if( a == b ) { // shortcut, handles infinities
				return true;
			}
			if( a == 0 || b == 0 || diff < std::numeric_limits<T>::min_exponent ) {
				// a or b is zero or both are extremely close to it
				// 			// relative error is less meaningful here
				return diff < ( std::numeric_limits<T>::epsilon( ) * std::numeric_limits<T>::min_exponent );
			}
			// use relative error
			return diff / std::min( ( absA + absB ), std::numeric_limits<T>::max( ) ) < std::numeric_limits<T>::epsilon( );
		}
#pragma GCC diagnostic pop

	} // namespace math
} // namespace daw
