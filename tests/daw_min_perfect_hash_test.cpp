// The MIT License (MIT)
//
// Copyright (c) 2019 Darrell Wright
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

#include "daw/daw_benchmark.h"
#include "daw/daw_fnv1a_hash.h"
#include "daw/daw_metro_hash.h"
#include "daw/daw_min_perfect_hash.h"
#include "daw/daw_string_view.h"

#include <cstdint>
#include <unordered_map>

struct IntHasher {
	template<typename Integer>
	constexpr size_t operator( )( Integer const &i, size_t seed ) const {
		return static_cast<size_t>( i ) ^ seed;
	}
	template<typename Integer>
	constexpr size_t operator( )( Integer const &i ) const {
		return static_cast<size_t>( i ) ^ daw::impl::fnv_prime( );
	}
};

struct HashMe {
	template<typename Value>
	constexpr size_t operator( )( Value const &v, size_t seed ) const {
		if constexpr( std::is_integral_v<Value> ) {
			return IntHasher{}( v, seed );
		} else {
			return daw::fnv1a_hash_t{}( v, seed );
		}
	}
	template<typename Value>
	constexpr size_t operator( )( Value const &v ) const {
		if constexpr( std::is_integral_v<Value> ) {
			return IntHasher{}( v );
		} else {
			return daw::fnv1a_hash_t{}( v );
		}
	}
};

inline constexpr std::pair<int, char const *> data_items[13] = {
  {100, "Continue"},
  {101, "Switching Protocols"},
  {102, "Processing"},
  {200, "OK"},
  {201, "Created"},
  {202, "Accepted"},
  {203, "Non-Authoritative Information"},
  {204, "No Content"},
  {205, "Reset Content"},
  {206, "Partial Content"},
  {207, "Multi-Status"},
  {208, "Already Reported"},
  {226, "IM Used"}};

inline constexpr auto ph =
  daw::perfect_hash_table<13, int, char const *, HashMe>( data_items );

using namespace std::string_view_literals;
static_assert( ph[100] == "Continue"sv );
static_assert( ph[101] == "Switching Protocols"sv );
static_assert( ph[102] == "Processing"sv );
static_assert( ph[200] == "OK"sv );
static_assert( ph[201] == "Created"sv );
static_assert( ph[202] == "Accepted"sv );
static_assert( ph[203] == "Non-Authoritative Information"sv );
static_assert( ph[204] == "No Content"sv );
static_assert( ph[205] == "Reset Content"sv );
static_assert( ph[206] == "Partial Content"sv );
static_assert( ph[207] == "Multi-Status"sv );
static_assert( ph[208] == "Already Reported"sv );
static_assert( ph[226] == "IM Used"sv );
static_assert( ph.find( 1234 ) == ph.end( ) );
static_assert( not ph.contains( 1234 ) );
static_assert( ph.contains( 204 ) );

char const *lookup( int key ) {
	return ph[key];
}

constexpr uint32_t u32( char const ( &str )[5] ) {
	uint32_t result = static_cast<uint32_t>( str[0] );
	for( size_t n = 1; n < 4; ++n ) {
		result <<= 8U;
		result |= static_cast<uint32_t>( str[n] );
	}
	return result;
}

inline constexpr std::pair<uint32_t, bool> values[16]{
  {u32( "INFO" ), true}, {u32( "CONN" ), true}, {u32( "PUB " ), true},
  {u32( "SUB " ), true}, {u32( "UNSU" ), true}, {u32( "PING" ), true},
  {u32( "PONG" ), true}, {u32( "+OK " ), true}, {u32( "-ERR" ), true},
  {u32( "AUTH" ), true}, {u32( "PUSH" ), true}, {u32( "ADD " ), true},
  {u32( "DECR" ), true}, {u32( "SET " ), true}, {u32( "GET " ), true},
  {u32( "QUIT" ), true}};

inline constexpr std::pair<std::string_view, bool> values2[16]{
  {"INFO"sv, true}, {"CONN"sv, true}, {"PUB "sv, true}, {"SUB "sv, true},
  {"UNSU"sv, true}, {"PING"sv, true}, {"PONG"sv, true}, {"+OK "sv, true},
  {"-ERR"sv, true}, {"AUTH"sv, true}, {"PUSH"sv, true}, {"ADD "sv, true},
  {"DECR"sv, true}, {"SET "sv, true}, {"GET "sv, true}, {"QUIT"sv, true}};

