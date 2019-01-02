// The MIT License ( MIT )
//
// Copyright ( c ) 2018-2019 Darrell Wright
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

#include <cstddef>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include "cpp_17.h"
#include "daw_exception.h"
#include "daw_move.h"
#include "daw_overload.h"
#include "daw_traits.h"
#include "daw_utility.h"
#include "daw_visit.h"

namespace daw {
	template<class T>
	struct expected_t {
		using value_type = T;
		using reference = value_type &;
		using const_reference = value_type const &;
		using pointer = value_type *;
		using const_pointer = value_type const *;

	private:
		std::variant<std::exception_ptr, value_type> m_value =
		  std::exception_ptr( nullptr );

	public:
		struct exception_tag {};
		expected_t( ) = default;
		~expected_t( ) = default;
		expected_t( expected_t const & ) = default;
		expected_t &operator=( expected_t const & ) = default;
		expected_t( expected_t &&other ) noexcept
		  : m_value( std::exchange( other.m_value, std::exception_ptr{nullptr} ) ) {
		}

		expected_t &operator=( expected_t &&rhs ) noexcept {
			m_value = std::exchange( rhs.m_value, std::exception_ptr{nullptr} );
			return *this;
		}

		//////////////////////////////////////////////////////////////////////////
		/// Summary: No value, aka null
		//////////////////////////////////////////////////////////////////////////
		friend bool operator==( expected_t const &lhs, expected_t const &rhs ) {
			return lhs.m_value == rhs.m_value;
		}

		friend bool operator!=( expected_t const &lhs, expected_t const &rhs ) {
			return lhs.m_value != rhs.m_value;
		}

		//////////////////////////////////////////////////////////////////////////
		/// Summary: With value
		//////////////////////////////////////////////////////////////////////////
		explicit expected_t( value_type &&value )
		  : m_value( daw::move( value ) ) {}

		explicit expected_t( value_type const &value )
		  : m_value( value ) {}

		expected_t &operator=( value_type &&value ) {
			m_value = daw::move( value );
			return *this;
		}

		expected_t &operator=( value_type const &value ) {
			m_value = value;
			return *this;
		}

		expected_t( std::exception_ptr ptr )
		  : m_value( ptr ) {}

		expected_t &operator=( std::exception_ptr ptr ) {
			m_value = ptr;
			return *this;
		}

		void clear( ) {
			m_value = std::exception_ptr{nullptr};
		}

		template<typename ExceptionType>
		expected_t( exception_tag, ExceptionType &&ex )
		  : m_value(
		      std::make_exception_ptr( std::forward<ExceptionType>( ex ) ) ) {}

		explicit expected_t( exception_tag )
		  : m_value( std::current_exception( ) ) {}

	private:
		template<class Function, typename... Args,
		         std::enable_if_t<
		           traits::is_callable_convertible_v<value_type, Function, Args...>,
		           std::nullptr_t> = nullptr>
		static std::variant<std::exception_ptr, value_type>
		variant_from_code( Function &&func, Args &&... args ) {
			try {
				return func( std::forward<Args>( args )... );
			} catch( ... ) { return std::current_exception( ); }
		}

	public:
		template<class Function, typename... Args,
		         std::enable_if_t<
		           traits::is_callable_convertible_v<value_type, Function, Args...>,
		           std::nullptr_t> = nullptr>
		explicit expected_t( Function &&func, Args &&... args )
		  : m_value( variant_from_code( std::forward<Function>( func ),
		                                std::forward<Args>( args )... ) ) {}

		template<class Function, typename... Args,
		         std::enable_if_t<
		           traits::is_callable_convertible_v<value_type, Function, Args...>,
		           std::nullptr_t> = nullptr>
		static expected_t from_code( Function &&func, Args &&... args ) {
			auto result = expected_t( );
			result.m_value = variant_from_code( std::forward<Function>( func ),
			                                    std::forward<Args>( args )... );
			return result;
		}

