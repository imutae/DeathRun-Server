#pragma once
#include <cstdint>

constexpr std::uint16_t INVALID_ROOM_ID = 0;
constexpr std::uint8_t MAX_ROOM_PLAYERS = 8;
constexpr std::uint8_t MAX_ROOM_COUNT = 32;
constexpr int MAX_CHAT_LENGTH = 256;

#pragma pack(push, 1)

struct ROOM_INFO
{
    std::uint16_t roomId;
    std::uint8_t currentPlayers;
};

struct CHAT
{
    std::uint64_t sessionId;
    char message[MAX_CHAT_LENGTH];
};

struct R_JOIN
{
    // roomId == 0 : 방 생성
    // roomId != 0 : 해당 방 참가
    std::uint16_t roomId;
};

struct S_JOIN
{
    std::uint8_t success; // 0: 실패, 1: 성공
    std::uint8_t playerCount;
    std::uint64_t sessionIds[MAX_ROOM_PLAYERS];
};

struct R_MOVE
{
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
    std::uint8_t reserved;
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