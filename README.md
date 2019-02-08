# fsmlite [![Build Status](https://travis-ci.org/tkem/fsmlite.svg?branch=master)](https://travis-ci.org/tkem/fsmlite/) [![Coverage Status](https://coveralls.io/repos/github/tkem/fsmlite/badge.svg?branch=master)](https://coveralls.io/github/tkem/fsmlite?branch=master) [![Documentation Status](https://readthedocs.org/projects/fsmlite/badge/?version=latest&style=flat)](http://fsmlite.readthedocs.io/en/latest/)

**fsmlite** is a lightweight finite state machine framework for C++11.
It is based on concepts first presented by David Abrahams and Aleksey
Gurtovoy in [C++ Template Metaprogramming][1], with additional ideas
taken liberally from Boost's [Meta State Machine][2] (MSM).

The canonical CD player example looks somewhat like this:

```C++
#include <fsmlite/fsm.h>

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

## License

Copyright (c) 2015-2019 Thomas Kemmer

Licensed under the [MIT License (MIT)][5].

[1]: http://www.informit.com/store/c-plus-plus-template-metaprogramming-concepts-tools-9780321227256
[2]: http://www.boost.org/doc/libs/1_59_0/libs/msm/doc/HTML/index.html
[3]: http://fsmlite.readthedocs.org/en/latest/
[4]: https://github.com/tkem/fsmlite/tree/master/tests
[5]: https://github.com/tkem/fsmlite/tree/master/LICENSE
