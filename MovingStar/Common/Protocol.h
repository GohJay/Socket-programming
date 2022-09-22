#ifndef __PROTOCOL__H_
#define __PROTOCOL__H_

#pragma pack(push, 1)
enum PACKET_TYPE
{
	ALLOC = 0,
	CREATE,
	DESTROY,
	MOVE
};
struct STAR_ALLOC_PACKET
{
	PACKET_TYPE type;
	int id;
	int reserve[2];
};
struct STAR_CREATE_PACKET
{
	PACKET_TYPE type;
	int id;
	int x;
	int y;
};
struct STAR_DESTROY_PACKET
{
	PACKET_TYPE type;
	int id;
	int reserve[2];
};
struct STAR_MOVE_PACKET
{
	PACKET_TYPE type;
	int id;
	int x;
	int y;
};
#pragma pack(pop)

#endif