#include "RoomManager.h"

#include "PacketStruct.h"
#include "Room.h"

#include <algorithm>
#include <limits>

std::shared_ptr<Room> RoomManager::CreateRoom()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_roomMap.size() >= static_cast<std::size_t>(MAX_ROOM_COUNT))
        return nullptr;

    const std::uint16_t roomId = GenerateRoomIdLocked();
    if (roomId == INVALID_ROOM_ID)
        return nullptr;

    auto room = std::make_shared<Room>(roomId);
    auto [it, inserted] = _roomMap.emplace(roomId, room);

    if (!inserted)
        return nullptr;

    return it->second;
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

    {
        std::lock_guard<std::mutex> lock(_mutex);

        rooms.reserve(_roomMap.size());
        for (const auto& room : _roomMap)
            rooms.push_back(room.second);
    }

    std::sort(rooms.begin(), rooms.end(), [](const auto& lhs, const auto& rhs)
    {
        return lhs->GetRoomId() < rhs->GetRoomId();
    });

    return rooms;
}

std::uint16_t RoomManager::GenerateRoomIdLocked()
{
    constexpr auto kMinRoomId = static_cast<std::uint16_t>(INVALID_ROOM_ID + 1);
    constexpr auto kMaxAttempts = static_cast<std::uint32_t>(std::numeric_limits<std::uint16_t>::max()) + 1;

    for (std::uint32_t attempts = 0; attempts < kMaxAttempts; ++attempts)
    {
        if (_nextRoomId == INVALID_ROOM_ID)
            _nextRoomId = kMinRoomId;

        const std::uint16_t candidate = _nextRoomId++;
        if (candidate == INVALID_ROOM_ID)
            continue;

        if (_roomMap.find(candidate) == _roomMap.end())
            return candidate;
    }

    return INVALID_ROOM_ID;
}
