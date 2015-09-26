#include <cassert>
#include <iostream>

#include "fsm.hpp"

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Init, Even, Odd };

    struct event {
        event(int v) : value(v) {}
        int value;
    };

private:
    bool is_even(event const& e) const {
        return e.value % 2 == 0;
    }

    bool is_odd(event const& e) const {
        return e.value % 2 != 0;
    }

private:
    typedef state_machine m;

    struct transition_table: table<
//              Start Event  Target Action   Guard
//  -----------+-----+------+------+--------+--------------+--
    mem_fn_row< Init, event, Even,  nullptr, &m::is_even    >,
    mem_fn_row< Init, event, Odd,   nullptr, &m::is_odd     >,
    mem_fn_row< Even, event, Even,  nullptr, &m::is_even    >,
    mem_fn_row< Even, event, Odd,   nullptr, &m::is_odd     >,
    mem_fn_row< Odd,  event, Even,  nullptr, &m::is_even    >,
    mem_fn_row< Odd,  event, Odd,   nullptr  /* fallback */ >
//  -----------+-----+------+------+--------+--------------+--
    > {};
};

void test_even()
{
    state_machine m;
    m.process_event(state_machine::event(0));
    assert(m.current_state() == state_machine::Even);
    m.process_event(state_machine::event(0));
    assert(m.current_state() == state_machine::Even);
    m.process_event(state_machine::event(1));
    assert(m.current_state() == state_machine::Odd);
}

void test_odd()
{
    state_machine m;
    m.process_event(state_machine::event(1));
    assert(m.current_state() == state_machine::Odd);
    m.process_event(state_machine::event(1));
    assert(m.current_state() == state_machine::Odd);
    m.process_event(state_machine::event(0));
    assert(m.current_state() == state_machine::Even);
}

int main()
{
    test_even();
    test_odd();
    return 0;
}
