#include "ProtocolV2.h"

#include <charconv>
#include <cmath>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace BloatBrain::ProtocolV2
{
    namespace
    {
        enum class JsonKind { number, string };

        struct JsonValue
        {
            JsonKind kind;
            std::string text;
        };

        using JsonObject = std::unordered_map<std::string, JsonValue>;

        void SetError(std::string* error, std::string message)
        {
            if (error) {
                *error = std::move(message);
            }
        }

        class ObjectParser
        {
        public:
            explicit ObjectParser(std::string_view input) : input_(input) {}

            std::optional<JsonObject> Parse(std::string* error)
            {
                JsonObject result;
                SkipWhitespace();
                if (!Take('{')) {
                    SetError(error, "expected a JSON object");
                    return std::nullopt;
                }

                SkipWhitespace();
                if (Take('}')) {
                    return result;
                }

                while (true) {
                    auto key = ParseString(error);
                    if (!key) {
                        return std::nullopt;
                    }
                    SkipWhitespace();
                    if (!Take(':')) {
                        SetError(error, "expected ':' after JSON field name");
                        return std::nullopt;
                    }
                    SkipWhitespace();

                    auto value = Peek() == '"' ? ParseStringValue(error) : ParseNumberValue(error);
                    if (!value) {
                        return std::nullopt;
                    }
                    if (!result.emplace(std::move(*key), std::move(*value)).second) {
                        SetError(error, "duplicate JSON field");
                        return std::nullopt;
                    }

                    SkipWhitespace();
                    if (Take('}')) {
                        break;
                    }
                    if (!Take(',')) {
                        SetError(error, "expected ',' or '}' in JSON object");
                        return std::nullopt;
                    }
                    SkipWhitespace();
                }

                SkipWhitespace();
                if (position_ != input_.size()) {
                    SetError(error, "unexpected data after JSON object");
                    return std::nullopt;
                }
                return result;
            }

        private:
            char Peek() const { return position_ < input_.size() ? input_[position_] : '\0'; }

            bool Take(char expected)
            {
                if (Peek() != expected) {
                    return false;
                }
                ++position_;
                return true;
            }

            void SkipWhitespace()
            {
                while (position_ < input_.size()) {
                    const char ch = input_[position_];
                    if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') {
                        return;
                    }
                    ++position_;
                }
            }

            std::optional<std::string> ParseString(std::string* error)
            {
                if (!Take('"')) {
                    SetError(error, "expected a JSON string");
                    return std::nullopt;
                }

                std::string result;
                while (position_ < input_.size()) {
                    const char ch = input_[position_++];
                    if (ch == '"') {
                        return result;
                    }
                    if (static_cast<unsigned char>(ch) < 0x20) {
                        SetError(error, "unescaped control character in JSON string");
                        return std::nullopt;
                    }
                    if (ch != '\\') {
                        result.push_back(ch);
                        continue;
                    }

                    if (position_ >= input_.size()) {
                        break;
                    }
                    const char escaped = input_[position_++];
                    switch (escaped) {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    default:
                        SetError(error, "unsupported escape in JSON string");
                        return std::nullopt;
                    }
                }

                SetError(error, "unterminated JSON string");
                return std::nullopt;
            }

            std::optional<JsonValue> ParseStringValue(std::string* error)
            {
                auto value = ParseString(error);
                if (!value) {
                    return std::nullopt;
                }
                return JsonValue{ JsonKind::string, std::move(*value) };
            }

            std::optional<JsonValue> ParseNumberValue(std::string* error)
            {
                const auto start = position_;
                while (position_ < input_.size()) {
                    const char ch = input_[position_];
                    if ((ch >= '0' && ch <= '9') || ch == '-' || ch == '+' ||
                        ch == '.' || ch == 'e' || ch == 'E') {
                        ++position_;
                    } else {
                        break;
                    }
                }

                if (position_ == start) {
                    SetError(error, "protocol fields must be JSON strings or numbers");
                    return std::nullopt;
                }
                return JsonValue{ JsonKind::number, std::string(input_.substr(start, position_ - start)) };
            }

            std::string_view input_;
            std::size_t position_{};
        };

        const JsonValue* Required(const JsonObject& object, std::string_view name, JsonKind kind, std::string* error)
        {
            const auto it = object.find(std::string(name));
            if (it == object.end()) {
                SetError(error, "missing field: " + std::string(name));
                return nullptr;
            }
            if (it->second.kind != kind) {
                SetError(error, "wrong JSON type for field: " + std::string(name));
                return nullptr;
            }
            return &it->second;
        }

        bool ReadUnsigned(const JsonObject& object, std::string_view name, std::uint64_t& output, std::string* error)
        {
            const auto* value = Required(object, name, JsonKind::number, error);
            if (!value) {
                return false;
            }
            if (value->text.empty() || value->text.front() == '-') {
                SetError(error, std::string(name) + " must be a non-negative integer");
                return false;
            }
            const auto* begin = value->text.data();
            const auto* end = begin + value->text.size();
            const auto result = std::from_chars(begin, end, output);
            if (result.ec != std::errc{} || result.ptr != end) {
                SetError(error, std::string(name) + " must be a non-negative integer");
                return false;
            }
            return true;
        }

        bool ReadFloat(const JsonObject& object, std::string_view name, float& output, std::string* error)
        {
            const auto* value = Required(object, name, JsonKind::number, error);
            if (!value) {
                return false;
            }
            const auto* begin = value->text.data();
            const auto* end = begin + value->text.size();
            const auto result = std::from_chars(begin, end, output, std::chars_format::general);
            if (result.ec != std::errc{} || result.ptr != end || !std::isfinite(output)) {
                SetError(error, std::string(name) + " must be a finite number");
                return false;
            }
            return true;
        }

        bool HasMetadata(const JsonObject& object, std::string_view type, std::string* error)
        {
            std::uint64_t version{};
            if (!ReadUnsigned(object, "protocol_version", version, error)) {
                return false;
            }
            if (version != kProtocolVersion) {
                SetError(error, "unsupported protocol version");
                return false;
            }
            const auto* messageType = Required(object, "type", JsonKind::string, error);
            if (!messageType) {
                return false;
            }
            if (messageType->text != type) {
                SetError(error, "unexpected protocol message type");
                return false;
            }
            return true;
        }

        bool Unit(float value, std::string_view name, std::string* error)
        {
            if (!std::isfinite(value) || value < 0.0F || value > 1.0F) {
                SetError(error, std::string(name) + " must be finite and between 0 and 1");
                return false;
            }
            return true;
        }

        bool SignedUnit(float value, std::string_view name, std::string* error)
        {
            if (!std::isfinite(value) || value < -1.0F || value > 1.0F) {
                SetError(error, std::string(name) + " must be finite and between -1 and 1");
                return false;
            }
            return true;
        }

        void AppendFloat(std::string& json, float value)
        {
            char buffer[64]{};
            const auto result = std::to_chars(
                buffer, buffer + sizeof(buffer), value, std::chars_format::general,
                std::numeric_limits<float>::max_digits10);
            json.append(buffer, result.ptr);
        }

        void AppendField(std::string& json, std::string_view name, float value)
        {
            json += ",\"";
            json += name;
            json += "\":";
            AppendFloat(json, value);
        }
    }

    bool Validate(const SensoryFrame& frame, std::string* error)
    {
        if (!std::isfinite(frame.dt) || frame.dt <= 0.0F || frame.dt > 1.0F) {
            SetError(error, "dt must be finite, greater than 0, and at most 1 second");
            return false;
        }
        return
            Unit(frame.optic_flow_left, "optic_flow_left", error) &&
            Unit(frame.optic_flow_right, "optic_flow_right", error) &&
            Unit(frame.optic_flow_up, "optic_flow_up", error) &&
            Unit(frame.optic_flow_down, "optic_flow_down", error) &&
            Unit(frame.looming_left, "looming_left", error) &&
            Unit(frame.looming_center, "looming_center", error) &&
            Unit(frame.looming_right, "looming_right", error) &&
            Unit(frame.familiar_cue_left, "familiar_cue_left", error) &&
            Unit(frame.familiar_cue_center, "familiar_cue_center", error) &&
            Unit(frame.familiar_cue_right, "familiar_cue_right", error) &&
            Unit(frame.damage_signal, "damage_signal", error) &&
            Unit(frame.health_fraction, "health_fraction", error);
    }

    bool Validate(const MotorCommand& command, std::string* error)
    {
        return
            SignedUnit(command.yaw, "yaw", error) &&
            SignedUnit(command.pitch, "pitch", error) &&
            SignedUnit(command.lift, "lift", error) &&
            Unit(command.thrust, "thrust", error) &&
            Unit(command.attack_drive, "attack_drive", error);
    }

    std::optional<std::string> Serialize(const SensoryFrame& frame, std::string* error)
    {
        if (!Validate(frame, error)) {
            return std::nullopt;
        }
        std::string json = "{\"protocol_version\":2,\"type\":\"sensory_frame\",\"tick\":" + std::to_string(frame.tick);
        AppendField(json, "dt", frame.dt);
        AppendField(json, "optic_flow_left", frame.optic_flow_left);
        AppendField(json, "optic_flow_right", frame.optic_flow_right);
        AppendField(json, "optic_flow_up", frame.optic_flow_up);
        AppendField(json, "optic_flow_down", frame.optic_flow_down);
        AppendField(json, "looming_left", frame.looming_left);
        AppendField(json, "looming_center", frame.looming_center);
        AppendField(json, "looming_right", frame.looming_right);
        AppendField(json, "familiar_cue_left", frame.familiar_cue_left);
        AppendField(json, "familiar_cue_center", frame.familiar_cue_center);
        AppendField(json, "familiar_cue_right", frame.familiar_cue_right);
        AppendField(json, "damage_signal", frame.damage_signal);
        AppendField(json, "health_fraction", frame.health_fraction);
        json.push_back('}');
        return json;
    }

    std::optional<std::string> Serialize(const MotorCommand& command, std::string* error)
    {
        if (!Validate(command, error)) {
            return std::nullopt;
        }
        std::string json = "{\"protocol_version\":2,\"type\":\"motor_command\",\"tick\":" + std::to_string(command.tick);
        AppendField(json, "yaw", command.yaw);
        AppendField(json, "pitch", command.pitch);
        AppendField(json, "lift", command.lift);
        AppendField(json, "thrust", command.thrust);
        AppendField(json, "attack_drive", command.attack_drive);
        json.push_back('}');
        return json;
    }

    std::optional<SensoryFrame> ParseSensoryFrame(std::string_view json, std::string* error)
    {
        auto object = ObjectParser(json).Parse(error);
        if (!object || !HasMetadata(*object, "sensory_frame", error)) {
            return std::nullopt;
        }

        SensoryFrame frame;
        if (!ReadUnsigned(*object, "tick", frame.tick, error) ||
            !ReadFloat(*object, "dt", frame.dt, error) ||
            !ReadFloat(*object, "optic_flow_left", frame.optic_flow_left, error) ||
            !ReadFloat(*object, "optic_flow_right", frame.optic_flow_right, error) ||
            !ReadFloat(*object, "optic_flow_up", frame.optic_flow_up, error) ||
            !ReadFloat(*object, "optic_flow_down", frame.optic_flow_down, error) ||
            !ReadFloat(*object, "looming_left", frame.looming_left, error) ||
            !ReadFloat(*object, "looming_center", frame.looming_center, error) ||
            !ReadFloat(*object, "looming_right", frame.looming_right, error) ||
            !ReadFloat(*object, "familiar_cue_left", frame.familiar_cue_left, error) ||
            !ReadFloat(*object, "familiar_cue_center", frame.familiar_cue_center, error) ||
            !ReadFloat(*object, "familiar_cue_right", frame.familiar_cue_right, error) ||
            !ReadFloat(*object, "damage_signal", frame.damage_signal, error) ||
            !ReadFloat(*object, "health_fraction", frame.health_fraction, error) ||
            !Validate(frame, error)) {
            return std::nullopt;
        }
        return frame;
    }

    std::optional<MotorCommand> ParseMotorCommand(std::string_view json, std::string* error)
    {
        auto object = ObjectParser(json).Parse(error);
        if (!object || !HasMetadata(*object, "motor_command", error)) {
            return std::nullopt;
        }

        MotorCommand command;
        if (!ReadUnsigned(*object, "tick", command.tick, error) ||
            !ReadFloat(*object, "yaw", command.yaw, error) ||
            !ReadFloat(*object, "pitch", command.pitch, error) ||
            !ReadFloat(*object, "lift", command.lift, error) ||
            !ReadFloat(*object, "thrust", command.thrust, error) ||
            !ReadFloat(*object, "attack_drive", command.attack_drive, error) ||
            !Validate(command, error)) {
            return std::nullopt;
        }
        return command;
    }
}
