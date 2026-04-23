#include "RoomManager.h"
#include "PacketStruct.h"
#include "Room.h"

std::shared_ptr<Room> RoomManager::CreateRoom()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_roomMap.size() >= MAX_ROOM_COUNT)
        return nullptr;

    const std::uint16_t roomId = _roomIdGenerator.fetch_add(1);

    auto room = std::make_shared<Room>(roomId);
    _roomMap.emplace(roomId, room);

    return room;
}

void RoomManager::DestroyRoom(std::uint16_t roomId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _roomMap.erase(roomId);
}

std::shared_ptr<Room> RoomManager::GetRoom(std::uint16_t roomId) const
{
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _roomMap.find(roomId);
    if (it == _roomMap.end())
        return nullptr;

    return it->second;
}

std::vector<std::shared_ptr<Room>> RoomManager::GetAllRooms() const
{
    std::vector<std::shared_ptr<Room>> rooms;

    std::lock_guard<std::mutex> lock(_mutex);
    rooms.reserve(_roomMap.size());

    for (const auto& [roomId, room] : _roomMap)
        rooms.push_back(room);

    return rooms;
}