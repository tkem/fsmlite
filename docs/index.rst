`fsmlite` --- Lightweight finite state machine framework for C++11
=======================================================================

.. default-domain:: cpp

.. namespace:: fsmlite

.. class:: fsmlite::fsm<Derived, State>

   .. type:: State state_type

   .. function:: fsm(state_type init_state = state_type())

   .. function:: void process_event(Event const& e)

      Process event `e`.

   .. function:: state_type current_state() const

      Return the current state of the state machine.

   .. function:: protected state_type no_transition(state_type state, Event const&)

   .. class:: protected table<Row...>

   .. class:: protected row<Start, Event, Target, Action, Guard>

   .. class:: protected fn_row<Start, Event, Target, Action, Guard>

   .. class:: protected mem_fn_row<Start, Event, Target, Action, Guard>
