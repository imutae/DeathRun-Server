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
    explicit Room(uint16_t roomId);

public:
    bool JoinUser(SE::Net::Session* userSession);
    void QuitUser(uint64_t userSessionId);

public:
    void Broadcast(uint16_t packetId, const void* data, int32_t len) const;
    void BroadcastExcept(uint64_t exceptSessionId, uint16_t packetId, const void* data, int32_t len) const;

public:
    uint16_t GetRoomId() const { return _roomId; }
    uint8_t GetPlayerCount() const;
    std::vector<uint64_t> GetPlayerSessionIds() const;

private:
    uint16_t _roomId;
    mutable std::mutex _mutex;
    std::unordered_map<uint64_t, SE::Net::Session*> _userMap;
};