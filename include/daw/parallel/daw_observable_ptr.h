// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Darrell Wright
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

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

#include "../daw_expected.h"
#include "../daw_traits.h"

namespace daw {
	template<typename T>
	class observable_ptr;

	namespace impl {
		template<typename T>
		class locked_ptr {
			T *m_ptr;
			std::function<void( )> m_on_exit;

		public:
			locked_ptr( ) = delete;
			locked_ptr( locked_ptr const & ) = delete;
			locked_ptr &operator=( locked_ptr const & ) = delete;

			locked_ptr( locked_ptr && ) noexcept = default;
			locked_ptr &operator=( locked_ptr && ) noexcept = default;

			template<typename OnExit>
			locked_ptr( T *ptr, OnExit &&on_exit ) noexcept
			  : m_ptr( ptr )
			  , m_on_exit( std::forward<OnExit>( on_exit ) ) {}

			~locked_ptr( ) noexcept {
				m_ptr = nullptr;
				try {
					if( m_on_exit ) {
						m_on_exit( );
					}
				} catch( ... ) {}
			}

			T *operator->( ) const noexcept {
				return m_ptr;
			}

			T &operator*( ) noexcept {
				return *m_ptr;
			}

			T const &operator*( ) const noexcept {
				return *m_ptr;
			}

			T *get( ) const noexcept {
				return m_ptr;
			}

			explicit operator bool( ) const noexcept {
				return m_ptr != nullptr;
			}
		};

		template<typename T>
		class control_block_t {
			using observer_count_t = int16_t;

			// Pointer we are guarding
			T *m_ptr;

			// The owner has gone out of scope, as soon as there are no borrows, run
			// values destructor
			std::atomic_bool m_ptr_destruct;

			// This controls access to deleting the control block
			std::once_flag m_cb_destruct;

			// Pointer has been borrowed
			mutable std::mutex m_is_borrowed;

			// Number of observers alive, cannot delete control block unless this is 0
			std::atomic<observer_count_t> m_observer_count;

			void destruct_if_should( std::lock_guard<std::mutex> const & ) {
				if( m_ptr_destruct.load( ) ) {
					auto tmp = std::exchange( m_ptr, nullptr );
					if( tmp ) {
						delete tmp;
					}
				}
			}

			void destruct_if_should( ) {
				std::lock_guard<std::mutex> lck{m_is_borrowed};
				destruct_if_should( lck );
			}
			constexpr control_block_t( T *ptr )
			  : m_ptr( ptr )
			  , m_ptr_destruct( false )
			  , m_cb_destruct( )
			  , m_is_borrowed( )
			  , m_observer_count( 0 ) {}

			friend class observable_ptr<T>;

		public:
			control_block_t( ) = delete;
			control_block_t( control_block_t const & ) = delete;
			control_block_t( control_block_t && ) noexcept = delete;
			control_block_t &operator=( control_block_t const & ) = delete;
			control_block_t &operator=( control_block_t && ) noexcept = delete;

			// Should never get to this point without point without owner/observer
			// cleaning up ptr
			//~control_block_t( ) noexcept = default;

			bool expired( ) const noexcept {
				return m_ptr_destruct;
			}

			locked_ptr<T> try_borrow( ) const {
				if( !m_is_borrowed.try_lock( ) ) {
					return locked_ptr<T>( nullptr, []( ) {} );
				}
				return locked_ptr<T>{m_ptr, [&]( ) { m_is_borrowed.unlock( ); }};
			}

			locked_ptr<T> borrow( ) const {
				m_is_borrowed.lock( );
				return locked_ptr<T>( m_ptr, [&]( ) { m_is_borrowed.unlock( ); } );
			}

			locked_ptr<T> borrow( ) {
				m_is_borrowed.lock( );
				return locked_ptr<T>( m_ptr, [&]( ) { m_is_borrowed.unlock( ); } );
			}

			constexpr T *get( ) const noexcept {
				return m_ptr;
			}

			bool add_observer( ) {
				if( m_ptr_destruct.load( ) ) {
					return false;
				}
				++m_observer_count;
				return true;
			}

		private:
			bool remove_observer( std::lock_guard<std::mutex> &lck ) {
				--m_observer_count;
				destruct_if_should( lck );
				return m_observer_count <= 0;
			}

			bool remove_owner( std::lock_guard<std::mutex> &lck ) {
				m_ptr_destruct = true;
				destruct_if_should( lck );
				return m_observer_count <= 0;
			}

