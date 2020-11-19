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
//  Row-Type   Start    Event  Target   Action-Type           Action      Guard-Type         Guard
//  ----------+--------+------+--------+---------------------+-----------+------------------+-------+-
    basic_row< Init,    event, Running, decltype(&store),     &store                                 >,
    basic_row< Running, event, Running, decltype(&store),     &store,     decltype(&is1),    &is1    >,
    basic_row< Running, event, Running, decltype(&m::store2), &m::store2, decltype(&m::is2), &m::is2 >,
    basic_row< Running, event, Running, decltype(&m::store3), &m::store3, decltype(&m::is3), &m::is3 >,
    basic_row< Running, event, Exit,    decltype(&clear),     &clear      /* fallback */             >,
    basic_row< Exit,    event, Exit                                                                  >
//  ----------+--------+------+--------+---------------------+-----------+------------------+-------+-
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
