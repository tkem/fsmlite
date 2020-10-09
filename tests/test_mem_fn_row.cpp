#include <cassert>

#include "fsm.h"

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Init, Even, Odd };

    using event = int;

private:

    bool is_even(const event& e) const {
        return e % 2 == 0;
    }

    bool is_odd(const event& e) const {
        return e % 2 != 0;
    }

private:
    using m = state_machine;

    using transition_table = table<
//              Start Event  Target Action   Guard
//  -----------+-----+------+------+--------+--------------+-
    mem_fn_row< Init, event, Even,  nullptr, &m::is_even    >,
    mem_fn_row< Init, event, Odd,   nullptr, &m::is_odd     >,
    mem_fn_row< Even, event, Odd,   nullptr, &m::is_odd     >,
    mem_fn_row< Even, event, Even,  nullptr, &m::is_even    >,
    mem_fn_row< Odd,  event, Even,  nullptr, &m::is_even    >,
    mem_fn_row< Odd,  event, Odd,   nullptr  /* fallback */ >
//  -----------+-----+------+------+--------+--------------+-
    >;
};

void test_even()
{
    state_machine m;
    assert(m.current_state() == state_machine::Init);
    m.process_event(0);
    assert(m.current_state() == state_machine::Even);
    m.process_event(0);
    assert(m.current_state() == state_machine::Even);
    m.process_event(1);
    assert(m.current_state() == state_machine::Odd);
}

void test_odd()
{
    state_machine m;
    assert(m.current_state() == state_machine::Init);
    m.process_event(1);
    assert(m.current_state() == state_machine::Odd);
    m.process_event(1);
    assert(m.current_state() == state_machine::Odd);
    m.process_event(0);
    assert(m.current_state() == state_machine::Even);
}

int main()
{
    test_even();
    test_odd();
    return 0;
}
