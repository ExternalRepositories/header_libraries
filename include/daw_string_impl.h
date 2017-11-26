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

#include "daw_traits.h"

namespace daw {
	template<typename CharT, typename Traits = std::char_traits<CharT>, typename InternalSizeType = size_t>
	struct basic_string_view;

	template<typename CharT, size_t Capacity, typename Traits = std::char_traits<CharT>,
	         typename InternalSizeType = size_t>
	struct basic_static_string;

	namespace details {
		template<class ForwardIt1, class ForwardIt2>
		constexpr ForwardIt1 search( ForwardIt1 first, ForwardIt1 last, ForwardIt2 s_first,
		                             ForwardIt2 s_last ) noexcept( noexcept( *first == *s_first ) ) {
			for( ;; ++first ) {
				ForwardIt1 it = first;
				for( ForwardIt2 s_it = s_first;; ++it, ++s_it ) {
					if( s_it == s_last ) {
						return first;
					}
					if( it == last ) {
						return last;
					}
					if( !( *it == *s_it ) ) {
						break;
					}
				}
			}
		}

		template<class ForwardIt1, class ForwardIt2, class BinaryPredicate>
		constexpr ForwardIt1 search( ForwardIt1 first, ForwardIt1 last, ForwardIt2 s_first, ForwardIt2 s_last,
		                             BinaryPredicate p ) noexcept( noexcept( !p( *first, *s_first ) ) ) {
			for( ;; ++first ) {
				ForwardIt1 it = first;
				for( ForwardIt2 s_it = s_first;; ++it, ++s_it ) {
					if( s_it == s_last ) {
						return first;
					}
					if( it == last ) {
						return last;
					}
					if( !p( *it, *s_it ) ) {
						break;
					}
				}
			}
		}

		template<typename SizeT, typename CharT>
		constexpr SizeT strlen( CharT const *const str ) noexcept {
			auto pos = str;
			while( *( pos ) != 0 ) {
				++pos;
			}
			return static_cast<SizeT>( pos - str );
		}

		template<typename InputIt, typename ForwardIt, typename BinaryPredicate>
		constexpr InputIt
		find_first_of( InputIt first, InputIt last, ForwardIt s_first, ForwardIt s_last, BinaryPredicate p ) noexcept(
		  noexcept( std::declval<BinaryPredicate>( )( *std::declval<InputIt>( ), *std::declval<ForwardIt>( ) ) ) ) {
			static_assert( daw::is_binary_predicate_v<BinaryPredicate, typename std::iterator_traits<InputIt>::value_type>,
			               "BinaryPredicate p does not fullfill the requires of a binary predicate concept.  See "
			               "http://en.cppreference.com/w/cpp/concept/BinaryPredicate" );
			for( ; first != last; ++first ) {
				for( ForwardIt it = s_first; it != s_last; ++it ) {
					if( p( *first, *it ) ) {
						return first;
					}
				}
			}
			return last;
		}

		template<typename InputIt, typename ForwardIt, typename BinaryPredicate>
		constexpr InputIt
		find_first_not_of( InputIt first, InputIt last, ForwardIt s_first, ForwardIt s_last, BinaryPredicate p ) noexcept(
		  noexcept( std::declval<BinaryPredicate>( )( *std::declval<InputIt>( ), *std::declval<ForwardIt>( ) ) ) ) {
			static_assert( daw::is_binary_predicate_v<BinaryPredicate, typename std::iterator_traits<InputIt>::value_type>,
			               "BinaryPredicate p does not fullfill the requires of a binary predicate concept.  See "
			               "http://en.cppreference.com/w/cpp/concept/BinaryPredicate" );
			bool found = false;
			for( ; first != last; ++first ) {
				found = false;
				for( ForwardIt it = s_first; it != s_last; ++it ) {
					if( p( *first, *it ) ) {
						found = true;
						break;
					}
				}
				if( !found ) {
					return first;
				}
			}
			return last;
		}

