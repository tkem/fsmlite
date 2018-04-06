#include <cassert>
#include <iostream>
#include <string>

#include "fsm.h"

class player: public fsmlite::fsm<player> {
    friend class fsm;  // base class needs access to transition_table
public:
    enum states { Stopped, Open, Empty, Playing, Paused };

    player(state_type init_state = Empty) : fsm(init_state) {}

    struct play {};
    struct open_close {};
    struct cd_detected {
        cd_detected(const char* s = "") : title(s) { }
        std::string title;
    };
    struct stop {};
    struct pause {};

private:
    void start_playback(const play&);
    void open_drawer(const open_close&);
    void close_drawer(const open_close&);
    void store_cd_info(const cd_detected& cd);
    void stop_playback(const stop&);
    void pause_playback(const pause&);
    void stop_and_open(const open_close&);
    void resume_playback(const play&);

private:
    using transition_table = table<
//              Start    Event        Target   Action
//  -----------+--------+------------+--------+------------------------+-
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
//  -----------+--------+------------+--------+------------------------+-
    >;
};

void player::start_playback(const play&) {
    std::cout << "Starting playback\n";
}

void player::open_drawer(const open_close&) {
    std::cout << "Opening drawer\n";
}

void player::close_drawer(const open_close&) {
    std::cout << "Closing drawer\n";
}

void player::store_cd_info(const cd_detected& cd) {
    std::cout << "Detected CD '" << cd.title << "'\n";
}

void player::stop_playback(const stop&) {
    std::cout << "Stopping playback\n";
}

void player::pause_playback(const pause&) {
    std::cout << "Pausing playback\n";
}

void player::stop_and_open(const open_close&) {
    std::cout << "Stopping and opening drawer\n";
}

void player::resume_playback(const play&) {
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
