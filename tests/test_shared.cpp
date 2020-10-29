#include <cassert>
#include <memory>

#include "fsm.h"

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Init, Exit };

    using event = std::shared_ptr<int>;

    void action(const event&) { }

private:
    using m = state_machine;

    using transition_table = table<
//  Row-Type    Start Event  Target Action
//  -----------+-----+------+------+----------+-
    mem_fn_row< Init, event, Exit,  &m::action >
//  -----------+-----+------+------+----------+-
    >;
};

int main()
{
    state_machine m;
    assert(m.current_state() == state_machine::Init);
    m.process_event(state_machine::event());
    assert(m.current_state() == state_machine::Exit);
    return 0;
}
