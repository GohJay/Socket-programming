#include "stdafx.h"
#include "ScreenBuffer.h"
#include <memory.h>

ScreenBuffer ScreenBuffer::_instance;
ScreenBuffer::ScreenBuffer()
{
	cs_Initial();
}
ScreenBuffer::~ScreenBuffer()
{
}
ScreenBuffer * ScreenBuffer::GetInstance()
{
	return &_instance;
}
void ScreenBuffer::Buffer_Flip(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		cs_MoveCursor(0, iCnt);
		printf_s(_screenBuffer[iCnt]);
	}
}
void ScreenBuffer::Buffer_Clear(void)
{
	int iCnt;
	memset(_screenBuffer, ' ', dfSCREEN_WIDTH * dfSCREEN_HEIGHT);
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		_screenBuffer[iCnt][dfSCREEN_WIDTH - 1] = '\0';
	}
}
void ScreenBuffer::Sprite_Draw(int iX, int iY, char chSprite)
{
	if (IsOutOfScreen(iX, iY))
		return;

	_screenBuffer[iY][iX] = chSprite;
}
bool ScreenBuffer::IsOutOfScreen(int x, int y)
{
	if (x < 0 || y < 0 || x >= dfSCREEN_WIDTH - 1 || y >= dfSCREEN_HEIGHT)
		return true;

	return false;
}