		template<typename InputIt, typename UnaryPredicate>
		constexpr InputIt find_first_of_if( InputIt first, InputIt last, UnaryPredicate p ) noexcept(
		  noexcept( std::declval<UnaryPredicate>( )( *std::declval<InputIt>( ) ) ) ) {
			static_assert( daw::is_unary_predicate_v<UnaryPredicate, typename std::iterator_traits<InputIt>::value_type>,
			               "UnaryPredicate p does not fullfill the requires of a unary predicate concept.  See "
			               "http://en.cppreference.com/w/cpp/concept/Predicate" );
			for( ; first != last; ++first ) {
				if( p( *first ) ) {
					return first;
				}
			}
			return last;
		}

		template<typename InputIt, typename UnaryPredicate>
		constexpr InputIt find_first_not_of_if( InputIt first, InputIt last, UnaryPredicate p ) noexcept(
		  noexcept( std::declval<UnaryPredicate>( )( *std::declval<InputIt>( ) ) ) ) {
			static_assert( daw::is_unary_predicate_v<UnaryPredicate, typename std::iterator_traits<InputIt>::value_type>,
			               "UnaryPredicate p does not fullfill the requires of a unary predicate concept.  See "
			               "http://en.cppreference.com/w/cpp/concept/Predicate" );
			for( ; first != last; ++first ) {
				if( !p( *first ) ) {
					return first;
				}
			}
			return last;
		}

		template<typename charT, typename traits>
		inline void sv_insert_fill_chars( std::basic_ostream<charT, traits> &os, std::size_t n ) {
			enum { chunk_size = 8 };
			charT fill_chars[chunk_size];
			std::fill_n( fill_chars, static_cast<std::size_t>( chunk_size ), os.fill( ) );
			for( ; n >= chunk_size && os.good( ); n -= chunk_size ) {
				os.write( fill_chars, static_cast<std::size_t>( chunk_size ) );
			}
			if( n > 0 && os.good( ) ) {
				os.write( fill_chars, static_cast<std::streamsize>( n ) );
			}
		}

		template<typename charT, typename traits>
		void sv_insert_aligned( std::basic_ostream<charT, traits> &os, daw::basic_string_view<charT, traits> const &str ) {
			auto const size = str.size( );
			auto const alignment_size = static_cast<std::size_t>( os.width( ) ) - size;
			bool const align_left =
			  ( os.flags( ) & std::basic_ostream<charT, traits>::adjustfield ) == std::basic_ostream<charT, traits>::left;
			if( !align_left ) {
				sv_insert_fill_chars( os, alignment_size );
				if( os.good( ) ) {
					os.write( str.data( ), static_cast<std::streamsize>( size ) );
				}
			} else {
				os.write( str.data( ), static_cast<std::streamsize>( size ) );
				if( os.good( ) ) {
					sv_insert_fill_chars( os, alignment_size );
				}
			}
		}

		template<typename charT, size_t Capacity, typename traits>
		void sv_insert_aligned( std::basic_ostream<charT, traits> &os,
		                        daw::basic_static_string<charT, Capacity, traits> const &str ) {
			auto const size = str.size( );
			auto const alignment_size = static_cast<std::size_t>( os.width( ) ) - size;
			bool const align_left =
			  ( os.flags( ) & std::basic_ostream<charT, traits>::adjustfield ) == std::basic_ostream<charT, traits>::left;
			if( !align_left ) {
				sv_insert_fill_chars( os, alignment_size );
				if( os.good( ) ) {
					os.write( str.data( ), static_cast<std::streamsize>( size ) );
				}
			} else {
				os.write( str.data( ), static_cast<std::streamsize>( size ) );
				if( os.good( ) ) {
					sv_insert_fill_chars( os, alignment_size );
				}
			}
		}

		// Make argument a lower priority than T[]
		template<typename T, std::enable_if_t<is_pointer_v<T>, std::nullptr_t> = nullptr>
		struct only_ptr {
			T *ptr;

			constexpr only_ptr( T *p ) noexcept
			  : ptr{p} {}

			constexpr operator T *( ) const noexcept {
				return ptr;
			}
		};
	} // namespace details

} // namespace daw
