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

    namespace mpl {

        template<class...> struct list {};

        template<class...> struct concat;

        template<class T, class... Args>
        struct concat<T, list<Args...>>
        {
            using type = list<T, Args...>;
        };

        template<>
        struct concat<> {
            using type = list<>;
        };

        template<template<typename> class Predicate, class...> struct filter;

        template<template<typename> class Predicate, class Head, class... Tail>
        struct filter<Predicate, Head, Tail...>
        {
            using type = typename std::conditional<
                Predicate<Head>::value,
                typename concat<Head, typename filter<Predicate, Tail...>::type>::type,
                typename filter<Predicate, Tail...>::type
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
        fsm() : state() {}
        fsm(state_type init_state) : state(init_state) {}

        template<class Event>
        void process_event(Event const& e) {
            static_assert(std::is_base_of<fsm, Derived>::value, "must derive from fsm");
            Derived* self = static_cast<Derived*>(this);
            state = Derived::transition_table::process(self, state, e);
        }

        state_type current_state() const { return state; }

    protected:
        using fsm_type = fsm;

        template<class Event>
        state_type no_transition(state_type state, Event const&) {
            return state;
        }

    protected:
        template<class... Args>
        struct table {
            template<class Event>
            static state_type process(Derived* self, state_type state, Event const& e) {
                using args = typename filter_by_event_type<Event, Args...>::type;
                return invoke<Event, args>::execute(self, state, e);
            }
        };

        template<
            State start,
            class Event,
            State target,
            void (Derived::*action)(Event const&) = nullptr,
            bool (Derived::*guard)(Event const&) = nullptr
        >
        struct row {
            using state_type = State;
            using event_type = Event;

            static constexpr state_type start_value = start;
            static constexpr state_type target_value = target;

            static state_type process(Derived* self, state_type state, Event const& e) {
                if (action) {
                    (self->*action)(e);
                }
                return target_value;
            }

            static bool check(Derived* self, Event const& e) {
                return guard ? (self->*guard)(e) : true;
            }
        };

    private:
        template<class T, class Event>
        struct has_event_type : std::is_same<typename T::event_type, Event> {};

        template<class Event, class... Args>
        struct filter_by_event_type {
            template <class T> using predicate = has_event_type<T, Event>;
            using type = typename mpl::filter<predicate, Args...>::type;
        };

        template<class Event, class...> struct invoke;

        template<class Event, class Head, class... Tail>
        struct invoke<Event, mpl::list<Head, Tail...>> {
            static State execute(Derived* self, State state, Event const& event) {
                return state == Head::start_value && Head::check(self, event)
                    ? Head::process(self, state, event)
                    : invoke<Event, mpl::list<Tail...>>::execute(self, state, event);
            }
        };

        template<class Event>
        struct invoke<Event, mpl::list<>> {
            static State execute(Derived* self, State state, Event const& e) {
                return self->no_transition(state, e);
            }
        };

    private:
        state_type state;
    };
}

#endif
