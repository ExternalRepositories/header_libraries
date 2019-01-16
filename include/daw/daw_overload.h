// The MIT License (MIT)
//
// Copyright (c) 2018-2019 Darrell Wright
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

#include <type_traits>

#include "cpp_17.h"
#include "daw_traits.h"

namespace daw {
	// overload_t/overload create a callable with the overloads of operator( )
	// provided
	//
	template<class... Fs>
	struct overload : Fs... {
		template<class... Ts,
		         std::enable_if_t<
		           (sizeof...( Ts ) != 1U or
		            !std::is_same_v<overload, std::remove_reference_t<
		                                        daw::traits::first_type<Ts...>>>),
		           std::nullptr_t> = nullptr>
		constexpr overload( Ts &&... ts )
		  : Fs{std::forward<Ts>( ts )}... {}

		using Fs::operator( )...;
	};

	template<class... Ts>
	overload( Ts &&... )->overload<std::remove_reference_t<Ts>...>;
} // namespace daw
