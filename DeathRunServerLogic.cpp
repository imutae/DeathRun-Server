#include "DeathRunServerLogic.h"

#define NOMINMAX

#include "PacketId.h"
#include "PacketStruct.h"
#include "Room.h"
#include "RoomManager.h"
#include "Session.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>

namespace
{
    std::uint16_t ToPacketId(PacketId packetId)
    {
        return static_cast<std::uint16_t>(packetId);
    }

    template <typename Packet>
    std::optional<Packet> ReadPacket(const char* data, std::int32_t len)
    {
        if (data == nullptr || len != static_cast<std::int32_t>(sizeof(Packet)))
            return std::nullopt;

        Packet packet{};
        std::memcpy(&packet, data, sizeof(Packet));
        return packet;
    }
}

DeathRunServerLogic::DeathRunServerLogic()
    : _roomManager(std::make_unique<RoomManager>())
{
}

DeathRunServerLogic::~DeathRunServerLogic() = default;

void DeathRunServerLogic::OnConnected(Session* session)
{
    if (session == nullptr)
        return;

	std::cout << "[Connected] Session: " << session->GetSessionId() << "\n";

    {
        std::lock_guard<std::mutex> lock(_stateMutex);

        if (std::find(_sessions.begin(), _sessions.end(), session) == _sessions.end())
            _sessions.push_back(session);
    }

    E_ACCEPT pkt{};
    pkt.sessionId = session->GetSessionId();
    session->Send(ToPacketId(PacketId::E_ACCEPT), &pkt, sizeof(pkt));
}

void DeathRunServerLogic::OnDisconnected(SE::Net::Session* session)
{
    if (session == nullptr)
        return;

    std::cout << "[Disconnected] Session: " << session->GetSessionId() << "\n";

    LeaveRoomInternal(session);
    RemoveSession(session);
}

void DeathRunServerLogic::DispatchPacket(Session* session, std::uint16_t packetId, const char* data, std::int32_t len)
{
    if (session == nullptr)
        return;

	PacketId pid = static_cast<PacketId>(packetId);

	std::cout << "[Received Packet] SessionId: "
		<< session->GetSessionId()
		<< ", PacketId: "
		<< packetId
		<< ", BodyLen: "
		<< len
		<< '\n';

    switch (pid)
    {
    case PacketId::R_CHAT:
    {
        auto req = ReadPacket<CHAT>(data, len);
        if (!req)
            return;

        HandleChat(session, *req);
        return;
    }
    case PacketId::R_JOIN:
    {
        auto req = ReadPacket<R_JOIN>(data, len);
        if (!req)
            return;

        HandleJoin(session, *req);
        return;
    }
    case PacketId::R_MOVE:
    {
        auto req = ReadPacket<R_MOVE>(data, len);
        if (!req)
            return;

        HandleMove(session, *req);
        return;
    }
    case PacketId::N_LEAVE:
    {
        auto req = ReadPacket<N_LEAVE>(data, len);
        if (!req)
            return;

        LeaveRoomInternal(session);
        return;
    }
    case PacketId::R_ROOM_LIST:
    {
        auto req = ReadPacket<R_ROOM_LIST>(data, len);
        if (!req)
            return;

        HandleRoomList(session);
        return;
    }
    default:
    {
		std::cout << "[Unknown Packet] SessionId: "
			<< session->GetSessionId()
			<< ", PacketId: "
			<< packetId
			<< ", BodyLen: "
			<< len
			<< '\n';
        return;
    }
    }
}

void DeathRunServerLogic::HandleChat(Session* session, const CHAT& req)
{
    CHAT out{};
    out.sessionId = session->GetSessionId();
    std::memcpy(out.message, req.message, sizeof(out.message));

    BroadcastLobbyPacket(ToPacketId(PacketId::S_CHAT), &out, sizeof(out));
}

void DeathRunServerLogic::HandleJoin(Session* session, const R_JOIN& req)
{
    const std::uint64_t sessionId = session->GetSessionId();

    if (IsSessionInRoom(sessionId))
    {
        SendJoinResult(session, false, nullptr);
        return;
    }

    const bool createRoom = (req.roomId == INVALID_ROOM_ID);
	std::shared_ptr<Room> room = createRoom ? _roomManager->CreateRoom() : _roomManager->GetRoom(req.roomId);

    if (!room)
    {
        SendJoinResult(session, false, nullptr);
        return;
    }

    if (!room->JoinUser(session))
    {
        if (createRoom && room->GetPlayerCount() == 0)
            _roomManager->DestroyRoom(room->GetRoomId());

        SendJoinResult(session, false, nullptr);
        return;
    }

    if (!TryRegisterSessionRoom(sessionId, room->GetRoomId()))
    {
        room->QuitUser(sessionId);

        if (createRoom && room->GetPlayerCount() == 0)
            _roomManager->DestroyRoom(room->GetRoomId());

        SendJoinResult(session, false, nullptr);
        return;
    }

    SendJoinResult(session, true, room);

    E_JOIN joinPkt{};
    joinPkt.sessionId = sessionId;

    room->BroadcastExcept(
        sessionId,
        ToPacketId(PacketId::E_JOIN),
        &joinPkt,
        sizeof(joinPkt));
}

