# Continuous sensorimotor control

The final BloatBrain companion should look alive because its **closed-loop behavior** differs from a conventional game companion, not because the game displays a neural-network badge or scripted animation.

Protocol v1 remains a debugging tool. Protocol v2 is the intended biological-control boundary.

## Rule: do not send game-semantic movement orders into FlyBrain

The neural backend must not receive commands such as:

- turn left
- approach the player
- evade target
- move to player coordinates

Those already contain the answer.

Instead, Fallout 4 should expose sensory evidence such as local motion, visual expansion, familiar-player salience, damage and internal state. The neural backend then produces continuous motor drives.

```text
Fallout 4 world
    -> sensory encoder
        -> optic flow L/R/U/D
        -> looming L/C/R
        -> familiar cue L/C/R
        -> damage / health
    -> FlyBrain / MaleCNS adapter
    -> motor aggregation
        -> yaw
        -> pitch
        -> lift
        -> thrust
        -> attack drive
    -> Fallout 4 flight actuator
    -> changed world state
    -> next sensory frame
```

## Desired visible behavior

If the loop works, players should notice the difference without being told how it works:

- imperfect fixation on the player instead of exact waypoint following
- frequent small corrections while hovering
- short abrupt direction changes when visual motion becomes asymmetric
- side-stepping or vertical escape when something rapidly expands in view
- overshoot followed by reacquisition rather than GPS-perfect tracking
- temporary searching when the player disappears behind geometry
- irregular approach distance, including occasionally hovering directly in the player's face
- continuous movement variation rather than a small menu of animation-state decisions

These behaviors must emerge from sensory/motor dynamics whenever possible. Do not add a special "annoy the player" state machine merely to fake them.

## Player following

The recruited player is represented by a `familiar_cue` perceptual channel, not by world coordinates. A visual encoder may estimate how much of the cue appears in left, central and right sensory regions.

This deliberately permits imperfect behavior. If the player leaves view, the neural controller is not handed the answer. Reacquisition/search behavior should emerge from the backend or a documented low-level sensory-memory model.

The ESP follow package remains only the **bridge-offline safety fallback**.

## Looming

Looming is the normalized rate at which a nearby object expands in the fly's sensory field. Keep left/center/right channels separate. This lets the neural side determine escape direction from asymmetric activity rather than receiving a pre-decoded `EVADE` instruction.

Examples of sources in Fallout 4 may include:

- player or enemy rapidly approaching
- a wall filling the forward field during flight
- a projectile or large object entering the near field

The first implementation may approximate looming from raycasts/object angular size changes. A later visual backend can derive it from rendered or sampled visual motion.

## Motor adapter

Protocol-v2 outputs are normalized neural motor drives, not world-space velocities.

- `yaw`: signed turning drive
- `pitch`: signed nose-up/nose-down drive
- `lift`: signed vertical drive
- `thrust`: forward drive
- `attack_drive`: propensity to trigger the Bloatfly's attack mechanism

The F4SE game adapter should enforce engine constraints, collision safety, animation/cooldown requirements and modest temporal smoothing. It should **not** reinterpret the output into high-level AI packages while the neural bridge is online.

## Control rate

Start at 20 Hz for protocol-v2 integration. This is fast enough to expose continuous corrections while leaving room for external neural computation. The actuator can interpolate between commands at the game frame rate.

Once the real backend is benchmarked, increase or reduce the control rate based on measured latency rather than assuming that more updates are always better.

## Safety fallback

When a motor command is stale or the bridge disconnects:

1. stop applying neural motor output;
2. clear `BB_BridgeOnline`;
3. let `BB_FollowFallbackPackage` take over if recruited;
4. never continue replaying the last yaw/thrust command.

This keeps failures boring and recoverable while allowing online behavior to remain deliberately insect-like.
