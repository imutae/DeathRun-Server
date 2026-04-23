#include "Room.h"
#include "PacketStruct.h"
#include "Session.h"

Room::Room(uint16_t roomId)
    : _roomId(roomId)
{
}

bool Room::JoinUser(SE::Net::Session* userSession)
{
    if (userSession == nullptr)
        return false;

    std::lock_guard<std::mutex> lock(_mutex);

    if (_userMap.size() >= MAX_ROOM_PLAYERS)
        return false;

    return _userMap.emplace(userSession->GetSessionId(), userSession).second;
}

void Room::QuitUser(uint64_t userSessionId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _userMap.erase(userSessionId);
}

void Room::Broadcast(uint16_t packetId, const void* data, int32_t len) const
{
    std::vector<SE::Net::Session*> targets;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        targets.reserve(_userMap.size());

        for (const auto& [sessionId, session] : _userMap)
            targets.push_back(session);
    }

    for (SE::Net::Session* session : targets)
    {
        if (session)
            session->Send(packetId, data, len);
    }
}

void Room::BroadcastExcept(uint64_t exceptSessionId, uint16_t packetId, const void* data, int32_t len) const
{
    std::vector<SE::Net::Session*> targets;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        targets.reserve(_userMap.size());

        for (const auto& [sessionId, session] : _userMap)
        {
            if (sessionId == exceptSessionId)
                continue;

            targets.push_back(session);
        }
    }

    for (SE::Net::Session* session : targets)
    {
        if (session)
            session->Send(packetId, data, len);
    }
}

uint8_t Room::GetPlayerCount() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return static_cast<uint8_t>(_userMap.size());
}

std::vector<uint64_t> Room::GetPlayerSessionIds() const
{
    std::vector<uint64_t> sessionIds;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        sessionIds.reserve(_userMap.size());

        for (const auto& [sessionId, session] : _userMap)
            sessionIds.push_back(sessionId);
    }

    return sessionIds;
}