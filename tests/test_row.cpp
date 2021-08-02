#include <cassert>

#include "fsm.h"

int value = 0;

// global actions

void store(int i) { value = i; }
void clear() { value = 0; }

// global guards

bool is1(int i) { return i == 1; }

// fsm

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsmlite::fsm<state_machine>;  // base class needs access to transition_table

public:
    enum states { Init, Running, Exit };

    using event = int;

public:

    static void store2() { value = 2; }

    static bool is2(int i) { return i == 2; }

    void store3() { value = 3; }

    bool is3(int i) const { return i == 3; }

private:
    using m = state_machine;

    using transition_table = table<
//       Start    Event  Target   Action      Guard
//  ----+--------+------+--------+-----------+-------+-
    row< Init,    event, Running, &store              >,
    row< Running, event, Running, &store,     &is1    >,
    row< Running, event, Running, &m::store2, &m::is2 >,
    row< Running, event, Running, &m::store3, &m::is3 >,
    row< Running, event, Exit,    &clear              >,
    row< Exit,    event, Exit                         >
//  ----+--------+------+--------+-----------+-------+-
    >;
};

int main()
{
    state_machine m;
    assert(m.current_state() == state_machine::Init);
    assert(value == 0);

    m.process_event(42);
    assert(m.current_state() == state_machine::Running);
    assert(value == 42);

    m.process_event(1);
    assert(m.current_state() == state_machine::Running);
    assert(value == 1);

    m.process_event(2);
    assert(m.current_state() == state_machine::Running);
    assert(value == 2);

    m.process_event(3);
    assert(m.current_state() == state_machine::Running);
    assert(value == 3);

    m.process_event(42);
    assert(m.current_state() == state_machine::Exit);
    assert(value == 0);

    m.process_event(42);
    assert(m.current_state() == state_machine::Exit);
    assert(value == 0);

    return 0;
}
