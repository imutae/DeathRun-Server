#pragma once
#include <cstdint>

enum class PacketId : uint16_t
{
    R_CHAT = 1,      // 로비 채팅 전송
    S_CHAT = 2,      // 로비 채팅 수신

    R_JOIN = 3,      // 방 생성 / 방 참가 요청
    S_JOIN = 4,      // 방 생성 / 참가 결과 + 현재 방 플레이어 정보

    R_MOVE = 5,      // 내 위치 전송
    S_MOVE = 6,      // 같은 방 플레이어들에게 위치 전송

    E_JOIN = 7,      // 같은 방의 다른 플레이어가 들어옴
    E_LEAVE = 8,     // 같은 방의 다른 플레이어가 나감

	N_LEAVE = 9,     // 클라이언트가 방에서 나감을 알림 (방 나가기 통보)

    R_ROOM_LIST = 10, // 방 목록 요청
    S_ROOM_LIST = 11, // 방 목록 응답

    E_ACCEPT = 12,   // 접속 직후 내 SessionId 전달
};
