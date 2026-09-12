# Architecture

## 1. Scope

The first BloatBrain-FO4 prototype separates game integration from neural simulation. Fallout 4 should not need to know how the fly-brain backend works, and the fly-brain backend should not depend on Fallout 4 internals.

The initial success condition is simple: one designated Bloatfly receives observations from the game and executes externally selected actions in a stable loop.

## 2. Components

### Fallout 4 / F4SE integration

Responsibilities:

- identify the test Bloatfly actor
- sample a small set of observations
- suppress or constrain vanilla decision-making for that actor
- send observations to the bridge
- receive an action command
- translate that command into game-side movement/combat behavior
- fail safely if the external controller disconnects

The F4SE side should avoid embedding connectome data or neural-model logic.

### Bridge

Responsibilities:

- version the IPC protocol
- validate observation/action messages
- log data for reproducible experiments
- provide a mock controller for integration testing
- later host socket/shared-memory transport as needed

The prototype protocol is newline-delimited JSON because it is transparent and easy to inspect. It can be replaced with a binary transport later if profiling shows that serialization matters.

### FlyBrain adapter

Responsibilities:

- load or connect to the selected Drosophila/connectome simulation
- convert normalized game observations into neural stimulation
- advance neural dynamics
- aggregate candidate motor-related outputs
- return scores or a decoded action

The adapter should isolate model-specific code so multiple neural backends can be compared without changing Fallout 4 integration.

## 3. Initial control loop

```text
1. F4SE samples Bloatfly + target state
2. F4SE emits Observation(tick=N)
3. Bridge validates / logs observation
4. Controller or FlyBrain computes action
5. Bridge emits ActionCommand(tick=N)
6. F4SE applies action for the next control interval
7. Repeat
```

Target control frequency for the first experiment: approximately 10 Hz. Rendering and animation remain game-side and can run at normal frame rate.

## 4. Observation contract

Version 1 observations contain:

- `tick`
- `target_visible`
- `target_distance`
- `target_bearing_deg`
- `target_elevation_deg`
- `health_fraction`
- `recently_damaged`
- `in_combat`

This intentionally avoids raw pixels. Raw or reduced visual input can be introduced after the actor-control loop is proven reliable.

## 5. Action contract

Version 1 action space:

- `IDLE`
- `FORWARD`
- `TURN_LEFT`
- `TURN_RIGHT`
- `ASCEND`
- `DESCEND`
- `APPROACH_TARGET`
- `EVADE_TARGET`
- `ATTACK`

Each command includes the source observation tick and a confidence value from 0 to 1.

## 6. Failure behavior

External AI must never leave the test actor in an undefined state.

Recommended prototype behavior:

- controller timeout -> `IDLE` or restore vanilla AI
- malformed action -> reject and log
- mismatched tick -> ignore stale command
- bridge disconnect -> restore safe fallback behavior
- game load / cell transition -> re-acquire actor before resuming control

## 7. Biological-model boundary

A connectome supplies structural connectivity, not a complete nervous-system state. Any neural simulation necessarily introduces assumptions about neuronal dynamics, synaptic effects, sensory encoding, neuromodulation, and motor decoding.

For that reason, BloatBrain-FO4 should keep the following layers explicit and separately configurable:

```text
Game observation
    -> sensory encoder
    -> neural simulation
    -> motor-neuron aggregation
    -> action decoder
    -> Fallout 4 actor command
```

This makes it possible to test which behaviors come from the connectome and which come from the chosen interface assumptions.

## 8. Development order

1. Validate JSON protocol locally.
2. Implement a mock game client and controller.
3. Build the F4SE test-actor loop against the mock controller.
4. Add logging and timeout recovery.
5. Integrate the first FlyBrain backend.
6. Only then increase sensory complexity or add reward/modulatory experiments.
