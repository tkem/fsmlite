# fsmlite [![CI build status](https://img.shields.io/github/workflow/status/tkem/fsmlite/CI)](https://github.com/tkem/fsmlite/actions) [![Test coverage](https://img.shields.io/codecov/c/github/tkem/fsmlite/master.svg)](https://codecov.io/gh/tkem/fsmlite) [![Documentation](https://img.shields.io/readthedocs/fsmlite.svg)](https://fsmlite.readthedocs.io/en/latest/)

**fsmlite** is a lightweight finite state machine framework for C++17.
It is based on concepts first presented by David Abrahams and Aleksey
Gurtovoy in [C++ Template Metaprogramming][1], with additional ideas
taken liberally from Boost's [Meta State Machine][2] (MSM), and
slightly "modernized" to make use of features more recently
introduced to the C++ standard.

The canonical CD player example (with CD-Text and auto-play support!)
therefore looks somewhat like this:

```C++
#include "fsm.h"

#include <string>

class player: public fsmlite::fsm<player> {
    // grant base class access to private transition_table
    friend class fsmlite::fsm<player>;

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

Basic [Documentation][3] is available, but please also have a look at
the [unit tests][4] for example usage.

## License

Copyright (c) 2015-2025 Thomas Kemmer

Licensed under the [MIT License (MIT)][5].

[1]: https://www.informit.com/store/c-plus-plus-template-metaprogramming-concepts-tools-9780321227256
[2]: https://www.boost.org/doc/libs/1_59_0/libs/msm/doc/HTML/index.html
[3]: https://fsmlite.readthedocs.io/en/latest/
[4]: https://github.com/tkem/fsmlite/tree/master/tests
[5]: https://github.com/tkem/fsmlite/tree/master/LICENSE
