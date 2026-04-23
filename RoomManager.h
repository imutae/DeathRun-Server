#pragma once
#include <unordered_map>
#include <atomic>

class Room;

class RoomManager
{
public:
	RoomManager();
	~RoomManager();

	RoomManager(const RoomManager&) = delete;
	RoomManager& operator=(const RoomManager&) = delete;
	RoomManager(RoomManager&&) = delete;
	RoomManager& operator=(RoomManager&&) = delete;

public:
	Room* CreateRoom();
	void DestroyRoom(std::uint16_t roomId);
	Room* GetRoom(std::uint16_t roomId) const;
	std::vector<Room*> GetAllRooms() const;

private:
	std::unordered_map<std::uint16_t, Room*> _roomMap;
	std::atomic<std::uint16_t> _roomIdGenerator = 1;
};

