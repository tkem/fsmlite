# fsmlite [![Build Status](https://travis-ci.org/tkem/fsmlite.svg?branch=master)](https://travis-ci.org/tkem/fsmlite/) [![Coverage Status](https://coveralls.io/repos/github/tkem/fsmlite/badge.svg?branch=master)](https://coveralls.io/github/tkem/fsmlite?branch=master) [![Documentation Status](https://readthedocs.org/projects/fsmlite/badge/?version=latest&style=flat)](http://fsmlite.readthedocs.io/en/latest/)

**fsmlite** is a lightweight finite state machine framework for C++11.
It is based on concepts first presented by David Abrahams and Aleksey
Gurtovoy in [C++ Template Metaprogramming][1], with additional ideas
taken liberally from Boost's [Meta State Machine][2] (MSM).

The canonical CD player example (with CD-Text and auto-play support!)
therefore looks somewhat like this:

```C++
#include "fsm.h"

#include <string>

class player: public fsmlite::fsm<player> {
    friend class fsm;  // base class needs access to transition_table

    std::string cd_title;
    bool autoplay = false;

public:
    enum states { Stopped, Open, Empty, Playing, Paused };

    player(state_type init_state = Empty) : fsm(init_state) { }

    void set_autoplay(bool f) { autoplay = f; }

    bool is_autoplay() const { return autoplay; }

    const std::string& get_cd_title() const { return cd_title; }

    struct play {};
    struct open_close {};
    struct cd_detected { std::string title; };
    struct stop {};
    struct pause {};

private:
    // guards
    bool is_autoplay(const cd_detected&) const { return autoplay; }
    bool is_bad_cd(const cd_detected& cd) const { return cd.title.empty(); }

    // actions
    void start_playback(const play&);
    void start_autoplay(const cd_detected& cd);
    void open_drawer(const open_close&);
    void open_drawer(const cd_detected& cd);
    void close_drawer(const open_close&);
    void store_cd_info(const cd_detected& cd);
    void stop_playback(const stop&);
    void pause_playback(const pause&);
    void stop_and_open(const open_close&);
    void resume_playback(const play&);

private:
    using m = player;  // for brevity

    using transition_table = table<
//              Start    Event        Target   Action              Guard (optional)
//  -----------+--------+------------+--------+-------------------+---------------+-
    mem_fn_row< Stopped, play,        Playing, &m::start_playback                  >,
    mem_fn_row< Stopped, open_close,  Open,    &m::open_drawer                     >,
    mem_fn_row< Open,    open_close,  Empty,   &m::close_drawer                    >,
    mem_fn_row< Empty,   open_close,  Open,    &m::open_drawer                     >,
    mem_fn_row< Empty,   cd_detected, Open,    &m::open_drawer,    &m::is_bad_cd   >,
    mem_fn_row< Empty,   cd_detected, Playing, &m::start_autoplay, &m::is_autoplay >,
    mem_fn_row< Empty,   cd_detected, Stopped, &m::store_cd_info   /* fallback */  >,
    mem_fn_row< Playing, stop,        Stopped, &m::stop_playback                   >,
    mem_fn_row< Playing, pause,       Paused,  &m::pause_playback                  >,
    mem_fn_row< Playing, open_close,  Open,    &m::stop_and_open                   >,
    mem_fn_row< Paused,  play,        Playing, &m::resume_playback                 >,
    mem_fn_row< Paused,  stop,        Stopped, &m::stop_playback                   >,
    mem_fn_row< Paused,  open_close,  Open,    &m::stop_and_open                   >
//  -----------+--------+------------+--------+-------------------+---------------+-
    >;
};
```

C++17 will give you a little more flexibility:

```C++
class player: public fsmlite::fsm<player> {
    friend class fsm;  // base class needs access to transition_table

    std::string cd_title;
    bool autoplay = false;

public:
    enum states { Stopped, Open, Empty, Playing, Paused };

    player(state_type init_state = Empty) : fsm(init_state) { }

    void set_autoplay(bool f) { autoplay = f; }

    bool is_autoplay() const { return autoplay; }

    const std::string& get_cd_title() const { return cd_title; }

    struct play {};
    struct open_close {};
    struct cd_detected {
        std::string title;
        bool bad() const { return title.empty(); }
    };
    struct stop {};
    struct pause {};

private:
    void start_playback();
    void start_autoplay(const cd_detected& cd);
    void open_drawer();
    void close_drawer();
    void store_cd_info(const cd_detected& cd);
    void stop_playback();
    void pause_playback();
    void resume_playback();
    void stop_and_open();

private:
    using m = player;  // for brevity

    using transition_table = table<
//       Start    Event        Target   Action              Guard (optional)
//  ----+--------+------------+--------+-------------------+-----------------+-
    row< Stopped, play,        Playing, &m::start_playback                    >,
    row< Stopped, open_close,  Open,    &m::open_drawer                       >,
    row< Open,    open_close,  Empty,   &m::close_drawer                      >,
    row< Empty,   open_close,  Open,    &m::open_drawer                       >,
    row< Empty,   cd_detected, Open,    &m::open_drawer,    &cd_detected::bad >,
    row< Empty,   cd_detected, Playing, &m::start_autoplay, &m::is_autoplay   >,
    row< Empty,   cd_detected, Stopped, &m::store_cd_info   /* fallback */    >,
    row< Playing, stop,        Stopped, &m::stop_playback                     >,
    row< Playing, pause,       Paused,  &m::pause_playback                    >,
    row< Playing, open_close,  Open,    &m::stop_and_open                     >,
    row< Paused,  play,        Playing, &m::resume_playback                   >,
    row< Paused,  stop,        Stopped, &m::stop_playback                     >,
    row< Paused,  open_close,  Open,    &m::stop_and_open                     >
//  ----+--------+------------+--------+-------------------+-----------------+-
    >;
};
```

[Documentation][3] is in the works.  In the mean time, please have a
look at the [unit tests][4] for example usage.  For compiler support,
please check out the [Travis CI](https://travis-ci.org/tkem/fsmlite/)
builds.

## License

Copyright (c) 2015-2020 Thomas Kemmer

Licensed under the [MIT License (MIT)][5].

[1]: http://www.informit.com/store/c-plus-plus-template-metaprogramming-concepts-tools-9780321227256
[2]: http://www.boost.org/doc/libs/1_59_0/libs/msm/doc/HTML/index.html
[3]: http://fsmlite.readthedocs.org/en/latest/
[4]: https://github.com/tkem/fsmlite/tree/master/tests
[5]: https://github.com/tkem/fsmlite/tree/master/LICENSE
