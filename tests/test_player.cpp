#include <cassert>
#include <iostream>
#include <string>

#include "fsm.hpp"

class player: public fsmlite::fsm<player> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Stopped, Open, Empty, Playing, Paused };

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
    using transition_table = table<
//              Start    Event        Target   Action
//  -----------+--------+------------+--------+------------------------+--
    mem_fn_row< Stopped, play,        Playing, &player::start_playback  >,
    mem_fn_row< Stopped, open_close,  Open,    &player::open_drawer     >,
    mem_fn_row< Open,    open_close,  Empty,   &player::close_drawer    >,
    mem_fn_row< Empty,   open_close,  Open,    &player::open_drawer     >,
    mem_fn_row< Empty,   cd_detected, Stopped, &player::store_cd_info   >,
    mem_fn_row< Playing, stop,        Stopped, &player::stop_playback   >,
    mem_fn_row< Playing, pause,       Paused,  &player::pause_playback  >,
    mem_fn_row< Playing, open_close,  Open,    &player::stop_and_open   >,
    mem_fn_row< Paused,  play,        Playing, &player::resume_playback >,
    mem_fn_row< Paused,  stop,        Stopped, &player::stop_playback   >,
    mem_fn_row< Paused,  open_close,  Open,    &player::stop_and_open   >
//  -----------+--------+------------+--------+------------------------+--
    >;
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

int main()
{
    player p;

    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);
    p.process_event(player::cd_detected("Rubber Soul"));
    assert(p.current_state() == player::Stopped);
    p.process_event(player::play());
    assert(p.current_state() == player::Playing);
    p.process_event(player::pause());
    assert(p.current_state() == player::Paused);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Open);
    p.process_event(player::open_close());
    assert(p.current_state() == player::Empty);

    return 0;
}