			static void destruct_cb( control_block_t *cb ) {
				if( !cb ) {
					return;
				}
				bool do_destruct = false;
				std::call_once( cb->m_cb_destruct,
				                [&do_destruct]( ) { do_destruct = true; } );
				if( do_destruct ) {
					delete cb;
				}
			}

		public:
			static void remove_observer( control_block_t *cb ) {
				if( !cb ) {
					return;
				}
				bool destruction = false;
				{
					std::lock_guard<std::mutex> lck{cb->m_is_borrowed};
					destruction = cb->remove_observer( lck );
				}
				if( destruction ) {
					destruct_cb( cb );
				}
			}

			static void remove_owner( control_block_t *cb ) {
				if( !cb ) {
					return;
				}
				bool destruction = false;
				{
					std::lock_guard<std::mutex> lck{cb->m_is_borrowed};
					destruction = cb->remove_owner( lck );
				}
				if( destruction ) {
					destruct_cb( cb );
				}
			}
		};
	} // namespace impl

	template<typename T>
	class observer_ptr {
		impl::control_block_t<T> *m_control_block;

	public:
		constexpr observer_ptr( ) noexcept
		  : m_control_block( nullptr ) {}

		constexpr observer_ptr( std::nullptr_t ) noexcept
		  : m_control_block( nullptr ) {}

		/// @brief An observer of an observable_ptr
		/// @param cb Control block for observable pointer.  Must never be null,
		/// will abort if so
		observer_ptr( impl::control_block_t<T> *cb )
		  : m_control_block( cb ) {}

		void reset( ) noexcept {
			auto tmp = std::exchange( m_control_block, nullptr );
			if( tmp ) {
				try {
					impl::control_block_t<T>::remove_observer( tmp );
				} catch( ... ) {}
			}
		}

		~observer_ptr( ) noexcept {
			reset( );
		}

		constexpr observer_ptr( observer_ptr const &other )
		  : m_control_block( other.m_control_block ) {

			m_control_block->add_observer( );
		}

		observer_ptr &operator=( observer_ptr const &rhs ) {
			if( this != &rhs ) {
				m_control_block = rhs.m_control_block;
				m_control_block->add_observer( );
			}
			return *this;
		}

		constexpr observer_ptr( observer_ptr &&other ) noexcept
		  : m_control_block( std::exchange( other.m_control_block, nullptr ) ) {}

		constexpr observer_ptr &operator=( observer_ptr &&rhs ) noexcept {
			if( this != &rhs ) {
				m_control_block = std::exchange( rhs.m_control_block, nullptr );
			}
			return *this;
		}

		constexpr T *get( ) const noexcept {
			if( !m_control_block ) {
				return nullptr;
			}
			return m_control_block->get( );
		}

		impl::locked_ptr<T> try_borrow( ) const {
			if( !m_control_block ) {
				return impl::locked_ptr<T>( nullptr, []( ) {} );
			}
			return m_control_block->try_borrow( );
		}

		impl::locked_ptr<T> borrow( ) const {
			if( !m_control_block ) {
				return impl::locked_ptr<T>( nullptr, []( ) {} );
			}
			return m_control_block->borrow( );
		}

		template<typename Callable,
		         std::enable_if_t<daw::is_callable_v<Callable, T const &>,
		                          std::nullptr_t> = nullptr>
		decltype( auto ) lock( Callable &&c ) const
		  noexcept( noexcept( c( std::declval<T const &>( ) ) ) ) {

			auto lck_ptr = borrow( );
			using result_t = std::decay_t<decltype( c( *lck_ptr ) )>;

			if( !lck_ptr ) {
				return daw::expected_t<result_t>{};
			}
			return daw::expected_t<result_t>::from_code( std::forward<Callable>( c ),
			                                             *lck_ptr );
		}

		template<typename Callable,
		         std::enable_if_t<daw::is_callable_v<Callable, T &>,
		                          std::nullptr_t> = nullptr>
		decltype( auto )
		lock( Callable &&c ) noexcept( noexcept( c( std::declval<T &>( ) ) ) ) {

			auto lck_ptr = borrow( );
			using result_t = std::decay_t<decltype( c( *lck_ptr ) )>;

			if( !lck_ptr ) {
				return daw::expected_t<result_t>{};
			}
			return daw::expected_t<result_t>::from_code( std::forward<Callable>( c ),
			                                             *lck_ptr );
		}

