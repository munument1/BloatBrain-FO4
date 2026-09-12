# BloatBrain-FO4

Experimental Fallout 4 modding project that drives a Bloatfly with an external **Drosophila-inspired connectome/neural simulation** instead of relying only on the vanilla game AI.

> **Status:** early research prototype — Python bridge tested, F4SE transport scaffold builds successfully in Windows CI; in-game loading is the next verification step.

## Goal

Build a bridge between Fallout 4 and an external fly-brain simulation so that a Bloatfly can perceive a simplified version of the game world and receive movement/combat decisions from neural activity.

The first milestone is intentionally small:

> Replace the decision-making of **one test Bloatfly** with an external controller that can choose basic actions such as approach, evade, turn, attack, and idle.

## Architecture

```text
Fallout 4
   |
   | game-state observations
   v
F4SE / CommonLibF4 plugin
   |
   | persistent localhost TCP + JSONL
   v
Bridge (127.0.0.1:8765)
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
├─ f4se-plugin/   # Fallout 4 integration + WinSock client
├─ bridge/        # TCP server and observation/action protocol
├─ flybrain/      # neural/connectome simulation adapter
├─ configs/       # sensory and motor mappings
├─ tests/         # protocol and TCP round-trip tests
└─ docs/          # architecture and research notes
```

## Quick bridge test

Python 3.11+:

```powershell
python -m pip install -e ".[dev]"
python -m bridge.tcp_server
```

The bridge listens on `127.0.0.1:8765`. It currently routes observations to the deterministic mock controller so the Fallout 4 integration can be tested before the real neural backend exists.

Run the automated protocol/TCP tests with:

```powershell
pytest
```

The F4SE worker currently sends a synthetic protocol-v1 observation once per second and logs the returned action. This is a temporary smoke loop that will be replaced by observations from a real Bloatfly actor.

The Windows CI workflow compiles the plugin and publishes `BloatBrainFO4.dll` as the `BloatBrainFO4-CI` workflow artifact on successful builds.

See [`f4se-plugin/README.md`](f4se-plugin/README.md) for the CommonLibF4/XMake setup.

## MVP action space

The first prototype uses a deliberately small discrete action set:

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
6. Never block Fallout 4's main thread on external neural computation or socket I/O.

## Planned phases

### Phase 0 — Scaffold

- [x] define observation/action protocol
- [x] create local mock controller
- [x] implement persistent localhost TCP bridge
- [x] test multi-tick TCP round trips
- [x] create F4SE/CommonLibF4 transport scaffold
- [x] move blocking bridge I/O to a background worker
- [x] compile the F4SE plugin in Windows CI
- [x] add a synthetic F4SE-to-Python smoke round-trip loop

### Phase 1 — Fallout 4 control loop

- [ ] load the F4SE plugin in Fallout 4 and verify the smoke loop in logs
- [ ] detect a designated Bloatfly
- [ ] read target/game state
- [ ] serialize protocol-v1 observations from the actor
- [ ] receive and validate matching action commands
- [ ] disable or constrain vanilla decision-making for the test actor
- [ ] apply returned actions
- [ ] recover safely on timeout/disconnect

### Phase 2 — FlyBrain integration

- [ ] load/connect to the selected Drosophila neural model
- [ ] map game observations to sensory stimulation
- [ ] aggregate candidate motor-neuron activity
- [ ] decode activity into the MVP action space

### Phase 3 — Experiments

- compare vanilla AI vs FlyBrain behavior
- test different sensory mappings
- record trajectories and action distributions
- explore reward/modulatory signals only after the base loop is stable

## Important note

This project does **not** claim to reproduce a conscious fly or perfectly simulate a biological nervous system. A connectome provides wiring information; simulation dynamics, sensory encoding, neuromodulation, and motor decoding all require modeling assumptions.

## License

No license has been selected yet.
