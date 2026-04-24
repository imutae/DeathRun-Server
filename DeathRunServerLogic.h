#pragma once

#include "IServerLogic.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class Room;
class RoomManager;
struct CHAT;
struct R_JOIN;
struct R_MOVE;

class DeathRunServerLogic : public SE::IServerLogic
{
public:
    DeathRunServerLogic();
    ~DeathRunServerLogic() override;

    DeathRunServerLogic(const DeathRunServerLogic&) = delete;
    DeathRunServerLogic& operator=(const DeathRunServerLogic&) = delete;
    DeathRunServerLogic(DeathRunServerLogic&&) = delete;
    DeathRunServerLogic& operator=(DeathRunServerLogic&&) = delete;

    void OnConnected(SE::Net::Session* session) override;
    void OnDisconnected(SE::Net::Session* session) override;
    void DispatchPacket(SE::Net::Session* session, std::uint16_t packetId, const char* data, std::int32_t len) override;

private:
    using Session = SE::Net::Session;
    using RoomPtr = std::shared_ptr<Room>;

    void HandleChat(Session* session, const CHAT& req);
    void HandleJoin(Session* session, const R_JOIN& req);
    void HandleMove(Session* session, const R_MOVE& req);
    void HandleRoomList(Session* session);

    void BroadcastLobbyPacket(std::uint16_t packetId, const void* data, std::int32_t len);
    void SendJoinResult(Session* session, bool success, const RoomPtr& room);
    bool LeaveRoomInternal(Session* session);

    bool IsSessionInRoom(std::uint64_t sessionId) const;
    bool TryRegisterSessionRoom(std::uint64_t sessionId, std::uint16_t roomId);
    bool TryGetSessionRoomId(std::uint64_t sessionId, std::uint16_t& roomId) const;
    void RemoveSession(Session* session);

private:
    mutable std::mutex _stateMutex;
    std::vector<Session*> _sessions;
    std::unique_ptr<RoomManager> _roomManager;
    std::unordered_map<std::uint64_t, std::uint16_t> _sessionRoomMap;
};
