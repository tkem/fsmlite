/*
 * Copyright (c) 2015-2020 Thomas Kemmer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef FSMLITE_FSM_H
#define FSMLITE_FSM_H

#include <cstddef>
#include <type_traits>

#if !defined(NDEBUG) && (!__GNUC__ || __EXCEPTIONS)
#include <stdexcept>
#endif

namespace fsmlite {
    namespace detail {
#if __cplusplus >= 201703L
        template<class F, class... Args>
        using invoke_result_t = std::invoke_result_t<F, Args...>;

        template <class F, class... Args>
        using is_invocable = std::is_invocable<F, Args...>;
#elif __cplusplus >= 201103L
        template<class F, class... Args>
        using invoke_result_t = typename std::result_of<F&&(Args&&...)>::type;

        struct is_invocable_test {
            struct no_type { int a; int b; };

            template<class F, class... Args, class = invoke_result_t<F, Args...>>
            static char test(int);

            template<class, class...>
            static no_type test(...);
        };

        template<class F, class... Args>
            using is_invocable = typename std::integral_constant<
            bool,
            sizeof(is_invocable_test::test<F, Args...>(0)) == 1
        >::type;
#else
#error "fsmlite requires C++11 support."
#endif
        // C++11 std::forward() is in <utility>, which may not be
        // present on freestanding implementations
        template<class T>
        constexpr T&& forward(typename std::remove_reference<T>::type& t) noexcept {
            return static_cast<T&&>(t);
        }

        template<class T>
        constexpr T&& forward(typename std::remove_reference<T>::type&& t) noexcept {
            return static_cast<T&&>(t);
        }

        // C++17 std::invoke() is in <functional>, which may not be
        // present on freestanding implementations
        template <class F, class... Args>
        invoke_result_t<F, Args...> invoke(F&& f, Args&&... args) {
            return f(args...);
        }

        template <class M, class T, class T1, class... Args>
        invoke_result_t<M T::*, T1, Args...> invoke(M T::* f, T1&& obj, Args&&... args) {
            return (obj.*f)(args...);
        }

        // use any of F(), F(Arg1), F(Arg2), F(Arg1, Arg2)
        template<
            class F, class Arg1, class Arg2,
            bool f1 = is_invocable<F>::value,
            bool f2 = is_invocable<F, Arg1>::value,
            bool f3 = is_invocable<F, Arg2>::value,
            bool f4 = is_invocable<F, Arg1, Arg2>::value
        > struct binary_fn_helper;

        template<class F, class Arg1, class Arg2>
        struct binary_fn_helper<F, Arg1, Arg2, true, false, false, false> {
            using result_type = invoke_result_t<F>;

            static result_type invoke(F&& f, Arg1&&, Arg2&&) {
                return detail::invoke(f);
            }
        };

        template<class F, class Arg1, class Arg2>
        struct binary_fn_helper<F, Arg1, Arg2, false, true, false, false> {
            using result_type = invoke_result_t<F, Arg1>;

            static result_type invoke(F&& f, Arg1&& a, Arg2&&) {
                return detail::invoke(f, a);
            }
        };

        template<class F, class Arg1, class Arg2>
        struct binary_fn_helper<F, Arg1, Arg2, false, false, true, false> {
            using result_type = invoke_result_t<F, Arg2>;

            static result_type invoke(F&& f, Arg1&&, Arg2&& b) {
                return detail::invoke(f, b);
            }
        };

        template<class F, class Arg1, class Arg2>
        struct binary_fn_helper<F, Arg1, Arg2, false, false, false, true> {
            using result_type = invoke_result_t<F, Arg1, Arg2>;

            static result_type invoke(F&& f, Arg1&& a, Arg2&& b) {
                return detail::invoke(f, a, b);
            }
        };

        template<class F, class Arg1, class Arg2>
        using invoke_as_binary_fn_result_t = typename binary_fn_helper<F, Arg1, Arg2>::result_type;

        template<class F, class Arg1, class Arg2>
        invoke_as_binary_fn_result_t<F, Arg1, Arg2>invoke_as_binary_fn(F&& f, Arg1&& a, Arg2&& b) {
            return binary_fn_helper<F, Arg1, Arg2>::invoke(forward<F>(f), forward<Arg1>(a), forward<Arg2>(b));
        }

        // basic template metaprogramming stuff; note that we could
        // use std::tuple instead of list, but <tuple> may nor be
        // present on freestanding implementations
        template<class...> struct list {};

        template<class...> struct concat;

        template<class T, class... Types>
        struct concat<T, list<Types...>> {
            using type = list<T, Types...>;
        };

        template<>
        struct concat<> {
            using type = list<>;
        };

        template<template<typename> class Predicate, class...> struct filter;

        template<template<typename> class Predicate, class T, class... Types>
        struct filter<Predicate, T, Types...>
        {
            using type = typename std::conditional<
                Predicate<T>::value,
                typename concat<T, typename filter<Predicate, Types...>::type>::type,
                typename filter<Predicate, Types...>::type
            >::type;
        };

        template<template<typename> class Predicate>
        struct filter<Predicate> {
            using type = list<>;
        };
    }

    /**
     * Finite state machine (FSM) base class template.
     *
     * @tparam Derived the derived state machine class
     *
     * @tparam State the FSM's state type, defaults to `int`
     */
    template<class Derived, class State = int>
    class fsm {
    public:
        /**
         * The FSM's state type.
         */
        typedef State state_type;

    public:
        /**
         * Create a state machine with an optional initial state.
         *
         * @param init_state the FSM's initial state
         */
        fsm(state_type init_state = state_type()) : m_state(init_state) {}

        /**
         * Process an event.
         *
         * @warning This member function must not be called
         * recursively, e.g. from another `fsm` instance.
         *
         * @tparam Event the event type
         *
         * @param event the event instance
         *
         * @throw std::logic_error if a recursive invocation is
         * detected
         */
        template<class Event>
        void process_event(const Event& event) {
            using rows = typename by_event_type<Event, typename Derived::transition_table>::type;
            processing_lock lock(*this);
            static_assert(std::is_base_of<fsm, Derived>::value, "must derive from fsm");
            Derived& self = static_cast<Derived&>(*this);
            m_state = handle_event<Event, rows>::execute(self, event, m_state);
        }

        /**
         * Return the state machine's current state.
         */
        state_type current_state() const { return m_state; }

    protected:
        /**
         * Called when no transition can be found for the given event
         * in the current state.  Derived state machines may override
         * this to throw an exception, or change to some other (error)
         * state.  The default is to return the current state, so no
         * state change occurs.
         *
         * @tparam Event the event type
         *
         * @param event the event instance
         *
         * @return the FSM's new state
         */
        template<class Event>
        state_type no_transition(const Event& /*event*/) {
            return m_state;
        }

    private:
        template<State start, class Event, State target>
        struct row_base {
            using state_type = State;
            using event_type = Event;

            static constexpr state_type start_value() { return start; }
            static constexpr state_type target_value() { return target; }

        protected:
            template<class Action>
            static void process_event(Action&& action, Derived& self, const Event& event) {
                detail::invoke_as_binary_fn(action, self, event);
            }

            // clang++-5.0: constexpr function's return type 'void' is not a literal type
            static /*constexpr*/ void process_event(std::nullptr_t, Derived& /*self*/, const Event& /*event*/) {
            }

            template<class Guard>
            static bool check_guard(Guard&& guard, const Derived& self, const Event& event) {
                return detail::invoke_as_binary_fn(guard, self, event);
            }

            static constexpr bool check_guard(std::nullptr_t, const Derived&, const Event&) {
                return true;
            }
        };

    protected:
        /**
         * Transition table variadic class template.
         *
         * Each derived state machine class must define a nested
         * non-template type `transition_table` that's either derived
         * from or a type alias of `table`.
         */
        template<class... Rows> using table = detail::list<Rows...>;

        /**
         * Basic transition class template.
         *
         * @tparam start the start state of the transition
         *
         * @tparam Event the event type triggering the transition
         *
         * @tparam target the target state of the transition
         *
         * @tparam Action an action function type, or `std::nullptr_t`
         *
         * @tparam action a static `Action` instance
         *
         * @tparam Guard a guard function type, or `std::nullptr_t`
         *
         * @tparam guard a static `Guard` instance
         */
        template<
            State start,
            class Event,
            State target,
            class Action = std::nullptr_t,
            Action action = nullptr,
            class Guard = std::nullptr_t,
            Guard guard = nullptr
        >
        struct basic_row : public row_base<start, Event, target> {
            static void process_event(Derived& self, const Event& event) {
                row_base<start, Event, target>::process_event(action, self, event);
            }

            static bool check_guard(const Derived& self, const Event& event) {
                return row_base<start, Event, target>::check_guard(guard, self, event);
            }
        };

        /**
         * Member function transition class template.
         *
         * @tparam start the start state of the transition
         *
         * @tparam Event the event type triggering the transition
         *
         * @tparam target the target state of the transition
         *
         * @tparam action an action member function, or `nullptr`
         *
         * @tparam guard a guard member function, or `nullptr`
         */
        template<
            State start,
            class Event,
            State target,
            void (Derived::*action)(const Event&) = nullptr,
            bool (Derived::*guard)(const Event&) const = nullptr
        >
        struct mem_fn_row : public row_base<start, Event, target> {
            static void process_event(Derived& self, const Event& event) {
                if (action != nullptr) {
                    row_base<start, Event, target>::process_event(action, self, event);
                }
            }

            static bool check_guard(const Derived& self, const Event& event) {
                if (guard != nullptr) {
                    return row_base<start, Event, target>::check_guard(guard, self, event);
                } else {
                    return true;
                }
            }
        };

#if __cplusplus >= 201703L
        /**
         * Generic transition class template (requires C++17).
         *
         * @tparam start the start state of the transition
         *
         * @tparam Event the event type triggering the transition
         *
         * @tparam target the target state of the transition
         *
         * @tparam action a static action function pointer, or `nullptr`
         *
         * @tparam guard a static guard function pointer, or `nullptr`
         */
        template<
            State start,
            class Event,
            State target,
            auto action = nullptr,
            auto guard = nullptr
        >
        struct row : public row_base<start, Event, target> {
            static void process_event(Derived& self, const Event& event) {
                row_base<start, Event, target>::process_event(action, self, event);
            }

            static bool check_guard(const Derived& self, const Event& event) {
                return row_base<start, Event, target>::check_guard(guard, self, event);
            }
        };
#endif

    private:
        template<class Event, class...> struct by_event_type;

        template<class Event, class... Types>
        struct by_event_type<Event, detail::list<Types...>> {
            template<class T> using predicate = std::is_same<typename T::event_type, Event>;
            using type = typename detail::filter<predicate, Types...>::type;
        };

        template<class Event>
        struct by_event_type<Event, detail::list<>> {
            using type = detail::list<>;
        };

        template<class Event, class...> struct handle_event;

        template<class Event, class T, class... Types>
        struct handle_event<Event, detail::list<T, Types...>> {
            static State execute(Derived& self, const Event& event, State state) {
                return state == T::start_value() && T::check_guard(self, event) ?
                    T::process_event(self, event), T::target_value() :
                    handle_event<Event, detail::list<Types...>>::execute(self, event, state);
            }
        };

        template<class Event>
        struct handle_event<Event, detail::list<>> {
            static State execute(Derived& self, const Event& event, State) {
                return self.no_transition(event);
            }
        };

    private:
        state_type m_state;

    private:
#if !defined(NDEBUG) && (!__GNUC__ || __EXCEPTIONS)
        class processing_lock {
        public:
            processing_lock(fsm& m) : processing(m.processing) {
                if (processing) {
                    throw std::logic_error("process_event called recursively");
                }
                processing = true;
            }
            ~processing_lock() {
                processing = false;
            }
        private:
            bool& processing;
        };
        bool processing = false;
#else
        struct processing_lock {
            processing_lock(fsm&) {}
        };
#endif
    };
}

#endif
