#include "stdafx.h"
#include "KeyHandler.h"

KeyHandler KeyHandler::_instance;
KeyHandler::KeyHandler()
{
}
KeyHandler::~KeyHandler()
{
}
KeyHandler * KeyHandler::GetInstance()
{
	return &_instance;
}
void KeyHandler::KeyProcess()
{
	_keyState.up = GetAsyncKeyState(VK_UP);
	_keyState.left = GetAsyncKeyState(VK_LEFT);
	_keyState.down = GetAsyncKeyState(VK_DOWN);
	_keyState.right = GetAsyncKeyState(VK_RIGHT);
}

bool KeyHandler::IsMove()
{
	return _keyState.up || _keyState.left || _keyState.down || _keyState.right;
}
