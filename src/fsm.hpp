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

    namespace traits {

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

    template<class Derived, class State = int>
    class fsm {
    public:
        using state_type = State;

    public:
        fsm(state_type init_state = state_type()) : state(init_state) {}

        template<class Event>
        void process_event(Event const& e) {
            static_assert(std::is_base_of<fsm, Derived>::value, "must derive from fsm");
            Derived* self = static_cast<Derived*>(this);
            state = Derived::transition_table::process(self, state, e);
        }

        state_type current_state() const { return state; }

    protected:
        template<class Event>
        state_type no_transition(state_type state, Event const&) {
            return state;
        }

    private:
        template<class Event, void (*action)(Derived&, Event const&)>
        struct make_fn_action {
            static constexpr bool valid = action != nullptr;

            using type = typename std::conditional<valid, make_fn_action, void>::type;

            void operator()(Derived& self, Event const& e) {
                action(self, e);
            }
        };

        template<class Event, bool (*guard)(Derived const&, Event const&)>
        struct make_fn_guard {
            static constexpr bool valid = true;  // FIXME: guard != nullptr (gcc)

            using type = typename std::conditional<valid, make_fn_guard, void>::type;

            bool operator()(Derived const& self, Event const& e) {
                return guard != nullptr ? guard(self, e) : true;
            }
        };

        template<class Event, void (Derived::*action)(Event const&)>
        struct make_mem_fn_action {
            static constexpr bool valid = action != nullptr;

            using type = typename std::conditional<valid, make_mem_fn_action, void>::type;

            void operator()(Derived& self, Event const& e) {
                (self.*action)(e);
            }
        };

        template<class Event, bool (Derived::*guard)(Event const&) const>
        struct make_mem_fn_guard {
            static constexpr bool valid = true;  // FIXME: guard != nullptr (gcc)

            using type = typename std::conditional<valid, make_mem_fn_guard, void>::type;

            bool operator()(Derived const& self, Event const& e) {
                return guard != nullptr ? (self.*guard)(e) : true;
            }
        };

    protected:
        template<class... Rows>
        struct table {
            template<class Event>
            static state_type process(Derived* self, state_type state, Event const& e) {
                using rows = typename filter_by_event_type<Event, Rows...>::type;
                return invoke<Event, rows>::execute(self, state, e);
            }
        };

        template<State Start, class Event, State Target>
        struct row_base {
            using state_type = State;
            using event_type = Event;

            static constexpr state_type start_value = Start;
            static constexpr state_type target_value = Target;

            static constexpr state_type process(Derived*, state_type, Event const&) {
                return target_value;
            }

            static constexpr bool check_guard(Derived const*, Event const&) {
                return true;
            }
        };

        template<
            State Start,
            class Event,
            State Target,
            class Action = void,
            class Guard = void
        >
        struct row;

        template<State Start, class Event, State Target>
        struct row<Start, Event, Target, void, void> : row_base<Start, Event, Target> {
        };

        template<State Start, class Event, State Target, class Action>
        struct row<Start, Event, Target, Action, void> : row_base<Start, Event, Target> {
            static state_type process(Derived* self, state_type, Event const& e) {
                Action()(*self, e);
                return Target;
            }
        };

        template<State Start, class Event, State Target, class Guard>
        struct row<Start, Event, Target, void, Guard> : row_base<Start, Event, Target> {
            static bool check_guard(Derived const* self, Event const& e) {
                return Guard()(*self, e);
            }
        };

        template<State Start, class Event, State Target, class Action, class Guard>
        struct row : row_base<Start, Event, Target> {
            static state_type process(Derived* self, state_type, Event const& e) {
                Action()(*self, e);
                return Target;
            }

            static bool check_guard(Derived const* self, Event const& e) {
                return Guard()(*self, e);
            }
        };

        template<
            State Start,
            class Event,
            State Target,
            void (*action)(Derived&, Event const&),
            bool (*guard)(Derived const&, Event const&) = nullptr
        >
        struct fn_row : row<
            Start, Event, Target,
            typename make_fn_action<Event, action>::type,
            typename make_fn_guard<Event, guard>::type
        > {};

        template<
            State Start,
            class Event,
            State Target,
            void (Derived::*action)(Event const&),
            bool (Derived::*guard)(Event const&) const = nullptr
        >
        struct mem_fn_row : row<
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
            using type = typename traits::filter<predicate, Types...>::type;
        };

        template<class Event, class...> struct invoke;

        template<class Event, class T, class... Types>
        struct invoke<Event, traits::list<T, Types...>> {
            static State execute(Derived* self, State state, Event const& event) {
                return state == T::start_value && T::check_guard(self, event)
                    ? T::process(self, state, event)
                    : invoke<Event, traits::list<Types...>>::execute(self, state, event);
            }
        };

        template<class Event>
        struct invoke<Event, traits::list<>> {
            static State execute(Derived* self, State state, Event const& e) {
                return self->no_transition(state, e);
            }
        };

    private:
        state_type state;
    };
}

#endif
