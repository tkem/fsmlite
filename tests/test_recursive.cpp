#include <iostream>
#include <stdexcept>
#include <type_traits>

#include "fsm.h"

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Init, Exit };

private:
    void process(const int& event) {
        std::cout << "Processing event: " << event << "\n";
#ifdef NDEBUG
        if (event != 0) {
            throw std::logic_error("recursive invocation detected");
        }
#endif
        process_event(event + 1);
    }

private:
    typedef state_machine m;

    using transition_table = table<
//              Start Event  Target Action
//  -----------+-----+------+------+-----------+-
    mem_fn_row< Init, int,   Exit,  &m::process >
//  -----------+-----+------+------+-----------+-
    >;
};

int main()
{
    state_machine m;
    try {
        m.process_event(0);
    } catch (std::logic_error& e) {
        std::cerr << e.what() << "\n";
        return 0;
    }
    return 1;  /* LCOV_EXCL_LINE */
}