void DeathRunServerLogic::HandleMove(Session* session, const R_MOVE& req)
{
    const std::uint64_t sessionId = session->GetSessionId();

    std::uint16_t roomId = INVALID_ROOM_ID;
    if (!TryGetSessionRoomId(sessionId, roomId))
        return;

    auto room = _roomManager->GetRoom(roomId);
    if (!room)
        return;

    S_MOVE movePkt{};
    movePkt.sessionId = sessionId;
    movePkt.x = req.x;
    movePkt.y = req.y;

    room->Broadcast(ToPacketId(PacketId::S_MOVE), &movePkt, sizeof(movePkt));
}

void DeathRunServerLogic::HandleRoomList(Session* session)
{
    auto rooms = _roomManager->GetAllRooms();

    S_ROOM_LIST pkt{};
    const std::size_t roomCount = std::min(rooms.size(), static_cast<std::size_t>(MAX_ROOM_COUNT));
    pkt.roomCount = static_cast<std::uint8_t>(roomCount);

    for (std::size_t i = 0; i < roomCount; ++i)
    {
        pkt.rooms[i].roomId = rooms[i]->GetRoomId();
        pkt.rooms[i].currentPlayers = rooms[i]->GetPlayerCount();
    }

    session->Send(ToPacketId(PacketId::S_ROOM_LIST), &pkt, sizeof(pkt));
}

void DeathRunServerLogic::BroadcastLobbyPacket(std::uint16_t packetId, const void* data, std::int32_t len)
{
    std::vector<Session*> targets;

    {
        std::lock_guard<std::mutex> lock(_stateMutex);

        targets.reserve(_sessions.size());
        for (Session* session : _sessions)
        {
            if (session == nullptr)
                continue;

            const std::uint64_t sessionId = session->GetSessionId();
            if (_sessionRoomMap.find(sessionId) != _sessionRoomMap.end())
                continue;

            targets.push_back(session);
        }
    }

    for (Session* session : targets)
        session->Send(packetId, data, len);
}

void DeathRunServerLogic::SendJoinResult(Session* session, bool success, const std::shared_ptr<Room>& room)
{
    if (session == nullptr)
        return;

    S_JOIN pkt{};
    pkt.success = success ? 1 : 0;

    if (success && room)
    {
        auto sessionIds = room->GetPlayerSessionIds();
        const std::size_t count = std::min(sessionIds.size(), static_cast<std::size_t>(MAX_ROOM_PLAYERS));
        pkt.playerCount = static_cast<std::uint8_t>(count);

        for (std::size_t i = 0; i < count; ++i)
            pkt.sessionIds[i] = sessionIds[i];
    }

    session->Send(ToPacketId(PacketId::S_JOIN), &pkt, sizeof(pkt));
}

bool DeathRunServerLogic::LeaveRoomInternal(Session* session)
{
    if (session == nullptr)
        return false;

    const std::uint64_t sessionId = session->GetSessionId();
    std::uint16_t roomId = INVALID_ROOM_ID;

    {
        std::lock_guard<std::mutex> lock(_stateMutex);

        auto it = _sessionRoomMap.find(sessionId);
        if (it == _sessionRoomMap.end())
            return false;

        roomId = it->second;
        _sessionRoomMap.erase(it);
    }

    auto room = _roomManager->GetRoom(roomId);
    if (!room)
        return false;

    room->QuitUser(sessionId);

    E_LEAVE leavePkt{};
    leavePkt.sessionId = sessionId;
    room->Broadcast(ToPacketId(PacketId::E_LEAVE), &leavePkt, sizeof(leavePkt));

    if (room->GetPlayerCount() == 0)
        _roomManager->DestroyRoom(roomId);

    return true;
}

bool DeathRunServerLogic::IsSessionInRoom(std::uint64_t sessionId) const
{
    std::lock_guard<std::mutex> lock(_stateMutex);
    return _sessionRoomMap.find(sessionId) != _sessionRoomMap.end();
}

bool DeathRunServerLogic::TryRegisterSessionRoom(std::uint64_t sessionId, std::uint16_t roomId)
{
    std::lock_guard<std::mutex> lock(_stateMutex);
    return _sessionRoomMap.emplace(sessionId, roomId).second;
}

bool DeathRunServerLogic::TryGetSessionRoomId(std::uint64_t sessionId, std::uint16_t& roomId) const
{
    std::lock_guard<std::mutex> lock(_stateMutex);

    auto it = _sessionRoomMap.find(sessionId);
    if (it == _sessionRoomMap.end())
        return false;

    roomId = it->second;
    return true;
}

void DeathRunServerLogic::RemoveSession(Session* session)
{
    std::lock_guard<std::mutex> lock(_stateMutex);
    _sessions.erase(std::remove(_sessions.begin(), _sessions.end(), session), _sessions.end());
}