template<size_t N>
using matching_unsigned_t = std::conditional_t<
  N == 64, uint64_t,
  std::conditional_t<
    N == 32, uint32_t,
    std::conditional_t<N == 16, uint16_t,
                       std::conditional_t<N == 8, uint8_t, uintmax_t>>>>;

void test_min_perf_hash( ) {
	constexpr auto hm =
	  daw::perfect_hash_table<16, uint32_t, bool, IntHasher>( values );
	daw::bench_n_test<10000>(
	  "Minimal Perfect HashMap - uint32_t key",
	  [&]( auto m ) {
		  daw::do_not_optimize( m );
		  size_t result = 0;
		  daw::do_not_optimize( result );
		  for( auto &k : values ) {
			  result += static_cast<size_t>( m[k.first] );
		  }
		  daw::do_not_optimize( result );
	  },
	  hm );
}

struct MetroHash {
	template<typename Integer, std::enable_if_t<std::is_integral_v<Integer>,
	                                            std::nullptr_t> = nullptr>
	constexpr size_t operator( )( Integer value, size_t seed = 0 ) const {
		using int_t = matching_unsigned_t<sizeof( Integer )>;
		std::array<char, sizeof( Integer )> buff{};
		for( size_t n = 0; n < sizeof( Integer ); ++n ) {
			int_t mask = 0xFFU << ( 8U * n );
			buff[n] =
			  static_cast<int_t>( static_cast<int_t>( value & mask ) >> ( 8U * n ) );
		}
		return daw::metro::hash64(
		  daw::view<char const *>( buff.begin( ), buff.end( ) ), seed );
	}
	constexpr size_t operator( )( std::string_view sv, size_t seed = 0 ) const {
		return daw::metro::hash64( {sv.begin( ), sv.end( )}, seed );
	}
};

void test_min_perf_hash2( ) {
	constexpr auto hm =
	  daw::perfect_hash_table<16, std::string_view, bool, MetroHash>( values2 );
	daw::bench_n_test<10000>(
	  "Minimal Perfect HashMap - string_view key",
	  [&]( auto m ) {
		  daw::do_not_optimize( m );
		  size_t result = 0;
		  daw::do_not_optimize( result );
		  for( auto &k : values2 ) {
			  result += static_cast<size_t>( m[k.first] );
		  }
		  daw::do_not_optimize( result );
	  },
	  hm );
}

void test_unorderd_map( ) {
	auto const hm = std::unordered_map<uint32_t, bool>( std::begin( values ),
	                                                    std::end( values ) );
	daw::bench_n_test<10000>(
	  "std::unordered_map - uint32_t key",
	  [&]( auto m ) {
		  daw::do_not_optimize( m );
		  size_t result = 0;
		  daw::do_not_optimize( result );
		  for( auto &k : values ) {
			  result += static_cast<size_t>( m[k.first] );
		  }
		  daw::do_not_optimize( result );
	  },
	  hm );
}

void test_unorderd_map2( ) {
	auto const hm = std::unordered_map<std::string_view, bool>(
	  std::begin( values2 ), std::end( values2 ) );
	daw::bench_n_test<10000>(
	  "std::unordered_map - string_view key - std::hash",
	  [&]( auto m ) {
		  daw::do_not_optimize( m );
		  size_t result = 0;
		  daw::do_not_optimize( result );
		  for( auto &k : values2 ) {
			  result += static_cast<size_t>( m[k.first] );
		  }
		  daw::do_not_optimize( result );
	  },
	  hm );
}

void test_unorderd_map3( ) {
	auto const hm = std::unordered_map<std::string_view, bool, MetroHash>(
	  std::begin( values2 ), std::end( values2 ) );
	daw::bench_n_test<10000>(
	  "std::unordered_map - string_view key - MetroHash",
	  [&]( auto m ) {
		  daw::do_not_optimize( m );
		  size_t result = 0;
		  daw::do_not_optimize( result );
		  for( auto &k : values2 ) {
			  result += static_cast<size_t>( m[k.first] );
		  }
		  daw::do_not_optimize( result );
	  },
	  hm );
}

int main( ) {
	daw::expecting( ph.contains( 207 ) );
	test_min_perf_hash( );
	test_unorderd_map( );
	test_min_perf_hash2( );
	test_unorderd_map2( );
	test_unorderd_map3( );
	return 0;
}