#include <cassert>
#include <type_traits>

#include "fsm.h"

enum class State { Init, Exit };

class state_machine: public fsmlite::fsm<state_machine, State> {
    friend class fsmlite::fsm<state_machine, State>;  // base class needs access to transition_table
public:
    struct event {};

private:
    using transition_table = table<
//  Row-Type   Start        Event  Target
//  ----------+------------+------+-----------+-
    basic_row< State::Init, event, State::Exit >
//  ----------+------------+------+-----------+-
    >;

    static_assert(std::is_same<state_type, State>::value,
                  "state_machine::state_type == State");
};

int main()
{
    state_machine m;
    assert(m.current_state() == State::Init);
    m.process_event(state_machine::event());
    assert(m.current_state() == State::Exit);
    return 0;
}