		void set_exception( std::exception_ptr ptr ) {
			m_value = ptr;
		}

		void set_exception( ) {
			set_exception( std::current_exception( ) );
		}

		template<typename Visitor>
		decltype( auto ) visit( Visitor &&visitor ) {
			static_assert(
			  traits::is_callable_v<Visitor, value_type &>,
			  "Visitor must be callable with the variants expected value_type &" );
			static_assert( traits::is_callable_v<Visitor, std::exception_ptr>,
			               "Visitor must be callable with std::exception_ptr" );
			return daw::visit_nt( m_value, std::forward<Visitor>( visitor ) );
		}

		template<typename Visitor>
		decltype( auto ) visit( Visitor &&visitor ) const {
			static_assert( traits::is_callable_v<Visitor, value_type const &>,
			               "Visitor must be callable with the variants expected "
			               "value_type const &" );
			static_assert( traits::is_callable_v<Visitor, std::exception_ptr>,
			               "Visitor must be callable with std::exception_ptr" );
			return daw::visit_nt( m_value, std::forward<Visitor>( visitor ) );
		}

		bool has_value( ) const {
			return daw::visit_nt(
			  m_value, []( value_type const & ) noexcept { return true; },
			  []( std::exception_ptr ) noexcept { return false; } );
		}

		bool has_exception( ) const {
			return daw::visit_nt(
			  m_value, []( value_type const & ) noexcept { return false; },
			  []( std::exception_ptr ptr ) noexcept { return ( ptr != nullptr ); } );
		}

		std::exception_ptr get_exception_ptr( ) {
			return std::get<std::exception_ptr>( m_value );
		}

		bool empty( ) const {
			return daw::visit_nt(
			  m_value, []( value_type const & ) noexcept { return false; },
			  []( std::exception_ptr ptr ) noexcept { return ( ptr == nullptr ); } );
		}

		explicit operator bool( ) const {
			return !empty( );
		}

		explicit operator value_type( ) const {
			return get( );
		}

		void throw_if_exception( ) const {
			daw::visit_nt( m_value, []( value_type const & ) noexcept {},
			               []( std::exception_ptr ptr ) {
				               if( ptr != nullptr ) {
					               std::rethrow_exception( ptr );
				               }
			               } );
		}

		reference get( ) {
			return daw::visit_nt(
			  m_value, []( reference value ) noexcept->reference { return value; },
			  []( std::exception_ptr ptr ) -> reference {
				  if( ptr != nullptr ) {
					  std::rethrow_exception( ptr );
				  }
				  daw::exception::daw_throw<std::logic_error>(
				    "Unexpected empty state" );
			  } );
		}

		const_reference get( ) const {
			return daw::visit_nt(
			  m_value,
			  []( const_reference value ) noexcept->const_reference { return value; },
			  []( std::exception_ptr ptr ) -> const_reference {
				  if( ptr != nullptr ) {
					  std::rethrow_exception( ptr );
				  }
				  daw::exception::daw_throw<std::logic_error>(
				    "Unexpected empty state" );
			  } );
		}

		reference operator*( ) {
			return get( );
		}

		const_reference operator*( ) const {
			return get( );
		}

		pointer operator->( ) {
			return daw::visit_nt( m_value, []( reference value ) noexcept->pointer {
				return std::addressof( value );
			},
			                      []( std::exception_ptr ptr ) -> pointer {
				                      std::rethrow_exception( ptr );
			                      } );
		}

		const_pointer operator->( ) const {
			return daw::visit_nt(
			  m_value, []( const_reference value ) noexcept->const_pointer {
				  return std::addressof( value );
			  },
			  []( std::exception_ptr ptr ) -> const_pointer {
				  if( ptr != nullptr ) {
					  std::rethrow_exception( ptr );
				  }
				  daw::exception::daw_throw<std::logic_error>(
				    "Unexpected empty state" );
			  } );
		}

