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

#include <stdexcept>
#include <type_traits>

#include "daw_exception.h"
#include "daw_fnv1a_hash.h"
#include "daw_stack_array.h"
#include "daw_traits.h"
#include "daw_utility.h"

namespace daw {
	namespace impl {
		namespace sentinals {
			enum sentinals : size_t { empty = 0, removed, sentinals_size };
		} // namespace sentinals
	}   // namespace impl

	template<typename Value, size_t N>
	struct fixed_lookup {
		static_assert( N > 0, "Must supply a positive initial_size larger than 0" );

		using value_type = daw::traits::root_type_t<Value>;
		using reference = value_type &;
		using const_reference = value_type const &;

	private:
		daw::array_t<size_t, N> m_hashes;
		daw::array_t<value_type, N> m_values;

		static constexpr size_t capacity( ) noexcept {
			return N;
		}

		constexpr size_t size( ) noexcept {
			size_t count = 0;
			for( auto hash : m_hashes ) {
				count += hash >= impl::sentinals::sentinals_size ? 1 : 0;
			}
			return count;
		}

		template<typename KeyType>
		static constexpr size_t hash_fn( KeyType &&key ) noexcept {
			return ( daw::fnv1a_hash( std::forward<KeyType>( key ) ) %
			         ( std::numeric_limits<size_t>::max( ) - impl::sentinals::sentinals_size ) ) +
			       impl::sentinals::sentinals_size;
		}

		constexpr size_t scale_hash( size_t const hash ) const {
			// Scale value to capacity using MAD(Multiply-Add-Divide) compression
			// Use the two largest Prime's that fit in a 64bit unsigned integral
			daw::exception::daw_throw_on_false( capacity( ) > 0 );
			daw::exception::daw_throw_on_false( hash >= impl::sentinals::sentinals_size );

			size_t const prime_a = 18446744073709551557u;
			size_t const prime_b = 18446744073709551533u;
			return ( hash * prime_a + prime_b ) % capacity( );
		}

		constexpr auto lookup( size_t const hash ) const noexcept {
			struct lookup_result_t {
				size_t position;
				bool found;

				constexpr lookup_result_t( size_t pos, bool is_found ) noexcept : position{pos}, found{is_found} {}
				constexpr lookup_result_t( ) noexcept = delete;
				constexpr lookup_result_t( lookup_result_t const & ) noexcept = default;
				constexpr lookup_result_t( lookup_result_t && ) noexcept = default;
				constexpr lookup_result_t &operator=( lookup_result_t const & ) noexcept = default;
				constexpr lookup_result_t &operator=( lookup_result_t && ) noexcept = default;
				~lookup_result_t( ) = default;

				constexpr operator bool( ) const noexcept {
					return found;
				}
			};

			auto const hash_pos = scale_hash( hash );
			for( size_t n = hash_pos; n != m_hashes.size( ); ++n ) {
				if( hash == m_hashes[n] ) {
					return lookup_result_t{n, true};
				} else if( impl::sentinals::empty == m_hashes[n] ) {
					return lookup_result_t{n, false};
				}
			}
			for( size_t n = 0; n != hash_pos; ++n ) {
				if( m_hashes[n] == hash ) {
					return lookup_result_t{n, true};
				} else if( m_hashes[n] == impl::sentinals::empty ) {
					return lookup_result_t{n, false};
				}
			}
			return lookup_result_t{m_hashes.size( ), false};
		}

	public:
		constexpr fixed_lookup( ) = default;
		constexpr fixed_lookup( fixed_lookup const &other ) noexcept = default;
		constexpr fixed_lookup( fixed_lookup &&other ) noexcept = default;
		constexpr fixed_lookup &operator=( fixed_lookup const &other ) noexcept = default;
		constexpr fixed_lookup &operator=( fixed_lookup &&other ) noexcept = default;
		~fixed_lookup( ) = default;

		template<typename Key>
		constexpr const_reference operator[]( Key &&key ) const {
			auto const hash = hash_fn( std::forward<Key>( key ) );
			auto const is_found = lookup( hash );
			daw::exception::daw_throw_on_false( is_found, "Attempt to access an undefined key" );
			return m_values[is_found.position];
		}

		template<typename Key>
		constexpr reference operator[]( Key &&key ) {
			auto const hash = hash_fn( std::forward<Key>( key ) );
			auto const is_found = lookup( hash );
			daw::exception::daw_throw_on_true( !is_found && is_found.position == m_hashes.size( ),
			                                   "Fixed hash table does not have enough space to allocate all entries" );
			m_hashes[is_found.position] = hash;
			return m_values[is_found.position];
		}

		template<typename Key>
		constexpr bool exists( Key &&key ) const noexcept {
			auto const hash = hash_fn( std::forward<Key>( key ) );
			return static_cast<bool>( lookup( hash ) );
		}
	};
} // namespace daw

