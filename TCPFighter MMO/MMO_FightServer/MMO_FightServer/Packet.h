#pragma once
#include "../Network/include/SerializationBuffer.h"
#include "../Common/Protocol.h"

class Packet
{
public:
	static void MakeCreateMyCharacter(Jay::SerializationBuffer* packet, int ID, char Direction, short X, short Y, char HP);
	static void MakeCreateOtherCharacter(Jay::SerializationBuffer* packet, int ID, char Direction, short X, short Y, char HP);
	static void MakeDeleteCharacter(Jay::SerializationBuffer* packet, int ID);
	static void MakeMoveStart(Jay::SerializationBuffer* packet, int ID, char Direction, short X, short Y);
	static void MakeMoveStop(Jay::SerializationBuffer* packet, int ID, char Direction, short X, short Y);
	static void MakeAttack1(Jay::SerializationBuffer* packet, int ID, char Direction, short X, short Y);
	static void MakeAttack2(Jay::SerializationBuffer* packet, int ID, char Direction, short X, short Y);
	static void MakeAttack3(Jay::SerializationBuffer* packet, int ID, char Direction, short X, short Y);
	static void MakeDamage(Jay::SerializationBuffer* packet, int attackID, int damageID, char damageHP);
	static void MakeSync(Jay::SerializationBuffer* packet, int ID, short X, short Y);
	static void MakeEcho(Jay::SerializationBuffer* packet, int time);
};
