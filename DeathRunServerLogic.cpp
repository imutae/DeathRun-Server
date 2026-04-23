#include "DeathRunServerLogic.h"
#include <iostream>
#include "Session.h"
#include "PacketId.h"
#include "PacketStruct.h"
#include "RoomManager.h"
#include "Room.h"

DeathRunServerLogic::DeathRunServerLogic()
	: _roomManager(std::make_unique<RoomManager>())
{
}

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

	auto it = _sessionRoomMap.find(session->GetSessionId());
	if (it != _sessionRoomMap.end())
	{
		uint16_t roomId = it->second;
		auto room = _roomManager->GetRoom(roomId);
		if (room)
		{
			room->QuitUser(session->GetSessionId());
			E_LEAVE leavePkt;
			leavePkt.sessionId = session->GetSessionId();
			room->Broadcast(static_cast<uint16_t>(PacketId::E_LEAVE), &leavePkt, sizeof(leavePkt));
		}
		_sessionRoomMap.erase(it);
	}
	_sessions.erase(std::remove(_sessions.begin(), _sessions.end(), session), _sessions.end());
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
		if (len < sizeof(R_JOIN))
			return;

		const R_JOIN* pkt = reinterpret_cast<const R_JOIN*>(data);
		Room* room = nullptr;

		if(pkt->roomId == INVALID_ROOM_ID)
		{
			room = _roomManager->CreateRoom();
			if(!room)
			{
				room->JoinUser(session);
				S_JOIN sJoinPkt;
				sJoinPkt.success = false;
				session->Send(static_cast<uint16_t>(PacketId::S_JOIN), &sJoinPkt, sizeof(sJoinPkt));
				_sessionRoomMap.insert({ session->GetSessionId(), room->GetRoomId() });
				return;
			}
		}
		else
		{
			S_JOIN sJoinPkt;
			room = _roomManager->GetRoom(pkt->roomId);
			if (!room)
			{
				sJoinPkt.success = false;
				session->Send(static_cast<uint16_t>(PacketId::S_JOIN), &sJoinPkt, sizeof(sJoinPkt));
				return;
			} else if(room->GetPlayerCount() >= MAX_ROOM_PLAYERS)
			{
				sJoinPkt.success = false;
				session->Send(static_cast<uint16_t>(PacketId::S_JOIN), &sJoinPkt, sizeof(sJoinPkt));
				return;
			}

			room->JoinUser(session);

			
			sJoinPkt.playerCount = room->GetPlayerCount();
			auto sessionIds = room->GetPlayerSessionIds();
			for (size_t i = 0; i < sessionIds.size() && i < MAX_ROOM_PLAYERS; ++i)
			{
				sJoinPkt.sessionIds[i] = sessionIds[i];
			}

			session->Send(static_cast<uint16_t>(PacketId::S_JOIN), &sJoinPkt, sizeof(sJoinPkt));
			_sessionRoomMap.insert({ session->GetSessionId(), room->GetRoomId() });

			E_JOIN joinPkt;
			joinPkt.sessionId = session->GetSessionId();
			room->Broadcast(static_cast<uint16_t>(PacketId::E_JOIN), &joinPkt, sizeof(joinPkt));
		}

		break;
	}
	case PacketId::R_MOVE:
	{
		if(len < sizeof(R_MOVE))
			return;

		const R_MOVE* pkt = reinterpret_cast<const R_MOVE*>(data);

		Room* room = _roomManager->GetRoom(pkt->roomId);
		if (room)
		{
			S_MOVE movePkt;
			movePkt.sessionId = session->GetSessionId();
			movePkt.x = pkt->x;
			movePkt.y = pkt->y;

			room->Broadcast(static_cast<uint16_t>(PacketId::S_MOVE), &movePkt, sizeof(S_MOVE));
		}

		break;
	}
	case PacketId::N_LEAVE:
	{
		if(len < sizeof(N_LEAVE))
			return;

		const N_LEAVE* pkt = reinterpret_cast<const N_LEAVE*>(data);
		auto it = _roomManager->GetRoom(pkt->roomId);
		if (!it)
		{
			return;
		}

		it->QuitUser(session->GetSessionId());

		E_LEAVE leavePkt;
		leavePkt.sessionId = session->GetSessionId();

		it->Broadcast(static_cast<uint16_t>(PacketId::E_LEAVE), &leavePkt, sizeof(leavePkt));

		break;
	}
	case PacketId::R_ROOM_LIST:
	{
		if(len < sizeof(R_ROOM_LIST))
			return;

		auto rooms = _roomManager->GetAllRooms();
		S_ROOM_LIST pkt;

		pkt.roomCount = static_cast<std::uint8_t>(rooms.size());
		for (size_t i = 0; i < rooms.size() && i < MAX_ROOM_COUNT; ++i)
		{
			pkt.rooms[i].roomId = rooms[i]->GetRoomId();
			pkt.rooms[i].currentPlayers = rooms[i]->GetPlayerCount();
		}

		session->Send(static_cast<uint16_t>(PacketId::S_ROOM_LIST), &pkt, sizeof(pkt));
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
