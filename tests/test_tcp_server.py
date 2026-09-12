import socket
import threading

from bridge.protocol import Action, ActionCommand, Observation
from bridge.protocol_v2 import MotorCommand, SensoryFrame
from bridge.tcp_server import make_server


def make_observation(**overrides):
    values = {
        "tick": 42,
        "target_visible": True,
        "target_distance": 200.0,
        "target_bearing_deg": 0.0,
        "target_elevation_deg": 0.0,
        "health_fraction": 1.0,
        "recently_damaged": False,
        "in_combat": True,
    }
    values.update(overrides)
    return Observation(**values)


def make_sensory_frame(**overrides):
    values = {
        "tick": 100,
        "dt": 0.05,
        "optic_flow_left": 0.1,
        "optic_flow_right": 0.1,
        "optic_flow_up": 0.1,
        "optic_flow_down": 0.1,
        "looming_left": 0.0,
        "looming_center": 0.0,
        "looming_right": 0.0,
        "familiar_cue_left": 0.0,
        "familiar_cue_center": 0.0,
        "familiar_cue_right": 1.0,
        "damage_signal": 0.0,
        "health_fraction": 1.0,
    }
    values.update(overrides)
    return SensoryFrame(**values)


def start_server():
    server = make_server("127.0.0.1", 0)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    return server, thread


def stop_server(server, thread):
    server.shutdown()
    server.server_close()
    thread.join(timeout=2.0)


def test_tcp_round_trip_v1():
    server, thread = start_server()

    try:
        with socket.create_connection(server.server_address, timeout=2.0) as client:
            stream = client.makefile("rwb")
            observation = make_observation()
            stream.write(observation.to_json().encode("utf-8") + b"\n")
            stream.flush()

            response = stream.readline().decode("utf-8")
            command = ActionCommand.from_json(response)

            assert command.tick == observation.tick
            assert command.action is Action.ATTACK
    finally:
        stop_server(server, thread)


def test_tcp_connection_supports_multiple_v1_ticks():
    server, thread = start_server()

    try:
        with socket.create_connection(server.server_address, timeout=2.0) as client:
            stream = client.makefile("rwb")
            for tick in (1, 2, 3):
                observation = make_observation(tick=tick, target_distance=1000.0)
                stream.write(observation.to_json().encode("utf-8") + b"\n")
                stream.flush()

                command = ActionCommand.from_json(stream.readline().decode("utf-8"))
                assert command.tick == tick
                assert command.action is Action.APPROACH_TARGET
    finally:
        stop_server(server, thread)


def test_tcp_round_trip_v2_returns_continuous_motor_command():
    server, thread = start_server()

    try:
        with socket.create_connection(server.server_address, timeout=2.0) as client:
            stream = client.makefile("rwb")
            frame = make_sensory_frame()
            stream.write(frame.to_json().encode("utf-8") + b"\n")
            stream.flush()

            command = MotorCommand.from_json(stream.readline().decode("utf-8"))
            assert command.tick == frame.tick
            assert command.yaw > 0.0
            assert 0.0 <= command.thrust <= 1.0
    finally:
        stop_server(server, thread)


def test_single_connection_can_transition_from_v1_debug_to_v2_control():
    server, thread = start_server()

    try:
        with socket.create_connection(server.server_address, timeout=2.0) as client:
            stream = client.makefile("rwb")

            observation = make_observation(tick=10)
            stream.write(observation.to_json().encode("utf-8") + b"\n")
            stream.flush()
            action = ActionCommand.from_json(stream.readline().decode("utf-8"))
            assert action.tick == 10

            frame = make_sensory_frame(
                tick=11,
                familiar_cue_right=0.0,
                looming_right=1.0,
            )
            stream.write(frame.to_json().encode("utf-8") + b"\n")
            stream.flush()
            motor = MotorCommand.from_json(stream.readline().decode("utf-8"))
            assert motor.tick == 11
            assert motor.yaw < 0.0
    finally:
        stop_server(server, thread)
