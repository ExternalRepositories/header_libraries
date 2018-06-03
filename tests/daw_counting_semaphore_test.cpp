// The MIT License (MIT)
//
// Copyright (c) 2018 Darrell Wright
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

#include "boost_test.h"
#include <chrono>
#include <iostream>
#include <thread>

#include "daw_counting_semaphore.h"

BOOST_AUTO_TEST_CASE( construction_001 ) {
	daw::counting_semaphore sem1;
	auto sem1b = daw::counting_semaphore( );
	auto sem2 = daw::shared_counting_semaphore( std::move( sem1 ) );
}

BOOST_AUTO_TEST_CASE( barrier_001 ) {
	constexpr size_t const count = 5;
	auto sem = daw::shared_counting_semaphore( count );
	for( size_t n = 0; n < count; ++n ) {
		using namespace std::chrono_literals;
		auto th = std::thread( [sem]( ) mutable {
			std::this_thread::sleep_for( 2s );
			sem.notify( );
		} );
		th.detach( );
	}
	sem.wait( );
}