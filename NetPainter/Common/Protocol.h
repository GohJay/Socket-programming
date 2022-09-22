#ifndef __PROTOCOL__H_
#define __PROTOCOL__H_

#pragma pack(push, 1)
struct stHEADER
{
	unsigned short Len;
};
struct st_DRAW_PACKET
{
	int	iStartX;
	int	iStartY;
	int	iEndX;
	int	iEndY;
};
#pragma pack(pop)

#endif