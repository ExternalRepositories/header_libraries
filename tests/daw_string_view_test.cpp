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

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "boost_test.h"
#include "daw_string_view.h"

#define FALSE( b ) ( !( b ) )

BOOST_AUTO_TEST_CASE( daw_string_view_test_001 ) {
	constexpr daw::string_view const a = "This is a test";
	std::cout << a.to_string( ) << '\n';
	daw::string_view b = "Testing again" + a;

	daw::string_view c{a};
	c.remove_prefix( );
	std::cout << c << ' ' << c.front( ) << ' ' << c[3] << '\n';
	c = a;
	c = b;
}

BOOST_AUTO_TEST_CASE( daw_string_view_find_last_of_001 ) {
	daw::string_view const a = "abcdefghijklm";
	std::string const b = "abcdefghijklm";
	auto const pos = a.find_last_of( "ij" );
	auto const pos2 = b.find_last_of( "ij" );

	auto const es = a.find_last_of( "lm" );
	auto const es2 = b.find_last_of( "lm" );
	BOOST_REQUIRE_EQUAL( pos, pos2 );
	BOOST_REQUIRE_EQUAL( es, es2 );
}

BOOST_AUTO_TEST_CASE( tc001 ) {
	daw::string_view view;
	BOOST_TEST_MESSAGE( "Constructs an empty string" );

	{ BOOST_REQUIRE( view.empty( ) ); }
	BOOST_TEST_MESSAGE( "Is 0 bytes in size" );

	{ BOOST_REQUIRE( view.size( ) == 0 ); }

	BOOST_TEST_MESSAGE( "Points to null" );

	{ BOOST_REQUIRE( view.data( ) == nullptr ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc002 ) {
	std::string str = "Hello world";
	daw::string_view view = str;

	BOOST_TEST_MESSAGE( "Constructs a non-empty string" );
	{ BOOST_REQUIRE( FALSE( view.empty( ) ) ); }

	BOOST_TEST_MESSAGE( "Has non-zero size" );
	{ BOOST_REQUIRE( view.size( ) != 0 ); }

	BOOST_TEST_MESSAGE( "Points to original string" );
	{ BOOST_REQUIRE( view.data( ) == str.data( ) ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc003 ) {
	const char *empty_str = "";
	const char *non_empty_str = "Hello World";

	BOOST_TEST_MESSAGE( "Is empty with empty string" );
	{
		daw::string_view view = empty_str;

		BOOST_REQUIRE( view.empty( ) );
	}

	BOOST_TEST_MESSAGE( "Is non-empty with non-empty string" );
	{
		const char *str = "Hello world";
		daw::string_view view = non_empty_str;

		BOOST_REQUIRE( FALSE( view.empty( ) ) );
	}

	BOOST_TEST_MESSAGE( "Is size 0 with empty string" );
	{
		daw::string_view view = empty_str;

		BOOST_REQUIRE( view.size( ) == 0 );
	}

	BOOST_TEST_MESSAGE( "Is not size 0 with non-empty string" );
	{
		daw::string_view view = non_empty_str;

		BOOST_REQUIRE( view.size( ) != 0 );
	}

	BOOST_TEST_MESSAGE( "Points to original string" );
	{
		daw::string_view view = non_empty_str;

		BOOST_REQUIRE( view.data( ) == non_empty_str );
	}
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc004 ) {
	daw::string_view empty = "";
	daw::string_view view = "Hello world";

	BOOST_TEST_MESSAGE( "Is empty with empty string" );
	{ BOOST_REQUIRE( empty.empty( ) ); }

	BOOST_TEST_MESSAGE( "Is non-empty with non-empty string" );
	{ BOOST_REQUIRE( FALSE( view.empty( ) ) ); }

	BOOST_TEST_MESSAGE( "Is size 0 with empty string" );
	{ BOOST_REQUIRE( empty.size( ) == 0 ); }

	BOOST_TEST_MESSAGE( "Is not size 0 with non-empty string" );
	{ BOOST_REQUIRE( view.size( ) != 0 ); }
}

//----------------------------------------------------------------------------
// Capacity
//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc004capacity ) {
	const char *str = "Hello World";
	daw::string_view view = str;

	BOOST_TEST_MESSAGE( "Returns non-zero for non-empty string" );
	{ BOOST_REQUIRE( view.size( ) == std::char_traits<char>::length( str ) ); }

	BOOST_TEST_MESSAGE( "Returns 0 for empty string" );
	{
		view = "";

		BOOST_REQUIRE( view.size( ) == 0 );
	}
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc005capacity ) {
	const char *str = "Hello World";
	daw::string_view view = str;

	BOOST_TEST_MESSAGE( "Returns non-zero for non-empty string" );
	{ BOOST_REQUIRE( view.length( ) == std::char_traits<char>::length( str ) ); }

	BOOST_TEST_MESSAGE( "Returns 0 for empty string" );
	{
		view = "";

		BOOST_REQUIRE( view.length( ) == 0 );
	}
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc006capacity ) {
	const char *str = "Hello World";
	daw::string_view view = str;

	BOOST_TEST_MESSAGE( "Returns false on non-empty string" );
	{ BOOST_REQUIRE( FALSE( view.empty( ) ) ); }

	BOOST_TEST_MESSAGE( "Returns true on empty string" );
	{
		view = "";

		BOOST_REQUIRE( view.empty( ) );
	}
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc007accessor ) {
	const char *str = "Hello World";
	daw::string_view view = str;

	BOOST_TEST_MESSAGE( "Points to original data source" );
	{ BOOST_REQUIRE( view.c_str( ) == str ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc008accessor ) {
	const char *str = "Hello World";
	daw::string_view view = str;

	BOOST_TEST_MESSAGE( "Points to original data source" );
	{ BOOST_REQUIRE( view.data( ) == str ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc009accessor ) {
	const char *str = "Hello World";
	daw::string_view view = str;

	BOOST_TEST_MESSAGE( "Returns reference to entry at position" );
	{ BOOST_REQUIRE( &view[0] == str ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc010accessor ) {
	const char *str = "Hello World";
	daw::string_view view = str;

	BOOST_TEST_MESSAGE( "Returns reference to entry at position" );
	{ BOOST_REQUIRE( &view.at( 0 ) == str ); }

	BOOST_TEST_MESSAGE( "Throws when out of range" );
	{ BOOST_REQUIRE_THROW( view.at( 11 ), std::out_of_range ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc011accessor ) {
	daw::string_view view = "Hello World";

	BOOST_TEST_MESSAGE( "Returns reference to first character" );
	{ BOOST_REQUIRE( view.front( ) == 'H' ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc012accessor ) {
	daw::string_view view = "Hello World";

	BOOST_TEST_MESSAGE( "Returns reference to last character" );
	{ BOOST_REQUIRE( view.back( ) == 'd' ); }
}

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc013modifier ) {
	daw::string_view view = "Hello World";

	BOOST_TEST_MESSAGE( "Removes first n characters" );
	{
		view.remove_prefix( 6 );

		BOOST_REQUIRE( view == "World" );
	}
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc014modifier ) {
	daw::string_view view = "Hello World";

	BOOST_TEST_MESSAGE( "Removes last n characters" );
	{
		view.remove_suffix( 6 );

		BOOST_REQUIRE( view == "Hello" );
	}
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc015modifier ) {
	const char *str1 = "Hello World";
	const char *str2 = "Goodbye World";
	daw::string_view view1 = str1;
	daw::string_view view2 = str2;

	BOOST_TEST_MESSAGE( "Swaps entries" );
	{
		view1.swap( view2 );

		BOOST_REQUIRE( ( ( view1.data( ) == str2 ) && ( view2.data( ) == str1 ) ) );
	}
}

//----------------------------------------------------------------------------
// String Operations
//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc016conversion ) {
	daw::string_view view = "Hello World";

	std::string string = view.to_string( );

	BOOST_TEST_MESSAGE( "Copies view to new location in std::string" );
	{ BOOST_REQUIRE( view.data( ) != string.data( ) ); }

	BOOST_TEST_MESSAGE( "Copied string contains same contents as view" );
	{ BOOST_REQUIRE( string == "Hello World" ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc017conversion ) {
	daw::string_view view = "Hello World";

	std::string string = static_cast<std::string>( view );

	BOOST_TEST_MESSAGE( "Copies view to new location in std::string" );
	{ BOOST_REQUIRE( view.data( ) != string.data( ) ); }

	BOOST_TEST_MESSAGE( "Copied string contains same contents as view" );
	{ BOOST_REQUIRE( string == "Hello World" ); }
}

//----------------------------------------------------------------------------
// Operations
//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc018operation ) {
	daw::string_view const view = "Hello World";

	BOOST_TEST_MESSAGE( "Throws std::out_of_range if pos >= view.size()" );
	{
		char result[11];

		BOOST_REQUIRE_THROW( view.copy( result, 11, 11 ), std::out_of_range );
	}

	BOOST_TEST_MESSAGE( "Copies entire string" );
	{
		char result[11];
		view.copy( result, 11 );

		BOOST_REQUIRE( std::strncmp( result, "Hello World", 11 ) == 0 );
	}

	BOOST_TEST_MESSAGE( "Copies remaining characters if count > size" );
	{
		char result[11];
		view.copy( result, 20 );

		BOOST_REQUIRE( std::strncmp( result, "Hello World", 11 ) == 0 );
	}

	BOOST_TEST_MESSAGE( "Copies part of the string" );
	{
		char result[11];
		view.copy( result, 5 );

		BOOST_REQUIRE( std::strncmp( result, "Hello", 5 ) == 0 );
	}

	BOOST_TEST_MESSAGE( "Copies part of the string, offset from the beginning" );
	{
		char result[11];
		view.copy( result, 10, 6 );

		BOOST_REQUIRE( std::strncmp( result, "World", 5 ) == 0 );
	}

	BOOST_TEST_MESSAGE( "Returns number of characters copied" );
	{
		char result[11];

		BOOST_REQUIRE( view.copy( result, 20 ) == 11 );
	}
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc019operation ) {
	daw::string_view view = "Hello World";

	BOOST_TEST_MESSAGE( "Returns the full string when given no args" );
	{
		auto substr = view.substr( );
		BOOST_REQUIRE( substr == "Hello World" );
	}

	BOOST_TEST_MESSAGE( "Returns last part of string" );
	{
		auto substr = view.substr( 6 );
		BOOST_REQUIRE( substr == "World" );
	}

	BOOST_TEST_MESSAGE( "Substring returns at most count characters" );
	{
		auto substr = view.substr( 6, 1 );
		BOOST_REQUIRE( substr == "W" );
	}

	BOOST_TEST_MESSAGE( "Returns up to end of string if length > size" );
	{
		auto substr = view.substr( 6, 10 );
		BOOST_REQUIRE( substr == "World" );
	}

	BOOST_TEST_MESSAGE( "Throws std::out_of_range if pos > size" );
	{ BOOST_REQUIRE_THROW( view.substr( 15 ), std::out_of_range ); }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc020comparison ) {
	BOOST_TEST_MESSAGE( "Returns 0 for identical views" );
	{
		daw::string_view view = "Hello World";

		BOOST_REQUIRE( view.compare( "Hello World" ) == 0 );
	}

	BOOST_TEST_MESSAGE( "Returns nonzero for different views" );
	{
		daw::string_view view = "Hello World";

		BOOST_REQUIRE( view.compare( "Goodbye World" ) != 0 );
	}

	BOOST_TEST_MESSAGE( "Returns > 0 for substring beginning with same string" );
	{
		daw::string_view view = "Hello World";

		BOOST_REQUIRE( view.compare( "Hello" ) > 0 );
	}

	BOOST_TEST_MESSAGE( "Returns < 0 for superstring beginning with self" );
	{
		daw::string_view view = "Hello";

		BOOST_REQUIRE( view.compare( "Hello World" ) < 0 );
	}

	BOOST_TEST_MESSAGE( "Returns < 0 for same-sized string compared to character greater than char" );
	{
		daw::string_view view = "1234567";

		BOOST_REQUIRE( view.compare( "1234667" ) < 0 );
	}

	BOOST_TEST_MESSAGE( "Returns > 0 for same-sized string compared to character less than char" );
	{
		daw::string_view view = "1234567";

		BOOST_REQUIRE( view.compare( "1234467" ) > 0 );
	}
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc021comparison ) {
	daw::string_view view = "Hello World";

	BOOST_TEST_MESSAGE( "Is equal" );
	{
		BOOST_TEST_MESSAGE( "Returns true for equal string views" );
		{
			daw::string_view view2 = "Hello World";

			BOOST_REQUIRE( view == view2 );
		}

		BOOST_TEST_MESSAGE( "Returns true for equal strings with left char array" );
		{ BOOST_REQUIRE( "Hello World" == view ); }

		BOOST_TEST_MESSAGE( "Returns true for equal strings with right char array" );
		{ BOOST_REQUIRE( view == "Hello World" ); }

		BOOST_TEST_MESSAGE( "Returns true for equal strings with left char ptr" );
		{
			const char *str = "Hello World";
			BOOST_REQUIRE( str == view );
		}

		BOOST_TEST_MESSAGE( "Returns true for equal strings with right char ptr" );
		{
			const char *str = "Hello World";
			BOOST_REQUIRE( view == str );
		}

		BOOST_TEST_MESSAGE( "Returns true for equal strings with left std::string" );
		{
			std::string str = "Hello World";
			BOOST_REQUIRE( str == view );
		}

		BOOST_TEST_MESSAGE( "Returns true for equal strings with right std::string" );
		{
			std::string str = "Hello World";
			BOOST_REQUIRE( view == str );
		}
	}

	BOOST_TEST_MESSAGE( "Is not equal" );
	{
		BOOST_TEST_MESSAGE( "Returns false for non-equal string views" );
		{
			daw::string_view view2 = "Goodbye World";

			BOOST_REQUIRE( FALSE( view == view2 ) );
		}

		BOOST_TEST_MESSAGE( "Returns false for non-equal strings with left char array" );
		{ BOOST_REQUIRE( FALSE( "Goodbye World" == view ) ); }

		BOOST_TEST_MESSAGE( "Returns false for non-equal strings with right char array" );
		{ BOOST_REQUIRE( FALSE( view == "Goodbye World" ) ); }

		BOOST_TEST_MESSAGE( "Returns false for non-equal strings with left char ptr" );
		{
			const char *str = "Goodbye World";
			BOOST_REQUIRE( FALSE( str == view ) );
		}

		BOOST_TEST_MESSAGE( "Returns false for non-equal strings with right char ptr" );
		{
			const char *str = "Goodbye World";
			BOOST_REQUIRE( FALSE( view == str ) );
		}

		BOOST_TEST_MESSAGE( "Returns false for non-equal strings with left std::string" );
		{
			std::string str = "Goodbye World";
			BOOST_REQUIRE( FALSE( str == view ) );
		}

		BOOST_TEST_MESSAGE( "Returns false for non-equal strings with right std::string" );
		{
			std::string str = "Goodbye World";
			BOOST_REQUIRE( FALSE( view == str ) );
		}
	}
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( tc022comparison ) {
	daw::string_view view = "Hello World";

	BOOST_TEST_MESSAGE( "Is equal" );
	{
		BOOST_TEST_MESSAGE( "Returns false for equal string views" );
		{
			daw::string_view view2 = "Hello World";

			BOOST_REQUIRE( FALSE( view != view2 ) );
		}

		BOOST_TEST_MESSAGE( "Returns false for equal strings with left char array" );
		{ BOOST_REQUIRE( FALSE( "Hello World" != view ) ); }

		BOOST_TEST_MESSAGE( "Returns false for equal strings with right char array" );
		{ BOOST_REQUIRE( FALSE( view != "Hello World" ) ); }

		BOOST_TEST_MESSAGE( "Returns false for equal strings with left char ptr" );
		{
			const char *str = "Hello World";
			BOOST_REQUIRE( FALSE( str != view ) );
		}

		BOOST_TEST_MESSAGE( "Returns false for equal strings with right char ptr" );
		{
			const char *str = "Hello World";
			BOOST_REQUIRE( FALSE( view != str ) );
		}

		BOOST_TEST_MESSAGE( "Returns false for equal strings with left std::string" );
		{
			std::string str = "Hello World";
			BOOST_REQUIRE( FALSE( str != view ) );
		}

		BOOST_TEST_MESSAGE( "Returns false for equal strings with right std::string" );
		{
			std::string str = "Hello World";
			BOOST_REQUIRE( FALSE( view != str ) );
		}
	}

	BOOST_TEST_MESSAGE( "Is not equal" );
	{
		BOOST_TEST_MESSAGE( "Returns true for non-equal string views" );
		{
			daw::string_view view2 = "Goodbye World";

			BOOST_REQUIRE( view != view2 );
		}

		BOOST_TEST_MESSAGE( "Returns true for non-equal strings with left char array" );
		{ BOOST_REQUIRE( "Goodbye World" != view ); }

		BOOST_TEST_MESSAGE( "Returns true for non-equal strings with right char array" );
		{ BOOST_REQUIRE( view != "Goodbye World" ); }

		BOOST_TEST_MESSAGE( "Returns true for non-equal strings with left char ptr" );
		{
			const char *str = "Goodbye World";
			BOOST_REQUIRE( str != view );
		}

		BOOST_TEST_MESSAGE( "Returns true for non-equal strings with right char ptr" );
		{
			const char *str = "Goodbye World";
			BOOST_REQUIRE( view != str );
		}

		BOOST_TEST_MESSAGE( "Returns true for non-equal strings with left std::string" );
		{
			std::string str = "Goodbye World";
			BOOST_REQUIRE( str != view );
		}

		BOOST_TEST_MESSAGE( "Returns true for non-equal strings with right std::string" );
		{
			std::string str = "Goodbye World";
			BOOST_REQUIRE( view != str );
		}
	}
}
