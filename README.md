**fsmlite** is a lightweight finite state machine framework for C++11.

Like many other C++ state machine implementations, **fsmlite** is
based on concepts first presented by David Abrahams and Aleksey
Gurtovoy in [C++ Template Metaprogramming][1], with additional ideas
taken liberally from Boost's [Meta State Machine][2] (MSM).

Unsurprisingly, the canonical CD player example looks somewhat like
this:

```C++
#include <fsmlite/fsm.hpp>

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
```

[Documentation][3] is in the works.  In the mean time, please have a
look at the [unit tests][4] for example usage.

[1]: http://www.informit.com/store/c-plus-plus-template-metaprogramming-concepts-tools-9780321227256
[2]: http://www.boost.org/doc/libs/1_59_0/libs/msm/doc/HTML/index.html
[3]: http://fsmlite.readthedocs.org/en/latest/
[4]: https://github.com/tkem/fsmlite/tree/master/tests
