#include "RoomManager.h"
#include "Room.h"

RoomManager::RoomManager()
{
}

RoomManager::~RoomManager()
{
	for (auto& [roomId, room] : _roomMap)
	{
		delete room;
	}
	_roomMap.clear();
}

Room* RoomManager::CreateRoom()
{
	if(_roomMap.size() >= 32)
	{
		return nullptr; // 최대 방 수 초과
	}

	Room* newRoom = new Room(_roomIdGenerator.fetch_add(1));
	_roomMap.emplace(newRoom->GetRoomId(), newRoom);
    return newRoom;
}

void RoomManager::DestroyRoom(std::uint16_t roomId)
{
	auto it = _roomMap.find(roomId);
	if (it != _roomMap.end())
	{
		delete it->second;
		_roomMap.erase(it);
	}
}

Room* RoomManager::GetRoom(std::uint16_t roomId) const
{
	auto it = _roomMap.find(roomId);
	if (it != _roomMap.end())
	{
		return it->second;
	}
	return nullptr;
}

std::vector<Room*> RoomManager::GetAllRooms() const
{
	std::vector<Room*> rooms;
	for (const auto& [roomId, room] : _roomMap)
	{
		rooms.push_back(room);
	}
	return rooms;
}
