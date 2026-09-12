#pragma once

namespace BloatBrain
{
    class BridgeClient
    {
    public:
        BridgeClient();
        ~BridgeClient();

        BridgeClient(const BridgeClient&) = delete;
        BridgeClient& operator=(const BridgeClient&) = delete;

        bool Connect(std::string_view host, std::uint16_t port);
        void Disconnect();
        [[nodiscard]] bool IsConnected() const noexcept;

        bool SendLine(std::string_view line);
        std::optional<std::string> ReceiveLine(std::size_t maxBytes = 64 * 1024);

    private:
        SOCKET socket_{ INVALID_SOCKET };
        bool winsockReady_{ false };
    };
}
