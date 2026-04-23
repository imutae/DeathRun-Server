#pragma once
#include <cstdint>

constexpr std::uint16_t INVALID_ROOM_ID = 0;   // 0이면 새 방 생성
constexpr std::uint8_t  MAX_ROOM_PLAYERS = 8;   // 고정
constexpr std::uint8_t  MAX_ROOM_COUNT = 32;
constexpr std::uint16_t MAX_CHAT_LENGTH = 256;

#pragma pack(push, 1)

struct ROOM_INFO
{
    std::uint16_t roomId;
    std::uint8_t currentPlayers;
};

struct CHAT
{
    // 로비 채팅 전용
    std::uint64_t sessionId;
    char message[MAX_CHAT_LENGTH];
};

struct R_JOIN
{
    // roomId == 0 이면 방 생성
    // roomId != 0 이면 해당 방 참가
    std::uint16_t roomId;
};

struct S_JOIN
{
	bool joined;    // true: 참가 성공, false: 참가 실패 (방이 가득 찼거나 존재하지 않음)
    std::uint8_t playerCount;
    std::uint64_t sessionIds[MAX_ROOM_PLAYERS];
};

struct R_MOVE
{
	std::uint16_t roomId;
    float x;
    float y;
};

struct S_MOVE
{
    std::uint64_t sessionId;
    float x;
    float y;
};

struct E_JOIN
{
    std::uint64_t sessionId;
};

struct E_LEAVE
{
    std::uint64_t sessionId;
};

struct N_LEAVE
{
    std::uint16_t roomId;
};

struct R_ROOM_LIST
{
    std::uint8_t reserved;
};

struct S_ROOM_LIST
{
    std::uint8_t roomCount;
    ROOM_INFO rooms[MAX_ROOM_COUNT];
};

struct E_ACCEPT
{
    std::uint64_t sessionId;
};

#pragma pack(pop)