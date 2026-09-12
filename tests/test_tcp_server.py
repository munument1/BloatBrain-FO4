import socket
import threading

from bridge.protocol import Action, ActionCommand, Observation
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


def test_tcp_round_trip():
    server = make_server("127.0.0.1", 0)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()

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
        server.shutdown()
        server.server_close()
        thread.join(timeout=2.0)


def test_tcp_connection_supports_multiple_ticks():
    server = make_server("127.0.0.1", 0)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()

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
        server.shutdown()
        server.server_close()
        thread.join(timeout=2.0)
