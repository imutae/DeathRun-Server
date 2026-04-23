#pragma once
#include "IServerLogic.h"
#include <vector>
#include <memory>

class RoomManager;

class DeathRunServerLogic : public SE::IServerLogic
{
public:
	DeathRunServerLogic();

public:
    void OnConnected(SE::Net::Session* session) override;

    void OnDisconnected(SE::Net::Session* session) override;

    void DispatchPacket(SE::Net::Session* session, uint16_t packetId, const char* data, int32_t len) override;

private:
	void BroadcastPacket(uint16_t packetId, const void* data, int32_t len);

private:
	std::vector<SE::Net::Session*> _sessions;
	std::unique_ptr<RoomManager> _roomManager;
	std::unordered_map<uint64_t, uint16_t> _sessionRoomMap; // sessionId -> roomId
};

