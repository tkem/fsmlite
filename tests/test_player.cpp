#include <cassert>
#include <iostream>
#include <string>

#include "fsm.hpp"

class player: public fsmlite::fsm<player> {
public:
    enum { Stopped, Open, Empty, Playing, Paused };

    player(state_type init_state = Empty) : fsm(init_state) {}

    struct play {};
    struct open_close {};
    struct cd_detected {
        cd_detected(char const* s = "") : title(s) { }
        std::string title;
    };
    struct stop {};
    struct pause {};

private:
    void start_playback(play const&);
    void open_drawer(open_close const&);
    void close_drawer(open_close const&);
    void store_cd_info(cd_detected const& cd);
    void stop_playback(stop const&);
    void pause_playback(pause const&);
    void stop_and_open(open_close const&);
    void resume_playback(play const&);

private:
    struct transition_table: table<
//       Start    Event        Target   Action
//  ----+--------+------------+--------+------------------------+--
    row< Stopped, play,        Playing, &player::start_playback  >,
    row< Stopped, open_close,  Open,    &player::open_drawer     >,
    row< Open,    open_close,  Empty,   &player::close_drawer    >,
    row< Empty,   open_close,  Open,    &player::open_drawer     >,
    row< Empty,   cd_detected, Stopped, &player::store_cd_info   >,
    row< Playing, stop,        Stopped, &player::stop_playback   >,
    row< Playing, pause,       Paused,  &player::pause_playback  >,
    row< Playing, open_close,  Open,    &player::stop_and_open   >,
    row< Paused,  play,        Playing, &player::resume_playback >,
    row< Paused,  stop,        Stopped, &player::stop_playback   >,
    row< Paused,  open_close,  Open,    &player::stop_and_open   >
//  ----+--------+------------+--------+------------------------+--
    > {};

    friend fsm_type;  // base class needs access to transition_table
};

void player::start_playback(play const&) {
    std::cout << "Starting playback\n";
}

void player::open_drawer(open_close const&) {
    std::cout << "Opening drawer\n";
}

void player::close_drawer(open_close const&) {
    std::cout << "Closing drawer\n";
}

void player::store_cd_info(cd_detected const& cd) {
    std::cout << "Detected CD '" << cd.title << "'\n";
}

void player::stop_playback(stop const&) {
    std::cout << "Stopping playback\n";
}

void player::pause_playback(pause const&) {
    std::cout << "Pausing playback\n";
}

void player::stop_and_open(open_close const&) {
    std::cout << "Stopping and opening drawer\n";
}

void player::resume_playback(play const&) {
    std::cout << "Resuming playback\n";
}

template<class Event>
player::state_type transition(player& p, Event const& e) {
    p.process_event(e);
    return p.current_state();
}

int main()
{
    player p;
    assert(transition(p, player::open_close()) == player::Open);
    assert(transition(p, player::open_close()) == player::Empty);
    assert(transition(p, player::cd_detected("Rubber Soul")) == player::Stopped);
    assert(transition(p, player::play()) == player::Playing);
    assert(transition(p, player::pause()) == player::Paused);
    assert(transition(p, player::open_close()) == player::Open);
    assert(transition(p, player::open_close()) == player::Empty);
    return 0;
}
