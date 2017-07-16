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

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <exception>
#include <functional>
#include <list>
#include <utility>
#include <vector>

#include "daw_string_view.h"
#include "daw_traits.h"

namespace daw {
	namespace parser {
		template<typename CharT, typename TraitsT, typename Predicate>
		constexpr auto find_first_of( daw::basic_string_view<CharT, TraitsT> str, CharT value ) noexcept {
			auto it = str.begin( );
			while( it != str.end( ) && *it != value ) {
				++it;
			}
			return it;
		}

		template<typename CharT, typename TraitsT, typename Predicate>
		constexpr auto find_first_of_when( daw::basic_string_view<CharT, TraitsT> str, Predicate pred ) noexcept( noexcept( pred( CharT{ }} ) ) ) {
			auto it = str.begin( );
			while( it != str.end( ) && !pred( *it ) ) {
				++it;
			}
			return it;
		}
	} // namespace parser
} // namespace daw

