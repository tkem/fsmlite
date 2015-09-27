#include <cassert>
#include <iostream>

#include "fsm.hpp"

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Init, Running, Exit };

    struct event {
        event(int v) : value(v) {}
        int value;
    };

    struct action {
        void operator()(state_machine&, event const&) {
            ++called;
        }
        static int called;
    };

private:
    struct is_even {
        bool operator()(state_machine const&, event const& e) {
            return e.value % 2 == 0;
        }
    };

    struct is_odd {
        bool operator()(state_machine const&, event const& e) {
            return e.value % 2 != 0;
        }
    };

private:
    typedef state_machine m;

private:
    using transition_table = table<
//       Start    Event  Target   Action     Guard
//  ----+--------+------+--------+----------+----------+--
    row< Init,    event, Running, m::action             >,
    row< Running, event, Running, m::action, m::is_even >,
    row< Running, event, Exit,    void,      m::is_odd  >,
    row< Exit,    event, Exit                           >
//  ----+--------+------+--------+----------+----------+--
    >;
};


int state_machine::action::called = 0;

int main()
{
    state_machine m;
    assert(state_machine::action::called == 0);
    assert(m.current_state() == state_machine::Init);

    m.process_event(state_machine::event(0));
    assert(state_machine::action::called == 1);
    assert(m.current_state() == state_machine::Running);

    m.process_event(state_machine::event(0));
    assert(state_machine::action::called == 2);
    assert(m.current_state() == state_machine::Running);

    m.process_event(state_machine::event(1));
    assert(state_machine::action::called == 2);
    assert(m.current_state() == state_machine::Exit);

    m.process_event(state_machine::event(0));
    assert(state_machine::action::called == 2);
    assert(m.current_state() == state_machine::Exit);

    return 0;
}
