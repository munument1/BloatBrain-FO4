from bridge.mock_controller import choose_action
from bridge.protocol import Action, ActionCommand, Observation


def make_observation(**overrides):
    values = {
        "tick": 1,
        "target_visible": True,
        "target_distance": 1000.0,
        "target_bearing_deg": 0.0,
        "target_elevation_deg": 0.0,
        "health_fraction": 1.0,
        "recently_damaged": False,
        "in_combat": True,
    }
    values.update(overrides)
    return Observation(**values)


def test_observation_json_round_trip():
    source = make_observation()
    restored = Observation.from_json(source.to_json())
    assert restored == source


def test_action_json_round_trip():
    source = ActionCommand(tick=10, action=Action.ATTACK, confidence=0.9)
    restored = ActionCommand.from_json(source.to_json())
    assert restored == source


def test_mock_controller_turns_toward_target():
    left = choose_action(make_observation(target_bearing_deg=-30.0))
    right = choose_action(make_observation(target_bearing_deg=30.0))
    assert left.action is Action.TURN_LEFT
    assert right.action is Action.TURN_RIGHT


def test_mock_controller_attacks_at_close_range():
    command = choose_action(make_observation(target_distance=200.0))
    assert command.action is Action.ATTACK


def test_recent_damage_has_priority():
    command = choose_action(
        make_observation(target_distance=100.0, recently_damaged=True)
    )
    assert command.action is Action.EVADE_TARGET
