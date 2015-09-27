#include <cassert>
#include <iostream>

#include "fsm.hpp"

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Init, Exit, Error };

    struct event {};
    struct reset {};

private:
    template<class Event>
    state_type no_transition(Event const&) {
        return Error;
    }

    state_type no_transition(reset const&) {
        return Init;
    }

private:
    using transition_table = table<
//       Start Event  Target
//  ----+-----+------+------+--
    row< Init, event, Exit   >
//  ----+-----+------+------+--
    >;
};

int main()
{
    state_machine m;
    assert(m.current_state() == state_machine::Init);
    m.process_event(state_machine::reset());
    assert(m.current_state() == state_machine::Init);
    m.process_event(state_machine::event());
    assert(m.current_state() == state_machine::Exit);
    m.process_event(state_machine::reset());
    assert(m.current_state() == state_machine::Init);
    m.process_event(state_machine::event());
    assert(m.current_state() == state_machine::Exit);
    m.process_event(state_machine::event());
    assert(m.current_state() == state_machine::Error);
    return 0;
}
