#pragma once
#include <unordered_map>

namespace SE::Net {
	class Session;
}

class Room
{
public:
	Room(uint16_t roomId);

public:
	void JoinUser(SE::Net::Session* userSession);
	void QuitUser(uint64_t userSessionId);

public:
	void Broadcast(uint16_t packetId, const void* data, int32_t len);

public:
	uint16_t GetRoomId() const { return _roomId; }
	uint8_t GetPlayerCount() const { return static_cast<uint8_t>(_userMap.size()); }
	std::vector<uint64_t> GetPlayerSessionIds() const;

private:
	uint16_t _roomId;
	std::unordered_map<uint64_t, SE::Net::Session*> _userMap;
};

