#include <cassert>
#include <functional>
#include <iostream>

#include "fsm.h"

using namespace std::placeholders;

int value = 0;

// global actions

void store(int i) { value = i; }
void store1() { value = 1; }
auto store2 = [](){ value = 2; };
struct { void operator()() { value = 3; } } store3;
std::function<void()> store4 = [](){ value = 4; };
void clear() { value = 0; }

// global guards

bool is1(int i) { return i == 1; }
auto is2 = [](int i) { return i == 2; };
struct { bool operator()(int i) const { return i == 3; } } is3;
auto is4 = std::bind(std::equal_to<int>(), _1, 4);

// fsm

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table

public:
    enum states { Init, Running, Exit };

    using event = int;

public:

    static void store5() { value = 5; }

    static bool is5(int i) { return i == 5; }

private:
    using m = state_machine;

    using transition_table = table<
//  Row-Type   Start    Event  Target   Action-Type        Action   Guard-Type      Guard
//  ----------+--------+------+--------+------------------+--------+---------------+-----+-
    typed_row< Init,    event, Running, decltype(&store),  &store                         >,
    typed_row< Running, event, Running, decltype(&store1), &store1, decltype(&is1), &is1  >,
    typed_row< Running, event, Running, decltype(&store2), &store2, decltype(&is2), &is2  >,
    typed_row< Running, event, Running, decltype(&store3), &store3, decltype(&is3), &is3  >,
    typed_row< Running, event, Running, decltype(&store4), &store4, decltype(&is4), &is4  >,
    typed_row< Running, event, Running, decltype(&store5), &store5, decltype(&is5), &is5  >,
    typed_row< Running, event, Exit,    decltype(&clear),  &clear   /* fallback */        >,
    typed_row< Exit,    event, Exit                                                       >
//  ----------+--------+------+--------+------------------+--------+---------------+-----+-
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

    m.process_event(4);
    assert(m.current_state() == state_machine::Running);
    assert(value == 4);

    m.process_event(5);
    assert(m.current_state() == state_machine::Running);
    assert(value == 5);

    m.process_event(42);
    assert(m.current_state() == state_machine::Exit);
    assert(value == 0);

    m.process_event(42);
    assert(m.current_state() == state_machine::Exit);
    assert(value == 0);
    return 0;
}
