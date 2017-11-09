// The MIT License (MIT)
//
// Copyright (c) 2016-2017 Darrell Wright
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

#include <limits>

#include "daw_traits.h"

namespace daw {
	template<typename T>
	constexpr T get_left_mask( size_t left_zero_bits ) noexcept {
		return static_cast<T>( std::numeric_limits<T>::max( ) >> left_zero_bits );
	}

	template<typename T>
	constexpr T get_right_mask( size_t right_zero_bits ) noexcept {
		return static_cast<T>( std::numeric_limits<T>::max( ) << right_zero_bits );
	}

	template<typename Bit, typename... Bits>
	constexpr auto make_mask( Bit bit, Bits... bits ) noexcept {
		static_assert( is_integral_v<Bit>, "Only integer types are supported" );

		using expand = int[];
		static_cast<void>( expand{0, ( ( bit |= bits ), 0 )...} );
		return bit;
	}

	template<typename Integer, typename MaskBit, typename... MaskBits>
	constexpr bool are_set( Integer value, MaskBit mask_bit, MaskBits... mask_bits ) noexcept {
		static_assert( is_integral_v<Integer>, "Only integer types are supported" );
		static_assert( is_integral_v<MaskBit>, "Only integer types are supported" );

		return ( value & make_mask( mask_bit, mask_bits... ) ) != 0;
	}
} // namespace daw

