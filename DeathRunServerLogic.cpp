#include "DeathRunServerLogic.h"
#include <iostream>
#include "Session.h"
#include "PacketId.h"
#include "PacketStruct.h"

void DeathRunServerLogic::OnConnected(SE::Net::Session* session)
{	
	std::cout << "[Connected] Session: " << session << std::endl;
	_sessions.push_back(session);

	E_ACCEPT pkt;
	pkt.sessionId = session->GetSessionId();
	session->Send(static_cast<uint16_t>(PacketId::E_ACCEPT), &pkt, sizeof(pkt));
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
		if (len < sizeof(CHAT))
			return;

		const CHAT* pkt = reinterpret_cast<const CHAT*>(data);
		BroadcastPacket(static_cast<uint16_t>(PacketId::S_CHAT), pkt, sizeof(CHAT));
		break;
	}
	case PacketId::R_JOIN:
	{
		break;
	}
	case PacketId::R_MOVE:
	{
		break;
	}
	case PacketId::N_LEAVE:
	{
		break;
	}
	case PacketId::R_ROOM_LIST:
	{
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
