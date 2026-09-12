# BloatBrain-FO4

Experimental Fallout 4 modding project that drives a Bloatfly with an external **Drosophila-inspired connectome/neural simulation** instead of relying only on the vanilla game AI.

> **Status:** early research / prototype scaffold

## Goal

Build a bridge between Fallout 4 and an external fly-brain simulation so that a Bloatfly can perceive a simplified version of the game world and receive movement/combat decisions from neural activity.

The first milestone is intentionally small:

> Replace the decision-making of **one test Bloatfly** with an external controller that can choose basic actions such as approach, evade, turn, attack, and idle.

## Proposed architecture

```text
Fallout 4
   |
   | game-state observations
   v
F4SE plugin
   |
   | local IPC
   v
Bridge
   |
   | sensory encoding
   v
FlyBrain simulation
   |
   | motor activity / action scores
   v
Action decoder
   |
   v
F4SE plugin -> Bloatfly movement / combat behavior
```

## Repository layout

```text
BloatBrain-FO4/
├─ f4se-plugin/   # Fallout 4 integration layer
├─ bridge/        # IPC and observation/action protocol
├─ flybrain/      # neural/connectome simulation adapter
├─ configs/       # sensory and motor mappings
└─ docs/          # architecture and research notes
```

## MVP action space

The first prototype will use a deliberately small discrete action set:

- `IDLE`
- `FORWARD`
- `TURN_LEFT`
- `TURN_RIGHT`
- `ASCEND`
- `DESCEND`
- `APPROACH_TARGET`
- `EVADE_TARGET`
- `ATTACK`

## MVP observations

The Fallout 4 side should initially expose only information that is easy to validate:

- target visible / not visible
- target distance
- target bearing relative to the Bloatfly
- relative target elevation
- Bloatfly health fraction
- recent damage event
- combat state

Later versions can experiment with richer visual, spatial, and reward signals.

## Design principles

1. **Get the loop working before increasing biological complexity.**
2. Keep Fallout 4 integration and neural simulation loosely coupled.
3. Make sensory/motor mappings configurable instead of hard-coding experiments.
4. Log every observation and selected action so behavior can be reproduced.
5. Treat biological fidelity as an experimental question, not a marketing claim.

## Planned phases

### Phase 0 — Scaffold

- define observation/action protocol
- create local mock controller
- document architecture

### Phase 1 — Fallout 4 control loop

- detect a designated Bloatfly
- read target/game state
- disable or constrain vanilla decision-making for the test actor
- send observations to the external controller
- apply returned actions

### Phase 2 — FlyBrain integration

- load/connect to the selected Drosophila neural model
- map game observations to sensory stimulation
- aggregate candidate motor-neuron activity
- decode activity into the MVP action space

### Phase 3 — Experiments

- compare vanilla AI vs FlyBrain behavior
- test different sensory mappings
- record trajectories and action distributions
- explore reward/modulatory signals only after the base loop is stable

## Important note

This project does **not** claim to reproduce a conscious fly or perfectly simulate a biological nervous system. A connectome provides wiring information; simulation dynamics, sensory encoding, neuromodulation, and motor decoding all require modeling assumptions.

## License

No license has been selected yet.
