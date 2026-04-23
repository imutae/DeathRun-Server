#pragma once
#include "IServerLogic.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class Room;
class RoomManager;

class DeathRunServerLogic : public SE::IServerLogic
{
public:
    DeathRunServerLogic();
    ~DeathRunServerLogic();

public:
    void OnConnected(SE::Net::Session* session) override;
    void OnDisconnected(SE::Net::Session* session) override;
    void DispatchPacket(SE::Net::Session* session, uint16_t packetId, const char* data, int32_t len) override;

private:
    void BroadcastLobbyPacket(uint16_t packetId, const void* data, int32_t len);
    void SendJoinResult(SE::Net::Session* session, bool success, const std::shared_ptr<Room>& room);
    bool LeaveRoomInternal(SE::Net::Session* session);

private:
    mutable std::mutex _stateMutex;
    std::vector<SE::Net::Session*> _sessions;
    std::unique_ptr<RoomManager> _roomManager;
    std::unordered_map<uint64_t, uint16_t> _sessionRoomMap;
};