		std::string get_exception_message( ) const {
			auto result = std::string( );
			try {
				throw_if_exception( );
			} catch( std::system_error const &e ) {
				result = e.code( ).message( ) + ": " + e.what( );
			} catch( std::exception const &e ) { result = e.what( ); } catch( ... ) {
			}
			return result;
		}
	}; // namespace daw

	static_assert( traits::is_regular<expected_t<int>> );

	namespace impl {
		struct void_value_t {
			constexpr void_value_t( ) noexcept = default;
		};
		constexpr bool operator==( void_value_t, void_value_t ) noexcept {
			return true;
		}
	} // namespace impl

	template<>
	struct expected_t<void> {
		using value_type = impl::void_value_t;

		struct exception_tag {};

	private:
		std::variant<std::exception_ptr, value_type> m_value =
		  std::exception_ptr{nullptr};

		expected_t( bool b )
		  : m_value( value_type{} ) {}

	public:
		//////////////////////////////////////////////////////////////////////////
		/// Summary: No value, aka null
		//////////////////////////////////////////////////////////////////////////
		expected_t( ) = default;
		~expected_t( ) = default;
		expected_t( expected_t const & ) = default;
		expected_t &operator=( expected_t const & ) = default;

		expected_t( expected_t &&other ) noexcept
		  : m_value( std::exchange( other.m_value, std::exception_ptr{nullptr} ) ) {
		}

		expected_t &operator=( expected_t &&rhs ) noexcept {
			m_value = std::exchange( rhs.m_value, std::exception_ptr{nullptr} );
			return *this;
		}

		friend bool operator==( expected_t const &lhs, expected_t const &rhs ) {
			return lhs.m_value == rhs.m_value;
		}

		//////////////////////////////////////////////////////////////////////////
		/// Summary: With value
		//////////////////////////////////////////////////////////////////////////
		expected_t &operator=( bool ) {
			m_value = daw::construct_a<value_type>{}( );
			return *this;
		}

		void clear( ) {
			m_value = std::exception_ptr{nullptr};
		}

		explicit expected_t( std::exception_ptr ptr )
		  : m_value( ptr ) {}

		expected_t &operator=( std::exception_ptr ptr ) {
			m_value = ptr;
			return *this;
		}

		template<typename ExceptionType>
		expected_t( exception_tag, ExceptionType &&ex )
		  : m_value(
		      std::make_exception_ptr( std::forward<ExceptionType>( ex ) ) ) {}

		explicit expected_t( exception_tag )
		  : m_value( std::current_exception( ) ) {}

	private:
		template<class Function, typename... Args>
		static std::variant<std::exception_ptr, value_type>
		variant_from_code( Function &&func, Args &&... args ) {
			try {
				func( std::forward<Args>( args )... );
				return impl::void_value_t( );
			} catch( ... ) { return std::current_exception( ); }
		}

	public:
		template<class Function, typename... Args,
		         std::enable_if_t<traits::is_callable_v<Function, Args...>,
		                          std::nullptr_t> = nullptr>
		explicit expected_t( Function &&func, Args &&... args )
		  : m_value( variant_from_code( std::forward<Function>( func ),
		                                std::forward<Args>( args )... ) ) {}

		template<class Function, typename... Args,
		         std::enable_if_t<traits::is_callable_v<Function, Args...>,
		                          std::nullptr_t> = nullptr>
		static expected_t from_code( Function &&func, Args &&... args ) {
			auto result = expected_t( );
			result.m_value = variant_from_code( std::forward<Function>( func ),
			                                    std::forward<Args>( args )... );
			return result;
		}

		void set_exception( std::exception_ptr ptr ) {
			m_value = ptr;
		}

		void set_exception( ) {
			set_exception( std::current_exception( ) );
		}

