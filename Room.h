#pragma once

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace SE::Net
{
    class Session;
}

class Room
{
public:
    explicit Room(std::uint16_t roomId);

    bool JoinUser(SE::Net::Session* userSession);
    void QuitUser(std::uint64_t userSessionId);

    void Broadcast(std::uint16_t packetId, const void* data, std::int32_t len) const;
    void BroadcastExcept(std::uint64_t exceptSessionId, std::uint16_t packetId, const void* data, std::int32_t len) const;

    std::uint16_t GetRoomId() const { return _roomId; }
    std::uint8_t GetPlayerCount() const;
    std::vector<std::uint64_t> GetPlayerSessionIds() const;

private:
    std::vector<SE::Net::Session*> GetSessionSnapshot(std::uint64_t exceptSessionId, bool excludeSession) const;

private:
    std::uint16_t _roomId;
    mutable std::mutex _mutex;
    std::unordered_map<std::uint64_t, SE::Net::Session*> _userMap;
};
