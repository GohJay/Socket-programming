#ifndef __SERVER__H_
#define __SERVER__H_

class ServerManager;
class Server
{
private:
	Server();
	~Server();
public:
	void Update();
	void Render();
private:
	void Listen();
	void Recv();

private:
	friend class ServerManager;
};

#endif