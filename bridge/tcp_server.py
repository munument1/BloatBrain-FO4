"""Localhost TCP bridge for BloatBrain-FO4.

The server accepts persistent TCP connections and supports both protocol paths:

- v1 observation -> discrete ActionCommand (debugging / transport validation)
- v2 SensoryFrame -> continuous MotorCommand (intended FlyBrain boundary)

Each newline-delimited JSON request produces exactly one newline-delimited JSON
response.

Usage:

    python -m bridge.tcp_server
    python -m bridge.tcp_server --host 127.0.0.1 --port 8765
"""

from __future__ import annotations

import argparse
import json
import logging
import socketserver
from typing import Final

from flybrain.continuous_mock import choose_motor

from .mock_controller import choose_action
from .protocol import Observation
from .protocol_v2 import SensoryFrame


DEFAULT_HOST: Final = "127.0.0.1"
DEFAULT_PORT: Final = 8765
MAX_LINE_BYTES: Final = 64 * 1024

LOGGER = logging.getLogger("bloatbrain.bridge")


def dispatch_message(text: str) -> str:
    """Route one JSON message to the matching protocol controller."""

    payload = json.loads(text)
    version = payload.get("protocol_version")

    if version == 1:
        observation = Observation.from_dict(payload)
        return choose_action(observation).to_json()

    if version == 2:
        frame = SensoryFrame.from_dict(payload)
        return choose_motor(frame).to_json()

    raise ValueError(f"unsupported protocol version: {version!r}")


class BloatBrainRequestHandler(socketserver.StreamRequestHandler):
    """Handle one persistent game-side TCP connection."""

    def handle(self) -> None:
        peer = self.client_address
        LOGGER.info("controller client connected: %s:%s", *peer)

        try:
            while True:
                raw_line = self.rfile.readline(MAX_LINE_BYTES + 1)
                if not raw_line:
                    return

                if len(raw_line) > MAX_LINE_BYTES:
                    raise ValueError("incoming JSONL message exceeds size limit")

                text = raw_line.decode("utf-8").strip()
                if not text:
                    continue

                response = dispatch_message(text)
                self.wfile.write(response.encode("utf-8") + b"\n")
                self.wfile.flush()
        except (UnicodeDecodeError, json.JSONDecodeError, ValueError, KeyError) as exc:
            LOGGER.warning("closing malformed client %s:%s: %s", *peer, exc)
        except (ConnectionError, OSError) as exc:
            LOGGER.info("client %s:%s disconnected: %s", *peer, exc)
        finally:
            LOGGER.info("controller client closed: %s:%s", *peer)


class BloatBrainTCPServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True


def make_server(host: str = DEFAULT_HOST, port: int = DEFAULT_PORT) -> BloatBrainTCPServer:
    """Create a bridge server. Port 0 may be used by tests for an ephemeral port."""

    return BloatBrainTCPServer((host, port), BloatBrainRequestHandler)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="BloatBrain-FO4 localhost bridge")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument(
        "--log-level",
        default="INFO",
        choices=("DEBUG", "INFO", "WARNING", "ERROR"),
    )
    return parser


def main() -> int:
    args = build_parser().parse_args()
    logging.basicConfig(
        level=getattr(logging, args.log_level),
        format="%(asctime)s %(levelname)s %(name)s: %(message)s",
    )

    with make_server(args.host, args.port) as server:
        host, port = server.server_address
        LOGGER.info("listening on %s:%s", host, port)
        try:
            server.serve_forever(poll_interval=0.25)
        except KeyboardInterrupt:
            LOGGER.info("shutdown requested")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
