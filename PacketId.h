#pragma once
#include <cstdint>

enum class PacketId : uint16_t {
	R_CHAT = 1,		// 채팅 전송
	S_CHAT = 2,		// 서버에서 받은 채팅

	R_JOIN = 3,		// 게임 입장
	S_JOIN = 4,		// 게임 입장 시 정보 제공

	R_MOVE = 5,		// 게임 내 본인 위치 제공
	S_MOVE = 6,		// 모든 플레이어 위치 제공
		
	E_JOIN = 7,		// 다른 플레이어가 참여함
	E_LEAVE = 8,	// 다른 플레이어 나감

	N_LEAVE = 9,	// 내가 나감

	R_ROOM_LIST = 10,	// 방 목록 요청
	S_ROOM_LIST = 11,

	E_ACCEPT = 12,	// 처 서버 접속 시 나의 SessionID를 얻기 위함
};