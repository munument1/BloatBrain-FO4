# F4SE Plugin

This directory will contain the Fallout 4 game-side integration.

## Prototype responsibilities

- acquire one designated Bloatfly actor
- read the minimal observation set defined by protocol v1
- send observations to the external bridge
- receive action commands
- translate actions into safe game-side behavior
- restore fallback behavior when the controller is unavailable

## Initial implementation strategy

The first F4SE prototype should prioritize observability over sophistication:

1. identify the actor reliably
2. log every sampled observation
3. prove external round-trip communication with the mock controller
4. apply only one or two actions first (`IDLE`, `ATTACK` or directional turn)
5. expand the action set after timeout/recovery behavior is stable

No neural-model code should live in the plugin. The game-facing layer only implements transport, observation collection, and action execution.

## Open decisions

Before the first C++ implementation we need to lock down:

- supported Fallout 4 runtime version(s)
- F4SE/CommonLibF4-ng toolchain choice
- actor selection method for the test Bloatfly
- initial IPC transport (localhost socket is the simplest debugging target)
- exact action-to-engine mapping for flying actors
