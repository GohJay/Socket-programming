#ifndef __KEYHANDLER__H_
#define __KEYHANDLER__H_

class KeyHandler
{
private:
	struct KeyState
	{
		short left;
		short right;
		short up;
		short down;
	};
private:
	KeyHandler();
	~KeyHandler();
public:
	static KeyHandler* GetInstance();
	void KeyProcess();
	bool IsMove();
public:
	KeyState _keyState;
private:
	static KeyHandler _instance;
};

#endif // !__KEYHANDLER__H_
