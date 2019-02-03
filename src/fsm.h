/*
 * Copyright (c) 2015-2018 Thomas Kemmer
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

#include <type_traits>

#if !defined(NDEBUG) && (!__GNUC__ || __EXCEPTIONS)
#include <stdexcept>
#endif

namespace fsmlite {
    namespace detail {
#if __cplusplus >= 201703L
        template<bool B>
        using bool_constant = std::bool_constant<B>;

        template<class F, class... Args>
        using invoke_result_t = std::invoke_result<F, Args...>;

        template <class F, class... Args>
        using is_invocable = std::is_invocable<F, Args...>;
#else
        template<bool B>
        using bool_constant = std::integral_constant<bool, B>;

        template<class F, class... Args>
        using invoke_result_t = typename std::result_of<F(Args...)>::type;

        struct is_invocable_test {
            struct no_type { int a; int b; };

            template<class F, class... Args, class = invoke_result_t<F, Args...>>
            static char test(int);

            template<class, class...>
            static no_type test(...);
        };

        template<class F, class... Args>
        using is_invocable = typename std::conditional<
            std::is_function<F>::value,
            bool_constant<sizeof(is_invocable_test::test<F*, Args...>(0)) == 1>,
            bool_constant<sizeof(is_invocable_test::test<F, Args...>(0)) == 1>
        >::type;
#endif

        // similar to std::integral_constant, but without const
        template<class T>
        struct typed_value {
            using type = T;
            static T value;
        };

        template<class T>
        T typed_value<T>::value;

        // basic template metaprogramming stuff; note that we could
        // use std::tuple instead of list, but std::tuple is not
        // required to be available on freestanding implementations
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

        // use any of fn(), fn(arg1), fn(arg2), fn(arg1, arg2)
        template<
            class T, T* fn, class Arg1, class Arg2,
            bool f1 = is_invocable<T>::value,
            bool f2 = is_invocable<T, Arg1>::value,
            bool f3 = is_invocable<T, Arg2>::value,
            bool f4 = is_invocable<T, Arg1, Arg2>::value
        > struct make_binary_function;

        template<class T, T* fn, class Arg1, class Arg2>
        struct make_binary_function<T, fn, Arg1, Arg2, true, false, false, false> {
            static struct type {
                invoke_result_t<T> operator()(Arg1, Arg2) const {
                    return (*fn)();
                }
            } value;
        };

        template<class T, T* fn, class Arg1, class Arg2>
        typename make_binary_function<T, fn, Arg1, Arg2, true, false, false, false>::type
        make_binary_function<T, fn, Arg1, Arg2, true, false, false, false>::value;

        template<class T, T* fn, class Arg1, class Arg2>
        struct make_binary_function<T, fn, Arg1, Arg2, false, true, false, false> {
            static struct type {
                invoke_result_t<T, Arg1> operator()(Arg1 arg1, Arg2) const {
                    return (*fn)(arg1);
                }
            } value;
        };

        template<class T, T* fn, class Arg1, class Arg2>
        typename make_binary_function<T, fn, Arg1, Arg2, false, true, false, false>::type
        make_binary_function<T, fn, Arg1, Arg2, false, true, false, false>::value;

        template<class T, T* fn, class Arg1, class Arg2>
        struct make_binary_function<T, fn, Arg1, Arg2, false, false, true, false> {
            static struct type {
                invoke_result_t<T, Arg2> operator()(Arg1, Arg2 arg2) const {
                    return (*fn)(arg2);
                }
            } value;
        };

        template<class T, T* fn, class Arg1, class Arg2>
        typename make_binary_function<T, fn, Arg1, Arg2, false, false, true, false>::type
        make_binary_function<T, fn, Arg1, Arg2, false, false, true, false>::value;

        template<class T, T* fn, class Arg1, class Arg2>
        struct make_binary_function<T, fn, Arg1, Arg2, false, false, false, true> {
            static struct type {
                invoke_result_t<T, Arg1, Arg2> operator()(Arg1 arg1, Arg2 arg2) const {
                    return (*fn)(arg1, arg2);
                }
            } value;
        };

        template<class T, T* fn, class Arg1, class Arg2>
        typename make_binary_function<T, fn, Arg1, Arg2, false, false, false, true>::type
        make_binary_function<T, fn, Arg1, Arg2, false, false, false, true>::value;
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
        state_type no_transition(const Event& event) {
            return m_state;
        }

    private:
        template<class Event>
        struct no_action_type {
            void operator()(Derived&, const Event&) const {}
        };

        template<class Event>
        struct no_guard_type {
            bool operator()(const Derived&, const Event&) const {
                return true;
            }
        };

        template<class Event, void (Derived::*action)(const Event&)>
        struct mem_fn_action_type {
            void operator()(Derived& self, const Event& event) const {
                (self.*action)(event);
            }
        };

        template<class Event, bool (Derived::*guard)(const Event&) const>
        struct mem_fn_guard_type {
            bool operator()(const Derived& self, const Event& event) const {
                return (self.*guard)(event);
            }
        };

        template<class Event>
        struct no_action: detail::typed_value<no_action_type<Event>> {};

        template<class Event>
        struct no_guard: detail::typed_value<no_guard_type<Event>> {};

        template<class Event, void (Derived::*action)(const Event&)>
        struct mem_fn_action: detail::typed_value<mem_fn_action_type<Event, action>> {};

        template<class Event, bool (Derived::*guard)(const Event&) const>
        struct mem_fn_guard: detail::typed_value<mem_fn_guard_type<Event, guard>> {};

        template<class Event, class T, T* fn>
        using make_action = typename std::conditional<
            !std::is_void<T>::value,
            detail::make_binary_function<T, fn, Derived&, const Event&>,
            no_action<Event>
        >::type;

        template<class Event, class T, T* fn>
        using make_guard = typename std::conditional<
            !std::is_void<T>::value,
            detail::make_binary_function<T, fn, const Derived&, const Event&>,
            no_guard<Event>
        >::type;

        template<class Event, void (Derived::*action)(const Event&)>
        struct make_mem_fn_action_type {
            void operator()(Derived& self, const Event& event) {
                if (action != nullptr) (self.*action)(event);
            }
        };

        template<class Event, void (Derived::*action)(const Event&)>
        struct make_mem_fn_action: detail::typed_value<make_mem_fn_action_type<Event, action>> {};

        template<class Event, bool (Derived::*guard)(const Event&) const>
        struct make_mem_fn_guard_type {
            bool operator()(const Derived& self, const Event& event) const {
                return guard != nullptr ? (self.*guard)(event) : true;
            }
        };

        template<class Event, bool (Derived::*guard)(const Event&) const>
        struct make_mem_fn_guard: detail::typed_value<make_mem_fn_guard_type<Event, guard>> {};

        template<
            State start,
            class Event,
            State target,
            class Action,
            Action* action,
            class Guard,
            Guard* guard
        >
        struct basic_row {
            using state_type = State;
            using event_type = Event;

            static constexpr state_type start_value() { return start; }
            static constexpr state_type target_value() { return target; }

            static void process_event(Derived& self, const Event& event) {
                (*action)(self, event);
            }

            static bool check_guard(const Derived& self, const Event& e) {
                return (*guard)(self, e);
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
         * Generic transition class template.
         *
         * @tparam start the start state of the transition
         *
         * @tparam Event the event type triggering the transition
         *
         * @tparam target the target state of the transition
         *
         * @tparam Action an action functor type, or `void`
         *
         * @tparam action a pointer to an `Action` instance
         *
         * @tparam Guard a guard functor type, or `void`
         *
         * @tparam guard a pointer to a `Guard` instance
         */
        template<
            State start,
            class Event,
            State target,
            class Action = void,
            Action* action = nullptr,
            class Guard = void,
            Guard* guard = nullptr
        >
        struct row: basic_row<
            start, Event, target,
            typename make_action<Event, Action, action>::type,
            &make_action<Event, Action, action>::value,
            typename make_guard<Event, Guard, guard>::type,
            &make_guard<Event, Guard, guard>::value
        > {};

        /**
         * Member function transition class template.
         *
         * @tparam start the start state of the transition
         *
         * @tparam Event the event type triggering the transition
         *
         * @tparam target the target state of the transition
         *
         * @tparam action an action member function
         *
         * @tparam guard a guard member function, or `nullptr`
         */
        template<
            State start,
            class Event,
            State target,
            void (Derived::*action)(const Event&),
            bool (Derived::*guard)(const Event&) const = nullptr
        >
        struct mem_fn_row: basic_row<
            start, Event, target,
            typename make_mem_fn_action<Event, action>::type,
            &make_mem_fn_action<Event, action>::value,
            typename make_mem_fn_guard<Event, guard>::type,
            &make_mem_fn_guard<Event, guard>::value
        > {};

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
