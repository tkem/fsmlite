#include <cassert>
#include <functional>
#include <iostream>

#include "fsm.hpp"

using namespace std::placeholders;

bool is_zero(int i) {
    return i == 0;
}

auto is_one = std::bind(std::equal_to<int>(), 1);

auto is_two = [](int i) { return i == 2; };

class state_machine: public fsmlite::fsm<state_machine> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Init, Running, Exit };

    typedef int event;

private:
    static bool is_done(state_machine const& sm) {
        return false;
    }

private:
    typedef state_machine m;

    using transition_table = table<
//       Start    Event  Target   Action  Action*    Guard              Guard*
//  ----+--------+------+--------+-------+----------+------------------+-----------+-
    row< Init,    event, Running, void,   nullptr                                   >,
    row< Running, event, Running, void,   nullptr,   decltype(is_zero),  &is_zero   >,
    row< Running, event, Running, void,   nullptr,   decltype(is_one),   &is_one    >,
    row< Running, event, Running, void,   nullptr,   decltype(is_two),   &is_two    >,
    row< Running, event, Exit,    void,   nullptr,   decltype(is_done),  &is_done   >,
    row< Exit,    event, Exit                                                       >
//  ----+--------+------+--------+-------+----------+------------------+-----------+-
    >;
};


int main()
{
    state_machine m;
    assert(m.current_state() == state_machine::Init);
    return 0;
}
