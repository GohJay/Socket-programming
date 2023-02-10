#include "stdafx.h"
#include "ChatServer.h"
#include "Packet.h"
#include "../Lib/Network/include/Error.h"
#include "../Lib/Network/include/NetException.h"
#include "../Common/Logger.h"
#include "../Common/CrashDump.h"
#pragma comment(lib, "../Lib/Network/lib/Network.lib")

using namespace Jay;

ChatServer::ChatServer() : _characterPool(0), _jobPool(0)
{
}
ChatServer::~ChatServer()
{
}
bool ChatServer::Start(const wchar_t* ipaddress, int port, int workerCreateCnt, int workerRunningCnt, WORD sessionMax, WORD userMax, BYTE packetCode, BYTE packetKey, int timeoutSec, bool nagle)
{
	//--------------------------------------------------------------------
	// Initial
	//--------------------------------------------------------------------
	if (!Initial())
		return false;

	//--------------------------------------------------------------------
	// Network IO Start
	//--------------------------------------------------------------------
	if (!NetServer::Start(ipaddress, port, workerCreateCnt, workerRunningCnt, sessionMax, packetCode, packetKey, timeoutSec, nagle))
		return false;

	_userMax = userMax;

	//--------------------------------------------------------------------
	// UpdateThread Begin
	//--------------------------------------------------------------------
	_updateThread = std::thread(&ChatServer::UpdateThread, this);
	return true;
}
void ChatServer::Stop()
{
	//--------------------------------------------------------------------
	// UpdateThread End
	//--------------------------------------------------------------------
	SetEvent(_hExitEvent);
	_updateThread.join();

	//--------------------------------------------------------------------
	// Network IO Stop
	//--------------------------------------------------------------------
	NetServer::Stop();

	//--------------------------------------------------------------------
	// Release
	//--------------------------------------------------------------------
	Release();
}
int ChatServer::GetCharacterCount()
{
	return _characterMap.size();
}
int ChatServer::GetUseCharacterPool()
{
	return _characterPool.GetUseCount();
}
int ChatServer::GetJobQueueCount()
{
	return _jobQ.size();
}
int ChatServer::GetUseJobPool()
{
	return _jobPool.GetUseCount();
}
bool ChatServer::OnConnectionRequest(const wchar_t* ipaddress, int port)
{
	return true;
}
void ChatServer::OnClientJoin(DWORD64 sessionID)
{
	//--------------------------------------------------------------------
	// ������ƮǮ���� Job �Ҵ�
	//--------------------------------------------------------------------
	JOB* job = _jobPool.Alloc();
	job->type = JOIN;
	job->sessionID = sessionID;

	//--------------------------------------------------------------------
	// Job ť��
	//--------------------------------------------------------------------
	_jobQ.Enqueue(job);

	//--------------------------------------------------------------------
	// UpdateThread ���� Job �̺�Ʈ �˸�
	//--------------------------------------------------------------------
	SetEvent(_hJobEvent);
}
void ChatServer::OnRecv(DWORD64 sessionID, NetPacket* packet)
{
	//--------------------------------------------------------------------
	// ������ƮǮ���� Job �Ҵ�
	//--------------------------------------------------------------------
	JOB* job = _jobPool.Alloc();
	job->type = RECV;
	job->sessionID = sessionID;
	job->packet = packet;

	//--------------------------------------------------------------------
	// Job ť��
	//--------------------------------------------------------------------
	packet->IncrementRefCount();
	_jobQ.Enqueue(job);

	//--------------------------------------------------------------------
	// UpdateThread ���� Job �̺�Ʈ �˸�
	//--------------------------------------------------------------------
	SetEvent(_hJobEvent);
}
void ChatServer::OnClientLeave(DWORD64 sessionID)
{
	//--------------------------------------------------------------------
	// ������ƮǮ���� Job �Ҵ�
	//--------------------------------------------------------------------
	JOB* job = _jobPool.Alloc();
	job->type = LEAVE;
	job->sessionID = sessionID;

	//--------------------------------------------------------------------
	// Job ť��
	//--------------------------------------------------------------------
	_jobQ.Enqueue(job);

	//--------------------------------------------------------------------
	// UpdateThread ���� Job �̺�Ʈ �˸�
	//--------------------------------------------------------------------
	SetEvent(_hJobEvent);
}
void ChatServer::OnError(int errcode, const wchar_t* funcname, int linenum, WPARAM wParam, LPARAM lParam)
{
	//--------------------------------------------------------------------
	// Network IO Error �α�
	//--------------------------------------------------------------------
	Logger::WriteLog(L"Chat"
		, LOG_LEVEL_ERROR
		, L"func: %s, line: %d, error: %d, wParam: %llu, lParam: %llu"
		, funcname, linenum, errcode, wParam, lParam);

	//--------------------------------------------------------------------
	// Fatal Error �� ��� ũ���ÿ� �Բ� �޸� ������ �����.
	//--------------------------------------------------------------------
	if (errcode >= NET_FATAL_INVALID_SIZE)
		CrashDump::Crash();
}
bool ChatServer::Initial()
{
	_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (_hExitEvent == NULL)
		return false;

	_hJobEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (_hJobEvent == NULL)
	{
		CloseHandle(_hExitEvent);
		return false;
	}

	return true;
}
void ChatServer::Release()
{
	CHARACTER* character;
	for (auto iter = _characterMap.begin(); iter != _characterMap.end();)
	{
		character = iter->second;
		_characterPool.Free(character);
		iter = _characterMap.erase(iter);
	}

	JOB* job;
	while (_jobQ.size() > 0)
	{
		_jobQ.Dequeue(job);
		if (job->type == RECV)
			NetPacket::Free(job->packet);
		_jobPool.Free(job);
	}

	CloseHandle(_hExitEvent);
	CloseHandle(_hJobEvent);
}
void ChatServer::UpdateThread()
{
	HANDLE hHandle[2] = { _hJobEvent, _hExitEvent };
	DWORD ret;
	while (1)
	{
		ret = WaitForMultipleObjects(2, hHandle, FALSE, INFINITE);
		if ((ret - WAIT_OBJECT_0) != 0)
			break;

		JOB* job;
		while (_jobQ.size() > 0)
		{
			//--------------------------------------------------------------------
			// Job ��ť��
			//--------------------------------------------------------------------
			_jobQ.Dequeue(job);

			//--------------------------------------------------------------------
			// Job Type �� ���� �б� ó��
			//--------------------------------------------------------------------
			switch (job->type)
			{
			case JOIN:
				JoinProc(job->sessionID);
				break;
			case RECV:
				RecvProc(job->sessionID, job->packet);
				break;
			case LEAVE:
				LeaveProc(job->sessionID);
				break;
			default:
				break;
			}

			//--------------------------------------------------------------------
			// ������ƮǮ�� Job �ݳ�
			//--------------------------------------------------------------------
			_jobPool.Free(job);
		}
	}
}
void ChatServer::JoinProc(DWORD64 sessionID)
{
	//--------------------------------------------------------------------
	// ���� ������ ���� Ȯ���Ͽ� Join ���� �Ǵ�
	//--------------------------------------------------------------------
	if (_characterMap.size() >= _userMax)
	{
		Disconnect(sessionID);
		return;
	}

	//--------------------------------------------------------------------
	// ���� ����� ���ǿ� ���� ĳ���� �Ҵ�
	//--------------------------------------------------------------------
	NewCharacter(sessionID);
}
void ChatServer::RecvProc(DWORD64 sessionID, NetPacket* packet)
{
	//--------------------------------------------------------------------
	// ���� ���� �޽��� ó��
	//--------------------------------------------------------------------
	WORD type;

	try
	{
		(*packet) >> type;

		if (!PacketProc(sessionID, packet, type))
			Disconnect(sessionID);
	}
	catch (NetException& ex)
	{
		Logger::WriteLog(L"Chat", LOG_LEVEL_ERROR, L"NetException: %d, sessionID: %p", ex.GetLastError(), sessionID);
		Disconnect(sessionID);
	}

	NetPacket::Free(packet);
}
void ChatServer::LeaveProc(DWORD64 sessionID)
{
	//--------------------------------------------------------------------
	// ���� ������ ������ ĳ���� ����
	//--------------------------------------------------------------------
	DeleteCharacter(sessionID);
}
CHARACTER* ChatServer::NewCharacter(DWORD64 sessionID)
{
	CHARACTER* character;

	character = _characterPool.Alloc();
	character->sessionID = sessionID;
	character->login = false;
	character->sector.x = dfUNKNOWN_SECTOR;
	character->sector.y = dfUNKNOWN_SECTOR;

	_characterMap.insert({ sessionID, character });
	return character;
}
void ChatServer::DeleteCharacter(DWORD64 sessionID)
{
	CHARACTER* character;

	auto iter = _characterMap.find(sessionID);
	if (iter == _characterMap.end())
		return;

	character = iter->second;
	_characterMap.erase(iter);

	if (character->sector.x != dfUNKNOWN_SECTOR && character->sector.y != dfUNKNOWN_SECTOR)
	{
		RemoveCharacter_Sector(character);
	}
	_characterPool.Free(character);
}
CHARACTER* ChatServer::FindCharacter(DWORD64 sessionID)
{
	CHARACTER* character;

	auto iter = _characterMap.find(sessionID);
	if (iter == _characterMap.end())
		character = nullptr;
	else
		character = iter->second;

	return character;
}
bool ChatServer::PacketProc(DWORD64 sessionID, NetPacket* packet, WORD type)
{
	//--------------------------------------------------------------------
	// ���� �޽��� Ÿ�Կ� ���� �б� ó��
	//--------------------------------------------------------------------
	switch (type)
	{
	case en_PACKET_CS_CHAT_SERVER:
		break;
	case en_PACKET_CS_CHAT_REQ_LOGIN:
		return PacketProc_ChatLogin(sessionID, packet);
	case en_PACKET_CS_CHAT_RES_LOGIN:
		break;
	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		return PacketProc_ChatSectorMove(sessionID, packet);
	case en_PACKET_CS_CHAT_RES_SECTOR_MOVE:
		break;
	case en_PACKET_CS_CHAT_REQ_MESSAGE:
		return PacketProc_ChatMessage(sessionID, packet);
	case en_PACKET_CS_CHAT_RES_MESSAGE:
		break;
	case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
		return true;
	default:
		break;
	}
	return false;
}
bool ChatServer::PacketProc_ChatLogin(DWORD64 sessionID, NetPacket* packet)
{
	//--------------------------------------------------------------------
	// �α��� �޽��� ó��
	//--------------------------------------------------------------------
	CHARACTER* character = FindCharacter(sessionID);
	INT64 accountNo;
	WCHAR id[20];
	WCHAR nickname[20];
	char sessionKey[64];

	(*packet) >> accountNo;
	if (packet->GetData((char*)&id, sizeof(id)) != sizeof(id))
		return false;

	if (packet->GetData((char*)&nickname, sizeof(nickname)) != sizeof(nickname))
		return false;

	if (packet->GetData(sessionKey, sizeof(sessionKey)) != sizeof(sessionKey))
		return false;

	//--------------------------------------------------------------------
	// �̹� �α����� �������� ����
	//--------------------------------------------------------------------
	if (character->login)
		return false;

	character->login = true;
	character->accountNo = accountNo;
	wcscpy_s(character->id, sizeof(id) / 2, id);
	wcscpy_s(character->nickname, sizeof(nickname) / 2, nickname);
	memmove(character->sessionKey, sessionKey, sizeof(sessionKey));

	//--------------------------------------------------------------------
	// ������ƮǮ���� ����ȭ ���� �Ҵ�
	//--------------------------------------------------------------------
	NetPacket* resPacket = NetPacket::Alloc();

	//--------------------------------------------------------------------
	// �ش� �������� �α��� �Ϸ� �޽��� ������
	//--------------------------------------------------------------------
	Packet::MakeChatLogin(resPacket, true, character->accountNo);
	SendPacket(character->sessionID, resPacket);

	//--------------------------------------------------------------------
	// ������ƮǮ�� ����ȭ ���� �ݳ�
	//--------------------------------------------------------------------
	NetPacket::Free(resPacket);
	return true;
}
bool ChatServer::PacketProc_ChatSectorMove(DWORD64 sessionID, NetPacket* packet)
{
	//--------------------------------------------------------------------
	// ���� �̵� ó��
	//--------------------------------------------------------------------
	CHARACTER* character = FindCharacter(sessionID);
	INT64 accountNo;
	WORD sectorX;
	WORD sectorY;

	(*packet) >> accountNo >> sectorX >> sectorY;

	//--------------------------------------------------------------------
	// �α������� ���� �������� ����
	//--------------------------------------------------------------------
	if (!character->login)
		return false;

	//--------------------------------------------------------------------
	// ���� ��ȣ ����
	//--------------------------------------------------------------------
	if (character->accountNo != accountNo)
		return false;

	//--------------------------------------------------------------------
	// ������ �̵� ��û�� ��ǥ�� ������ �̵� ������ ��ǥ���� Ȯ��
	//--------------------------------------------------------------------
	if (!IsMovableCharacter(sectorX, sectorY))
		return false;

	//--------------------------------------------------------------------
	// ���� �̵� ó��
	//--------------------------------------------------------------------
	if (character->sector.x != sectorX || character->sector.y != sectorY)
		UpdateCharacter_Sector(character, sectorX, sectorY);

	//--------------------------------------------------------------------
	// ������ƮǮ���� ����ȭ ���� �Ҵ�
	//--------------------------------------------------------------------
	NetPacket *resPacket = NetPacket::Alloc();

	//--------------------------------------------------------------------
	// �ش� �������� ���� �̵� �޽��� ������
	//--------------------------------------------------------------------
	Packet::MakeChatSectorMove(resPacket
		, character->accountNo
		, character->sector.x
		, character->sector.y);
	SendPacket(character->sessionID, resPacket);

	//--------------------------------------------------------------------
	// ������ƮǮ�� ����ȭ ���� �ݳ�
	//--------------------------------------------------------------------
	NetPacket::Free(resPacket);
	return true;
}
bool ChatServer::PacketProc_ChatMessage(DWORD64 sessionID, NetPacket* packet)
{
	//--------------------------------------------------------------------
	// ä�� �޽��� ó��
	//--------------------------------------------------------------------
	CHARACTER* character = FindCharacter(sessionID);
	INT64 accountNo;
	WORD messageLen;
	WCHAR message[256];

	(*packet) >> accountNo >> messageLen;
	if (messageLen > sizeof(message))
		return false;

	if (packet->GetData((char*)message, messageLen) != messageLen)
		return false;

	if (character->accountNo != accountNo)
		return false;

	//--------------------------------------------------------------------
	// �α������� ���� �������� ����
	//--------------------------------------------------------------------
	if (!character->login)
		return false;

	//--------------------------------------------------------------------
	// ������ƮǮ���� ����ȭ ���� �Ҵ�
	//--------------------------------------------------------------------
	NetPacket* resPacket = NetPacket::Alloc();

	//--------------------------------------------------------------------
	// �ֺ� ����� ������ �����鿡�� ä�� �޽��� ������
	//--------------------------------------------------------------------
	Packet::MakeChatMessage(resPacket
		, character->accountNo
		, character->id
		, character->nickname
		, messageLen
		, message);
	SendSectorAround(character, resPacket);

	//--------------------------------------------------------------------
	// ������ƮǮ�� ����ȭ ���� �ݳ�
	//--------------------------------------------------------------------
	NetPacket::Free(resPacket);
	return true;
}
bool ChatServer::IsMovableCharacter(int sectorX, int sectorY)
{
	//--------------------------------------------------------------------
	// ���� ��ǥ �̵� ���ɿ��� Ȯ��
	//--------------------------------------------------------------------
	if (sectorX < dfSECTOR_MAX_X && sectorY < dfSECTOR_MAX_Y)
		return true;

	return false;
}
void ChatServer::AddCharacter_Sector(CHARACTER* character, int sectorX, int sectorY)
{
	//--------------------------------------------------------------------
	// ���� ����Ʈ�� �÷��̾� �߰�
	//--------------------------------------------------------------------
	character->sector.x = sectorX;
	character->sector.y = sectorY;
	_sectorList[character->sector.y][character->sector.x].push_back(character);
}
void ChatServer::RemoveCharacter_Sector(CHARACTER* character)
{
	//--------------------------------------------------------------------
	// ���� ����Ʈ���� �÷��̾� ����
	//--------------------------------------------------------------------
	_sectorList[character->sector.y][character->sector.x].remove(character);
}
void ChatServer::UpdateCharacter_Sector(CHARACTER* character, int sectorX, int sectorY)
{
	if (character->sector.x != dfUNKNOWN_SECTOR && character->sector.y != dfUNKNOWN_SECTOR)
		RemoveCharacter_Sector(character);

	AddCharacter_Sector(character, sectorX, sectorY);
}
void ChatServer::GetSectorAround(int sectorX, int sectorY, SECTOR_AROUND* sectorAround)
{
	//--------------------------------------------------------------------
	// Ư�� ��ǥ �������� �ֺ� ����� ���� ���
	//--------------------------------------------------------------------
	sectorX--;
	sectorY--;

	sectorAround->count = 0;
	for (int y = 0; y < 3; y++)
	{
		if (sectorY + y < 0 || sectorY + y >= dfSECTOR_MAX_Y)
			continue;

		for (int x = 0; x < 3; x++)
		{
			if (sectorX + x < 0 || sectorX + x >= dfSECTOR_MAX_X)
				continue;

			sectorAround->around[sectorAround->count].x = sectorX + x;
			sectorAround->around[sectorAround->count].y = sectorY + y;
			sectorAround->count++;
		}
	}
}
void ChatServer::SendSectorOne(NetPacket* packet, int sectorX, int sectorY)
{
	//--------------------------------------------------------------------
	// Ư�� ���Ϳ� �ִ� �����鿡�� �޽��� ������
	//--------------------------------------------------------------------
	std::list<CHARACTER*> *sectorList = &_sectorList[sectorY][sectorX];
	for (auto iter = sectorList->begin(); iter != sectorList->end(); ++iter)
	{
		SendPacket((*iter)->sessionID, packet);
	}
}
void ChatServer::SendSectorAround(CHARACTER* character, NetPacket* packet)
{
	//--------------------------------------------------------------------
	// �ֺ� ����� ���Ϳ� �ִ� �����鿡�� �޽��� ������
	//--------------------------------------------------------------------
	SECTOR_AROUND sectorAround;
	GetSectorAround(character->sector.x, character->sector.y, &sectorAround);

	for (int i = 0; i < sectorAround.count; i++)
	{
		SendSectorOne(packet, sectorAround.around[i].x, sectorAround.around[i].y);
	}
}
