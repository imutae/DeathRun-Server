#include "DeathRunServerLogic.h"
#include <iostream>
#include "Session.h"
#include "PacketId.h"
#include "PacketStruct.h"

void DeathRunServerLogic::OnConnected(SE::Net::Session* session)
{	
	std::cout << "[Connected] Session: " << session << std::endl;
	_sessions.push_back(session);
}

void DeathRunServerLogic::OnDisconnected(SE::Net::Session* session)
{
	std::cout << "[Disconnected] Session: " << session << std::endl;
}

void DeathRunServerLogic::DispatchPacket(SE::Net::Session* session, uint16_t packetId, const char* data, int32_t len)
{
	switch (static_cast<PacketId>(packetId))
	{
	case PacketId::R_CHAT:
	{
		if (len < sizeof(ChatPacket))
			return;

		const ChatPacket* pkt = reinterpret_cast<const ChatPacket*>(data);
		std::cout << pkt->userName << " : " << pkt->message << std::endl;

		BroadcastPacket(static_cast<uint16_t>(PacketId::S_CHAT), pkt, sizeof(ChatPacket));
		break;
	}
	default:
	{
		std::cout << "[Received Unknown Packet] Session: " << session << ", PacketId: " << packetId << std::endl;
		break;
	}
	}
}

void DeathRunServerLogic::BroadcastPacket(uint16_t packetId, const void* data, int32_t len)
{
	for (SE::Net::Session* session : _sessions)
	{
		session->Send(packetId, data, len);
	}
}
