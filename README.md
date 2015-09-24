**fsmlite** is a lightweight finite state machine framework for C++11.

Like many other C++ state machine implementations, **fsmlite** is based
on concepts first presented by David Abrahams and Aleksey Gurtovoy in
[C++ Template Metaprogramming][1], with additional ideas taken liberally
from Boost's [MSM][2].  Unsurprisingly, the canonical CD player example
looks somewhat like this:

```
#include <fsmlite/fsm.hpp>

class player: public fsmlite::fsm<player> {
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
```

[1]: http://www.informit.com/store/c-plus-plus-template-metaprogramming-concepts-tools-9780321227256
[2]: http://www.boost.org/doc/libs/1_59_0/libs/msm/doc/HTML/index.html
