#pragma once

// WinSock2 must be included before Windows headers pulled in by F4SE/CommonLibF4.
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <RE/Fallout.h>
#include <F4SE/F4SE.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