		template<typename Visitor>
		decltype( auto ) visit( Visitor &&visitor ) {
			static_assert( traits::is_callable_v<Visitor>,
			               "Visitor must be callable without arguments" );
			static_assert( traits::is_callable_v<Visitor, std::exception_ptr>,
			               "Visitor must be callable with std::exception_ptr" );
			return daw::visit_nt(
			  m_value,
			  [&]( value_type const & ) mutable noexcept( noexcept( visitor( ) ) ) {
				  return visitor( );
			  },
			  [&]( value_type && ) mutable noexcept( noexcept( visitor( ) ) ) {
				  return visitor( );
			  },
			  [&]( std::exception_ptr ptr ) mutable noexcept(
			    noexcept( visitor( ptr ) ) ) { return visitor( ptr ); } );
		}

		template<typename Visitor>
		decltype( auto ) visit( Visitor &&visitor ) const {
			static_assert( traits::is_callable_v<Visitor>,
			               "Visitor must be callable without arguments" );
			static_assert( traits::is_callable_v<Visitor, std::exception_ptr>,
			               "Visitor must be callable with std::exception_ptr" );
			return daw::visit_nt( m_value,
			                      [&]( value_type const & ) noexcept(
			                        noexcept( visitor( ) ) ) { return visitor( ); },
			                      [&]( std::exception_ptr ptr ) noexcept( noexcept(
			                        visitor( ptr ) ) ) { return visitor( ptr ); } );
		}

		bool has_value( ) const noexcept {
			return daw::visit_nt(
			  m_value, []( value_type const & ) noexcept { return true; },
			  []( std::exception_ptr ) noexcept { return false; } );
		}

		bool has_exception( ) const noexcept {
			return daw::visit_nt(
			  m_value, []( value_type const & ) noexcept { return false; },
			  []( std::exception_ptr ptr ) noexcept { return ( ptr != nullptr ); } );
		}

		std::exception_ptr get_exception_ptr( ) {
			return std::get<std::exception_ptr>( m_value );
		}

		bool empty( ) const noexcept {
			return daw::visit_nt(
			  m_value, []( value_type const & ) noexcept { return false; },
			  []( std::exception_ptr ptr ) noexcept { return ( ptr == nullptr ); } );
		}

		explicit operator bool( ) const noexcept {
			return !empty( );
		}

		void throw_if_exception( ) const {
			daw::visit_nt( m_value, []( value_type const & ) noexcept {},
			               []( std::exception_ptr ptr ) {
				               if( ptr != nullptr ) {
					               std::rethrow_exception( ptr );
				               }
			               } );
		}

		void get( ) const {
			daw::visit_nt( m_value, []( value_type const & ) noexcept {},
			               []( std::exception_ptr ptr ) {
				               if( ptr != nullptr ) {
					               std::rethrow_exception( ptr );
				               }
				               daw::exception::daw_throw<std::logic_error>(
				                 "Unexpected empty state" );
			               } );
		}

		std::string get_exception_message( ) const noexcept {
			std::string result{};
			try {
				throw_if_exception( );
			} catch( std::exception const &e ) { result = e.what( ); } catch( ... ) {
			}
			return result;
		}
	}; // class expected_t<void>

	template<typename Result, typename Function, typename... Args>
	expected_t<Result> expected_from_code( Function &&func, Args &&... args ) {
		static_assert(
		  traits::is_callable_convertible_v<Result, Function, Args...>,
		  "Must be able to convert result of func to expected result type" );

		auto result = expected_t<Result>( );
		result.from_code( std::forward<Function>( func ),
		                  std::forward<Args>( args )... );
		return result;
	}

	template<typename Function, typename... Args>
	auto expected_from_code( Function &&func, Args &&... args ) {
		using result_t =
		  std::decay_t<decltype( func( std::forward<Args>( args )... ) )>;

		return expected_t<result_t>::from_code( std::forward<Function>( func ),
		                                        std::forward<Args>( args )... );
	}

	template<typename Result>
	expected_t<Result> expected_from_exception( ) {
		return expected_t<Result>( std::current_exception( ) );
	}

	template<typename Result>
	expected_t<Result> expected_from_exception( std::exception_ptr ptr ) {
		return expected_t<Result>( ptr );
	}
} // namespace daw
