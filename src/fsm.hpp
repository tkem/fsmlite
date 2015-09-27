/*
 * Copyright (c) 2015 Thomas Kemmer
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

#ifndef FSMLITE_FSM_HPP
#define FSMLITE_FSM_HPP

#include <type_traits>

namespace fsmlite {

    namespace detail {

        /* std::declval is in <utility>, which may not be available
           with freestanding implementations */
        template <class T>
        typename std::add_rvalue_reference<T>::type declval();

        /* from C++17 */
        template <bool B>
        using bool_constant = std::integral_constant<bool, B>;

        template<class...> struct list {
            using type = list;
        };

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

        struct is_callable_test {
            struct false_type { int a; int b; };

            template<class T, class... Args,
                     class = typename std::result_of<T(Args...)>::type>
            static char test(int);

            template<class, class...>
            static false_type test(...);
        };

        template<class T, class... Args>
        struct is_callable: public bool_constant<
            sizeof(is_callable_test::test<T, Args...>(0)) == 1
        >{};
    }

    /**
     * Finite state machine (FSM) base class template.
     *
     * @tparam Derived the derived state machine class
     *
     * @tparam State the state type to use, defaults to `int`
     */
    template<class Derived, class State = int>
    class fsm {
    public:
        /**
         * The state machine's state type.
         */
        typedef State state_type;

    public:
        /**
         * Create a state machine with a default initial state.
         */
        fsm() : m_state() {}

        /**
         * Create a state machine with a custom initial state.
         *
         * @param init_state the state machine's initial state
         */
        fsm(state_type init_state) : m_state(init_state) {}

        /**
         * Process an event.
         *
         * @tparam Event the event tyoe
         *
         * @param event an event instance
         */
        template<class Event>
        void process_event(Event const& event) {
            static_assert(std::is_base_of<fsm, Derived>::value, "must derive from fsm");
            Derived* self = static_cast<Derived*>(this);
            m_state = Derived::transition_table::execute(self, event, m_state);
        }

        /**
         * Return the state machine's current state.
         */
        state_type current_state() const { return m_state; }

    protected:
        /**
         * Called when no transition can be found for the current
         * state with the given event.
         *
         * @tparam Event the event type
         *
         * @param event an event instance
         */
        template<class Event>
        state_type no_transition(Event const& event) {
            return m_state;
        }

    private:
        template<State Start, class Event, State Target>
        struct row_base {
            using state_type = State;
            using event_type = Event;

            static constexpr state_type start_value = Start;
            static constexpr state_type target_value = Target;

            static void process_event(Derived*, Event const&) {}

            static bool check_guard(Derived const*, Event const&) {
                return true;
            }
        };

        template<State Start, class Event, State Target, class Action = void, class Guard = void>
        struct basic_row;

        template<State Start, class Event, State Target>
        struct basic_row<Start, Event, Target, void, void>: row_base<Start, Event, Target> {};

        template<State Start, class Event, State Target, class Action>
        struct basic_row<Start, Event, Target, Action, void>: row_base<Start, Event, Target> {
            static void process_event(Derived* self, Event const& event) {
                Action()(*self, event);
            }
        };

        template<State Start, class Event, State Target, class Guard>
        struct basic_row<Start, Event, Target, void, Guard>: row_base<Start, Event, Target> {
            static bool check_guard(Derived const* self, Event const& e) {
                return Guard()(*self, e);
            }
        };

        template<State Start, class Event, State Target, class Action, class Guard>
        struct basic_row: public row_base<Start, Event, Target> {
            static void process_event(Derived* self, Event const& event) {
                Action()(*self, event);
            }

            static bool check_guard(Derived const* self, Event const& e) {
                return Guard()(*self, e);
            }
        };

        template<class Action, class Event>
        struct make_action {
            struct action1 {
                void operator()(Derived& self, Event const& event) const {
                    Action()(self, event);
                }
            };

            struct action2 {
                void operator()(Derived& self, Event const&) const {
                    Action()(self);
                }
            };

            struct action3 {
                void operator()(Derived&, Event const& event) const {
                    Action()(event);
                }
            };

            struct action4 {
                void operator()(Derived&, Event const&) const {
                    Action()();
                }
            };

            using type = typename std::conditional<
                detail::is_callable<Action, Derived&, Event const&>::value,
                action1,
                typename std::conditional<
                    detail::is_callable<Action, Derived&>::value,
                    action2,
                    typename std::conditional<
                        detail::is_callable<Action, Event&>::value,
                        action3,
                        action4  // TODO: error?
                    >::type
                >::type
            >::type;
        };

        template<class Event>
        struct make_action<void, Event> {
            using type = void;
        };

        template<class Guard, class Event>
        struct make_guard {
            struct guard1 {
                bool operator()(Derived const& self, Event const& event) const {
                    return Guard()(self, event);
                }
            };

            struct guard2 {
                bool operator()(Derived const& self, Event const&) const {
                    return Guard()(self);
                }
            };

            struct guard3 {
                bool operator()(Derived const&, Event const& event) const {
                    return Guard()(event);
                }
            };

            struct guard4 {
                bool operator()(Derived const&, Event const&) const {
                    return Guard()();
                }
            };

            using type = typename std::conditional<
                detail::is_callable<Guard, Derived const&, Event const&>::value,
                guard1,
                typename std::conditional<
                    detail::is_callable<Guard, Derived const&>::value,
                    guard2,
                    typename std::conditional<
                        detail::is_callable<Guard, Event const&>::value,
                        guard3,
                        guard4  // TODO: error?
                    >::type
                >::type
            >::type;
        };

        template<class Event>
        struct make_guard<void, Event> {
            using type = void;
        };

        template<class Event, void (*action)(Derived&, Event const&)>
        struct make_fn_action {
            struct fn_action {
                void operator()(Derived& self, Event const& event) const {
                    action(self, event);
                }
            };

            using type = typename std::conditional<action != nullptr, fn_action, void>::type;
        };

        template<class Event, bool (*guard)(Derived const&, Event const&)>
        struct make_fn_guard {
            struct mem_fn_guard {
                bool operator()(Derived const& self, Event const& event) const {
                    return guard != nullptr ? guard(self, event) : true;
                }
            };

            // g++ 4.8 error: ‘(foo::bar != 0u)’ is not a constant expression
            //using type = typename std::conditional<guard != nullptr, mem_fn_guard, void>::type;
            using type = mem_fn_guard;
        };

        template<class Event, void (Derived::*action)(Event const&)>
        struct make_mem_fn_action {
            struct mem_fn_action {
                void operator()(Derived& self, Event const& event) const {
                    (self.*action)(event);
                }
            };

            using type = typename std::conditional<action != nullptr, mem_fn_action, void>::type;
        };

        template<class Event, bool (Derived::*guard)(Event const&) const>
        struct make_mem_fn_guard {
            struct mem_fn_guard {
                bool operator()(Derived const& self, Event const& event) const {
                    return guard != nullptr ? (self.*guard)(event) : true;
                }
            };

            // g++ 4.8 error: ‘(foo::bar != 0u)’ is not a constant expression
            //using type = typename std::conditional<guard != nullptr, mem_fn_guard, void>::type;
            using type = mem_fn_guard;
        };

    protected:
        /**
         * Transition table base class template.
         *
         * Every derived state machine class must define a
         * non-template class `transition_table` derived from `table`.
         */
        template<class... Rows>
        struct table {
            template<class Event>
            static state_type execute(Derived* self, Event const& event, State state) {
                using rows = typename filter_by_event_type<Event, Rows...>::type;
                return invoke<Event, rows>::execute(self, event, state);
            }
        };

        /**
         * Generic functor-based transition class template.
         *
         * @tparam Start the start state of the transition
         *
         * @tparam Event the event type triggering the transition
         *
         * @tparam Target the target state of the transition
         *
         * @tparam Action an action functor type, or `void` for no action
         *
         * @tparam Guard a guard functor type, or `void` for no guard
         */
        template<
            State Start,
            class Event,
            State Target,
            class Action = void,
            class Guard = void
        >
        struct row: basic_row<
            Start, Event, Target,
            typename make_action<Action, Event>::type,
            typename make_guard<Guard, Event>::type
            > {};

        /**
         * Function-based transition class template.
         *
         * @tparam Start the start state of the transition
         *
         * @tparam Event the event type triggering the transition
         *
         * @tparam Target the target state of the transition
         *
         * @tparam action an action function
         *
         * @tparam guard a guard function, or `nullptr` for no guard
         */
        template<
            State Start,
            class Event,
            State Target,
            void (*action)(Derived&, Event const&),
            bool (*guard)(Derived const&, Event const&) = nullptr
        >
        struct fn_row: basic_row<
            Start, Event, Target,
            typename make_fn_action<Event, action>::type,
            typename make_fn_guard<Event, guard>::type
        > {};

        /**
         * Member function-based transition class template.
         *
         * @tparam Start the start state of the transition
         *
         * @tparam Event the event type triggering the transition
         *
         * @tparam Target the target state of the transition
         *
         * @tparam action an action member function
         *
         * @tparam guard a guard member function, or `nullptr` for no guard
         */
        template<
            State Start,
            class Event,
            State Target,
            void (Derived::*action)(Event const&),
            bool (Derived::*guard)(Event const&) const = nullptr
        >
        struct mem_fn_row: basic_row<
            Start, Event, Target,
            typename make_mem_fn_action<Event, action>::type,
            typename make_mem_fn_guard<Event, guard>::type
        > {};

    private:
        template<class T, class Event>
        struct has_event_type : std::is_same<typename T::event_type, Event> {};

        template<class Event, class... Types>
        struct filter_by_event_type {
            template <class T> using predicate = has_event_type<T, Event>;
            using type = typename detail::filter<predicate, Types...>::type;
        };

        template<class Event, class...> struct invoke;

        template<class Event, class T, class... Types>
        struct invoke<Event, detail::list<T, Types...>> {
            static State execute(Derived* self, Event const& event, State state) {
                return state == T::start_value && T::check_guard(self, event) ?
                    T::process_event(self, event), T::target_value :
                    invoke<Event, detail::list<Types...>>::execute(self, event, state);
            }
        };

        template<class Event>
        struct invoke<Event, detail::list<>> {
            static State execute(Derived* self, Event const& event, State) {
                return self->no_transition(event);
            }
        };

    private:
        state_type m_state;
    };
}

#endif
