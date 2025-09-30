#include <cassert>

#include "fsmlite.h"

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsmlite::fsm<state_machine>;  // base class needs access to transition_table
public:
    enum states { Init, Exit, Error };

    struct event {};
    struct reset {};

private:
    template<class Event>
    state_type no_transition(const Event&) {
        return Error;
    }

    state_type no_transition(const reset&) {
        return Init;
    }

private:
    using transition_table = table<
//  Row-Type   Start Event  Target
//  ----------+-----+------+------+-
    basic_row< Init, event, Exit   >
//  ----------+-----+------+------+-
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
