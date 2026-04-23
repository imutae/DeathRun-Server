#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class Room;

class RoomManager
{
public:
    RoomManager() = default;
    ~RoomManager() = default;

    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;
    RoomManager(RoomManager&&) = delete;
    RoomManager& operator=(RoomManager&&) = delete;

public:
    std::shared_ptr<Room> CreateRoom();
    void DestroyRoom(std::uint16_t roomId);
    std::shared_ptr<Room> GetRoom(std::uint16_t roomId) const;
    std::vector<std::shared_ptr<Room>> GetAllRooms() const;

private:
    mutable std::mutex _mutex;
    std::unordered_map<std::uint16_t, std::shared_ptr<Room>> _roomMap;
    std::atomic<std::uint16_t> _roomIdGenerator{ 1 };
};