		decltype( auto ) operator-> ( ) const noexcept {
			return borrow( );
		}

		T const &operator*( ) const {
			T const &r = *borrow( );
			return r;
		}

		T &operator*( ) {
			T &r = *borrow( );
			return r;
		}

		explicit operator bool( ) const {
			return m_control_block != nullptr && !m_control_block->expired( );
		}

		observer_ptr<T> get_observer( ) const {
			return observer_ptr<T>{m_control_block};
		}
	};

	/// @brief A pointer wrapper that allows others to temporarily postpone
	/// destruction while in a locked scope
	/// @tparam T Type to construct/hold
	template<typename T>
	class observable_ptr {
		impl::control_block_t<T> *m_control_block;

	public:
		observable_ptr( observable_ptr const & ) = delete;
		observable_ptr &operator=( observable_ptr const & ) = delete;

		observable_ptr( observable_ptr &&other ) noexcept
		  : m_control_block( std::exchange( other.m_control_block, nullptr ) ) {}

		observable_ptr &operator=( observable_ptr &&rhs ) noexcept {
			if( this != &rhs ) {
				m_control_block = std::exchange( rhs.m_control_block, nullptr );
			}
			return *this;
		}

		~observable_ptr( ) noexcept {
			auto tmp = std::exchange( m_control_block, nullptr );
			try {
				if( tmp ) {
					impl::control_block_t<T>::remove_owner( tmp );
				}
			} catch( ... ) {}
		}

		/// @brief Take ownership of pointer and construct shared_ptr with it
		/// @param value pointer to take ownershiip of
		explicit observable_ptr( T *value )
		  : m_control_block( new impl::control_block_t<T>( value ) ) {}

		constexpr observable_ptr( ) noexcept
		  : m_control_block( nullptr ) {}

		constexpr observable_ptr( std::nullptr_t ) noexcept
		  : m_control_block( nullptr ) {}

		observer_ptr<T> get_observer( ) const {
			return observer_ptr<T>( m_control_block );
		}

		constexpr T *get( ) const noexcept {
			if( !m_control_block ) {
				return nullptr;
			}
			return m_control_block->get( );
		}

		decltype( auto ) operator-> ( ) const {
			return borrow( );
		}

		impl::locked_ptr<T> try_borrow( ) const {
			if( !m_control_block ) {
				return impl::locked_ptr<T>( nullptr, []( ) {} );
			}
			return m_control_block->try_borrow( );
		}

		impl::locked_ptr<T> borrow( ) const {
			if( !m_control_block ) {
				return impl::locked_ptr<T>( nullptr, []( ) {} );
			}
			return m_control_block->borrow( );
		}

		template<typename Callable>
		decltype( auto ) lock( Callable &&c ) const {
			using result_t =
			  std::decay_t<decltype( c( std::declval<T const &>( ) ) )>;
			auto lck_ptr = borrow( );
			if( !lck_ptr ) {
				return daw::expected_t<result_t>{};
			}
			T const &r = *lck_ptr;
			return daw::expected_t<result_t>::from_code( std::forward<Callable>( c ),
			                                             r );
		}

		template<typename Callable>
		decltype( auto ) lock( Callable &&c ) {
			using result_t = std::decay_t<decltype(
			  std::declval<Callable>( )( std::declval<T &>( ) ) )>;
			auto lck_ptr = borrow( );
			if( !lck_ptr ) {
				return daw::expected_t<result_t>{};
			}
			T &r = *lck_ptr;
			return daw::expected_t<result_t>::from_code( std::forward<Callable>( c ),
			                                             r );
		}

		T const &operator*( ) const {
			T const &r = *borrow( );
			return r;
		}

		T &operator*( ) {
			T &r = *borrow( );
			return r;
		}

		explicit operator bool( ) const {
			return m_control_block != nullptr && !m_control_block->expired( );
		}

		explicit operator observer_ptr<T>( ) const {
			return observer_ptr<T>( m_control_block );
		}
	};

	template<typename T, typename... Args>
	observable_ptr<T> make_observable_ptr( Args &&... args ) noexcept(
	  noexcept( new T( std::forward<Args>( args )... ) ) ) {
		T *tmp = new T( std::forward<Args>( args )... );
		if( !tmp ) {
			return observable_ptr<T>( );
		}
		return observable_ptr<T>( tmp );
	}
} // namespace daw

