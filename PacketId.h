#pragma once
#include <cstdint>

enum class PacketId : std::uint16_t
{
    R_CHAT = 1,      
    S_CHAT = 2,      

    R_JOIN = 3,      
    S_JOIN = 4,      

    R_MOVE = 5,      
    S_MOVE = 6,      

    E_JOIN = 7,      
    E_LEAVE = 8,     

	N_LEAVE = 9,     

    R_ROOM_LIST = 10,
    S_ROOM_LIST = 11,

    E_ACCEPT = 12,   
};
