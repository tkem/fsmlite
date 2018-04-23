#include <cassert>
#include <iostream>
#include <type_traits>

#include "fsm.h"

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Init, Exit };

    typedef int event;

private:
    void process(const event& e) {
        std::cout << "Processing event\n";
        process_event(e);
    }

    void ignore(const event& e) {
        std::cout << "Ignoring event\n";
    }

private:
    typedef state_machine m;

    using transition_table = table<
//              Start Event  Target Action
//  -----------+-----+------+------+-----------+-
    mem_fn_row< Init, event, Exit,  &m::process >,
    mem_fn_row< Init, event, Exit,  &m::ignore  >
//  -----------+-----+------+------+-----------+-
    >;
};

int main()
{
    state_machine m;
    m.process_event(state_machine::event());
    return 0;  // this should never happen!
}
