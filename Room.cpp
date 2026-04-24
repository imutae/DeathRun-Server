#include "Room.h"

#include "PacketStruct.h"
#include "Session.h"

namespace
{
    using Session = SE::Net::Session;

    void SendToTargets(const std::vector<Session*>& targets, std::uint16_t packetId, const void* data, std::int32_t len)
    {
        for (Session* session : targets)
        {
            if (session == nullptr)
                continue;

            session->Send(packetId, data, len);
        }
    }
}

Room::Room(std::uint16_t roomId)
    : _roomId(roomId)
{
}

bool Room::JoinUser(SE::Net::Session* userSession)
{
    if (userSession == nullptr)
        return false;

    std::lock_guard<std::mutex> lock(_mutex);

    if (_userMap.size() >= static_cast<std::size_t>(MAX_ROOM_PLAYERS))
        return false;

    return _userMap.emplace(userSession->GetSessionId(), userSession).second;
}

void Room::QuitUser(std::uint64_t userSessionId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _userMap.erase(userSessionId);
}

void Room::Broadcast(std::uint16_t packetId, const void* data, std::int32_t len) const
{
    SendToTargets(GetSessionSnapshot(0, false), packetId, data, len);
}

void Room::BroadcastExcept(std::uint64_t exceptSessionId, std::uint16_t packetId, const void* data, std::int32_t len) const
{
    SendToTargets(GetSessionSnapshot(exceptSessionId, true), packetId, data, len);
}

std::uint8_t Room::GetPlayerCount() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return static_cast<std::uint8_t>(_userMap.size());
}

std::vector<std::uint64_t> Room::GetPlayerSessionIds() const
{
    std::vector<std::uint64_t> sessionIds;

    {
        std::lock_guard<std::mutex> lock(_mutex);

        sessionIds.reserve(_userMap.size());
        for (const auto& user : _userMap)
            sessionIds.push_back(user.first);
    }

    return sessionIds;
}

std::vector<SE::Net::Session*> Room::GetSessionSnapshot(std::uint64_t exceptSessionId, bool excludeSession) const
{
    std::vector<SE::Net::Session*> sessions;

    {
        std::lock_guard<std::mutex> lock(_mutex);

        sessions.reserve(_userMap.size());
        for (const auto& user : _userMap)
        {
            if (excludeSession && user.first == exceptSessionId)
                continue;

            sessions.push_back(user.second);
        }
    }

    return sessions;
}
