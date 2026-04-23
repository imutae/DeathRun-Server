#pragma once
#include <cstdint>

enum class PacketId : uint16_t {
	R_CHAT = 1,
	S_CHAT = 2,
	R_PLAY = 3,
	S_PLAY = 4,
	R_MOVE = 5,
	S_MOVE = 6,
	E_JOIN = 7,
};