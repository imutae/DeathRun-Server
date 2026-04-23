#include "Room.h"
#include "Session.h"

Room::Room(uint16_t roomId) : _roomId(roomId)
{
}

void Room::JoinUser(SE::Net::Session* userSession)
{
	_userMap.emplace(userSession->GetSessionId(), userSession);
}

void Room::QuitUser(uint64_t userSessionId)
{
	auto it = _userMap.find(userSessionId);
	if (it != _userMap.end())
	{
		_userMap.erase(it);
	}
}

void Room::Broadcast(uint16_t packetId, const void* data, int32_t len)
{
	for (const auto& [sessionId, session] : _userMap)
	{
		session->Send(packetId, data, len);
	}
}

std::vector<uint64_t> Room::GetPlayerSessionIds() const
{
	std::vector<uint64_t> sessionIds;
	for (const auto& [sessionId, session] : _userMap)
	{
		sessionIds.push_back(sessionId);
	}
	return sessionIds;
}
