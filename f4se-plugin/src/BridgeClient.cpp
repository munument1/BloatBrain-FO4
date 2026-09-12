#include "pch.h"
#include "BridgeClient.h"

namespace BloatBrain
{
    namespace
    {
        constexpr int kSocketTimeoutMs = 1000;
    }

    BridgeClient::BridgeClient()
    {
        WSADATA data{};
        winsockReady_ = WSAStartup(MAKEWORD(2, 2), &data) == 0;
        if (!winsockReady_) {
            REX::ERROR("WSAStartup failed: {}", WSAGetLastError());
        }
    }

    BridgeClient::~BridgeClient()
    {
        Disconnect();
        if (winsockReady_) {
            WSACleanup();
        }
    }

    bool BridgeClient::Connect(std::string_view host, std::uint16_t port)
    {
        Disconnect();
        if (!winsockReady_) {
            return false;
        }

        socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket_ == INVALID_SOCKET) {
            REX::WARN("socket() failed: {}", WSAGetLastError());
            return false;
        }

        const DWORD timeout = kSocketTimeoutMs;
        setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&timeout), sizeof(timeout));
        setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<const char*>(&timeout), sizeof(timeout));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        std::string hostString(host);
        if (inet_pton(AF_INET, hostString.c_str(), &address.sin_addr) != 1) {
            REX::WARN("bridge host must currently be an IPv4 literal: {}", hostString);
            Disconnect();
            return false;
        }

        if (::connect(socket_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
            REX::DEBUG("bridge connect failed: {}", WSAGetLastError());
            Disconnect();
            return false;
        }

        REX::INFO("connected to BloatBrain bridge at {}:{}", hostString, port);
        return true;
    }

    void BridgeClient::Disconnect()
    {
        if (socket_ != INVALID_SOCKET) {
            ::shutdown(socket_, SD_BOTH);
            closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
    }

    bool BridgeClient::IsConnected() const noexcept
    {
        return socket_ != INVALID_SOCKET;
    }

    bool BridgeClient::SendLine(std::string_view line)
    {
        if (!IsConnected()) {
            return false;
        }

        std::string payload(line);
        payload.push_back('\n');

        std::size_t sentTotal = 0;
        while (sentTotal < payload.size()) {
            const int sent = ::send(
                socket_,
                payload.data() + sentTotal,
                static_cast<int>(payload.size() - sentTotal),
                0);

            if (sent == SOCKET_ERROR || sent == 0) {
                REX::DEBUG("bridge send failed: {}", WSAGetLastError());
                Disconnect();
                return false;
            }
            sentTotal += static_cast<std::size_t>(sent);
        }

        return true;
    }

    std::optional<std::string> BridgeClient::ReceiveLine(std::size_t maxBytes)
    {
        if (!IsConnected()) {
            return std::nullopt;
        }

        std::string line;
        line.reserve(256);

        while (line.size() < maxBytes) {
            char ch = '\0';
            const int received = ::recv(socket_, &ch, 1, 0);
            if (received == SOCKET_ERROR || received == 0) {
                REX::DEBUG("bridge receive failed: {}", WSAGetLastError());
                Disconnect();
                return std::nullopt;
            }

            if (ch == '\n') {
                return line;
            }
            line.push_back(ch);
        }

        REX::WARN("bridge response exceeded {} bytes", maxBytes);
        Disconnect();
        return std::nullopt;
    }
}
