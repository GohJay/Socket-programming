#include "stdafx.h"
#include "Packet.h"

void Packet::MakeCreateMyCharacter(Jay::SerializationBuffer * packet, int ID, char Direction, short X, short Y, char HP)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y) + sizeof(HP);
	header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID << Direction << X << Y << HP;
}
void Packet::MakeCreateOtherCharacter(Jay::SerializationBuffer * packet, int ID, char Direction, short X, short Y, char HP)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y) + sizeof(HP);
	header.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID << Direction << X << Y << HP;
}
void Packet::MakeDeleteCharacter(Jay::SerializationBuffer * packet, int ID)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID);
	header.byType = dfPACKET_SC_DELETE_CHARACTER;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID;
}
void Packet::MakeMoveStart(Jay::SerializationBuffer * packet, int ID, char Direction, short X, short Y)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	header.byType = dfPACKET_SC_MOVE_START;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID << Direction << X << Y;
}
void Packet::MakeMoveStop(Jay::SerializationBuffer * packet, int ID, char Direction, short X, short Y)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	header.byType = dfPACKET_SC_MOVE_STOP;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID << Direction << X << Y;
}
void Packet::MakeAttack1(Jay::SerializationBuffer * packet, int ID, char Direction, short X, short Y)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	header.byType = dfPACKET_SC_ATTACK1;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID << Direction << X << Y;
}
void Packet::MakeAttack2(Jay::SerializationBuffer * packet, int ID, char Direction, short X, short Y)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	header.byType = dfPACKET_SC_ATTACK2;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID << Direction << X << Y;
}
void Packet::MakeAttack3(Jay::SerializationBuffer * packet, int ID, char Direction, short X, short Y)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID) + sizeof(Direction) + sizeof(X) + sizeof(Y);
	header.byType = dfPACKET_SC_ATTACK3;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID << Direction << X << Y;
}
void Packet::MakeSync(Jay::SerializationBuffer * packet, int ID, short X, short Y)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(ID) + sizeof(X) + sizeof(Y);
	header.byType = dfPACKET_SC_SYNC;
	packet->PutData((char*)&header, sizeof(header));
	*packet << ID << X << Y;
}
void Packet::MakeDamage(Jay::SerializationBuffer * packet, int attackID, int damageID, char damageHP)
{
	PACKET_HEADER header;
	header.byCode = dfPACKET_CODE;
	header.bySize = sizeof(attackID) + sizeof(damageID) + sizeof(damageHP);
	header.byType = dfPACKET_SC_DAMAGE;
	packet->PutData((char*)&header, sizeof(header));
	*packet << attackID << damageID << damageHP;
}
