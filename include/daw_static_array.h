// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
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

#include <cstddef>

#include "daw_array_view.h"
#include "daw_span.h"

namespace daw {
	template<typename T, size_t N>
	struct static_array_t {
		using value_type = T;
		using reference = value_type &;
		using const_reference = value_type const &;
		using iterator = value_type *;
		using const_iterator = value_type const *;
		using pointer = value_type *;
		using const_pointer = value_type const *;
		using size_type = size_t;

		value_type m_data[N];

		constexpr reference front( ) noexcept {
			return m_data[0];
		}

		constexpr const_reference front( ) const noexcept {
			return m_data[0];
		}

		constexpr reference back( ) noexcept {
			return m_data[N - 1];
		}

		constexpr const_reference back( ) const noexcept {
			return m_data[N - 1];
		}

		constexpr pointer data( ) noexcept {
			return m_data;
		}

		constexpr const_pointer data( ) const noexcept {
			return m_data;
		}

		constexpr iterator begin( ) noexcept {
			return m_data;
		}

		constexpr const_iterator begin( ) const noexcept {
			return m_data;
		}

		constexpr const_iterator cbegin( ) const noexcept {
			return m_data;
		}

		constexpr iterator end( ) noexcept {
			return m_data + static_cast<intmax_t>( N );
		}

		constexpr const_iterator end( ) const noexcept {
			return m_data + static_cast<intmax_t>( N );
		}

		constexpr const_iterator cend( ) const noexcept {
			return m_data + static_cast<intmax_t>( N );
		}

		constexpr void fill( const_reference value ) noexcept {
			for( size_t n = 0; n < N; ++n ) {
				m_data[n] = value;
			}
		}

		constexpr size_type size( ) const noexcept {
			return N;
		}

		constexpr bool empty( ) const noexcept {
			return N == 0;
		}

		constexpr reference operator[]( size_type pos ) noexcept {
			return m_data[pos];
		}

		constexpr const_reference operator[]( size_type pos ) const noexcept {
			return m_data[pos];
		}

		constexpr daw::span<value_type const>
		to_span( size_type const pos = 0, size_type const count = std::numeric_limits<size_type>::max( ) ) const {
			if( pos >= size( ) ) {
				throw std::out_of_range{"Attempt to access array_view past end"};
			}
			auto const rcount = std::min( count, N - pos );
			return daw::span<value_type>{m_data + pos, rcount};
		}

		constexpr daw::span<value_type> to_span( size_type const pos = 0,
		                                         size_type const count = std::numeric_limits<size_type>::max( ) ) {
			if( pos >= size( ) ) {
				throw std::out_of_range{"Attempt to access array_view past end"};
			}
			auto const rcount = std::min( count, N - pos );
			return daw::span<value_type>{m_data + pos, rcount};
		}

		constexpr daw::array_view<value_type>
		to_array_view( size_type const pos = 0, size_type const count = std::numeric_limits<size_type>::max( ) ) const {
			if( pos >= size( ) ) {
				throw std::out_of_range{"Attempt to access array_view past end"};
			}
			auto const rcount = std::min( count, N - pos );
			return daw::array_view<value_type>{m_data + pos, rcount};
		}
	}; // static_array_t
} // namespace daw

