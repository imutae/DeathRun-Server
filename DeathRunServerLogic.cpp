#include "DeathRunServerLogic.h"

#include "PacketId.h"
#include "PacketStruct.h"
#include "Room.h"
#include "RoomManager.h"
#include "Session.h"

#include <algorithm>
#include <cstring>
#include <iostream>

DeathRunServerLogic::DeathRunServerLogic()
    : _roomManager(std::make_unique<RoomManager>())
{
}

DeathRunServerLogic::~DeathRunServerLogic() = default;

void DeathRunServerLogic::OnConnected(SE::Net::Session* session)
{
    std::cout << "[Connected] Session: " << session << std::endl;

    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _sessions.push_back(session);
    }

    E_ACCEPT pkt{};
    pkt.sessionId = session->GetSessionId();
    session->Send(static_cast<uint16_t>(PacketId::E_ACCEPT), &pkt, sizeof(pkt));
}

void DeathRunServerLogic::OnDisconnected(SE::Net::Session* session)
{
    std::cout << "[Disconnected] Session: " << session << std::endl;

    LeaveRoomInternal(session);

    std::lock_guard<std::mutex> lock(_stateMutex);
    _sessions.erase(std::remove(_sessions.begin(), _sessions.end(), session), _sessions.end());
}

void DeathRunServerLogic::DispatchPacket(SE::Net::Session* session, uint16_t packetId, const char* data, int32_t len)
{
    switch (static_cast<PacketId>(packetId))
    {
    case PacketId::R_CHAT:
    {
        if (len < static_cast<int32_t>(sizeof(CHAT)))
            return;

        const CHAT* req = reinterpret_cast<const CHAT*>(data);

        CHAT out{};
        out.sessionId = session->GetSessionId();
        std::memcpy(out.message, req->message, MAX_CHAT_LENGTH);

        BroadcastLobbyPacket(static_cast<uint16_t>(PacketId::S_CHAT), &out, sizeof(out));
        break;
    }

    case PacketId::R_JOIN:
    {
        if (len < static_cast<int32_t>(sizeof(R_JOIN)))
            return;

        const R_JOIN* req = reinterpret_cast<const R_JOIN*>(data);
        const uint64_t sessionId = session->GetSessionId();

        {
            std::lock_guard<std::mutex> lock(_stateMutex);
            auto it = _sessionRoomMap.find(sessionId);
            if (it != _sessionRoomMap.end())
            {
                SendJoinResult(session, false, nullptr);
                return;
            }
        }

        std::shared_ptr<Room> room;
        if (req->roomId == INVALID_ROOM_ID)
        {
            room = _roomManager->CreateRoom();
            if (!room)
            {
                SendJoinResult(session, false, nullptr);
                return;
            }
        }
        else
        {
            room = _roomManager->GetRoom(req->roomId);
            if (!room)
            {
                SendJoinResult(session, false, nullptr);
                return;
            }
        }

        if (!room->JoinUser(session))
        {
            SendJoinResult(session, false, nullptr);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(_stateMutex);
            _sessionRoomMap[sessionId] = room->GetRoomId();
        }

        SendJoinResult(session, true, room);

        E_JOIN joinPkt{};
        joinPkt.sessionId = sessionId;

        room->BroadcastExcept(
            sessionId,
            static_cast<uint16_t>(PacketId::E_JOIN),
            &joinPkt,
            sizeof(joinPkt));

        break;
    }

    case PacketId::R_MOVE:
    {
        if (len < static_cast<int32_t>(sizeof(R_MOVE)))
            return;

        const R_MOVE* req = reinterpret_cast<const R_MOVE*>(data);

        uint16_t roomId = INVALID_ROOM_ID;
        {
            std::lock_guard<std::mutex> lock(_stateMutex);
            auto it = _sessionRoomMap.find(session->GetSessionId());
            if (it == _sessionRoomMap.end())
                return;

            roomId = it->second;
        }

        auto room = _roomManager->GetRoom(roomId);
        if (!room)
            return;

        S_MOVE movePkt{};
        movePkt.sessionId = session->GetSessionId();
        movePkt.x = req->x;
        movePkt.y = req->y;

        room->Broadcast(static_cast<uint16_t>(PacketId::S_MOVE), &movePkt, sizeof(movePkt));
        break;
    }

    case PacketId::N_LEAVE:
    {
        if (len < static_cast<int32_t>(sizeof(N_LEAVE)))
            return;

        LeaveRoomInternal(session);
        break;
    }

    case PacketId::R_ROOM_LIST:
    {
        if (len < static_cast<int32_t>(sizeof(R_ROOM_LIST)))
            return;

        auto rooms = _roomManager->GetAllRooms();

        S_ROOM_LIST pkt{};
        const size_t roomCount = std::min(rooms.size(), static_cast<size_t>(MAX_ROOM_COUNT));
        pkt.roomCount = static_cast<uint8_t>(roomCount);

        for (size_t i = 0; i < roomCount; ++i)
        {
            pkt.rooms[i].roomId = rooms[i]->GetRoomId();
            pkt.rooms[i].currentPlayers = rooms[i]->GetPlayerCount();
        }

        session->Send(static_cast<uint16_t>(PacketId::S_ROOM_LIST), &pkt, sizeof(pkt));
        break;
    }

    default:
    {
        std::cout << "[Received Unknown Packet] Session: "
            << session
            << ", PacketId: "
            << packetId
            << std::endl;
        break;
    }
    }
}

void DeathRunServerLogic::BroadcastLobbyPacket(uint16_t packetId, const void* data, int32_t len)
{
    std::vector<SE::Net::Session*> targets;

    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        targets.reserve(_sessions.size());

        for (SE::Net::Session* session : _sessions)
        {
            if (session == nullptr)
                continue;

            const uint64_t sessionId = session->GetSessionId();
            if (_sessionRoomMap.find(sessionId) != _sessionRoomMap.end())
                continue;

            targets.push_back(session);
        }
    }

    for (SE::Net::Session* session : targets)
        session->Send(packetId, data, len);
}

void DeathRunServerLogic::SendJoinResult(SE::Net::Session* session, bool success, const std::shared_ptr<Room>& room)
{
    S_JOIN pkt{};
    pkt.success = success ? 1 : 0;

    if (success && room)
    {
        auto sessionIds = room->GetPlayerSessionIds();
        const size_t count = std::min(sessionIds.size(), static_cast<size_t>(MAX_ROOM_PLAYERS));
        pkt.playerCount = static_cast<uint8_t>(count);

        for (size_t i = 0; i < count; ++i)
            pkt.sessionIds[i] = sessionIds[i];
    }

    session->Send(static_cast<uint16_t>(PacketId::S_JOIN), &pkt, sizeof(pkt));
}

bool DeathRunServerLogic::LeaveRoomInternal(SE::Net::Session* session)
{
    const uint64_t sessionId = session->GetSessionId();

    uint16_t roomId = INVALID_ROOM_ID;
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
    room->Broadcast(static_cast<uint16_t>(PacketId::E_LEAVE), &leavePkt, sizeof(leavePkt));

    if (room->GetPlayerCount() == 0)
        _roomManager->DestroyRoom(roomId);

    return true;
}