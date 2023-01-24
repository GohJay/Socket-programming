#ifndef __NETPACKET__H_
#define __NETPACKET__H_
#include "Base.h"
#include "ObjectPool_TLS.h"

namespace Jay
{
	class LanClient;
	class LanServer;
	class NetClient;
	class NetServer;
	class NetPacketPtr;
	class NetPacket
	{
		/**
		* @file		NetPacket.h
		* @brief	Network Packet Class (SerializationBuffer)
		* @details	��Ʈ��ũ �ۼ����� ���� ����ȭ���� Ŭ����
		* @author   ������
		* @date		2023-01-22
		* @version  1.0.8
		**/
	private:
		NetPacket(int bufferSize = 1024);
		~NetPacket();
	public:
		/**
		* @brief	����ȭ ���� �Ҵ�
		* @details
		* @param	void
		* @return	NetPacket*(����ȭ ���� ������)
		**/
		static NetPacket* Alloc(void);

		/**
		* @brief	����ȭ ���� ����
		* @details
		* @param	NetPacket*(����ȭ ���� ������)
		* @return	void
		**/
		static void Free(NetPacket* packet);

		/**
		* @brief	������ ���� ī��Ʈ ����
		* @details
		* @param	void
		* @return	int(������ ���� ī��Ʈ ��)
		**/
		long IncrementRefCount(void);

		/**
		* @brief	���� ������ ���
		* @details
		* @param	void
		* @return	int(���� ������)
		**/
		int	GetBufferSize(void);

		/**
		* @brief	���� ���ۿ� ���� �뷮 ���
		* @details
		* @param	void
		* @return	int(���� �뷮)
		**/
		int GetFreeSize(void);

		/**
		* @brief	���� ������� �뷮 ���
		* @details
		* @param	void
		* @return	int(������� �뷮)
		**/
		int GetUseSize(void);

		/**
		* @brief	������ ���� ī��Ʈ ���
		* @details
		* @param	void
		* @return	int(���� ī��Ʈ)
		**/
		int	GetRefCount(void);

		/**
		* @brief	���� ũ�� ����
		* @details
		* @param	int(������ ���� ũ��)
		* @return	void
		**/
		void Resize(int bufferSize);

		/**
		* @brief	������ ��� ������ ����
		* @details
		* @param	void
		* @return	void
		**/
		void ClearBuffer(void);

		/**
		* @brief	������ �Է�
		* @details
		* @param	const char*(������ ������), int(ũ��)
		* @return	int(�Էµ� ������ ũ��)
		**/
		int	PutData(const char* input, int size);

		/**
		* @brief	������ ���
		* @details
		* @param	char*(������ ������), int(ũ��)
		* @return	int(��µ� ������ ũ��)
		**/
		int	GetData(char* output, int size);
	private:
		/**
		* @brief	��Ŷ ������ ���
		* @details
		* @param	void
		* @return	int(��Ŷ ������)
		**/
		int GetPacketSize(void);

		/**
		* @brief	���ϴ� ���̸�ŭ �б���ġ���� ���� / ������ġ �̵�
		* @details
		* @param	int(���ϴ� ����)
		* @return	void
		**/
		void MoveFront(int size);
		void MoveRear(int size);

		/**
		* @brief	������ Front ������ ����
		* @details
		* @param	void
		* @return	char*(���� ������)
		**/
		char* GetFrontBufferPtr(void);

		/**
		* @brief	������ Rear ������ ����
		* @details
		* @param	void
		* @return	char*(���� ������)
		**/
		char* GetRearBufferPtr(void);

		/**
		* @brief	������ Header ������ ����
		* @details
		* @param	void
		* @return	char*(���� ������)
		**/
		char* GetHeaderPtr(void);

		/**
		* @brief	��� �Է�
		* @details
		* @param	const char*(��� ������), int(ũ��)
		* @return	int(�Էµ� ��� ũ��)
		**/
		int PutHeader(const char* header, int size);

		/**
		* @brief	��Ŷ ���ڵ� ó��
		* @details
		* @param	BYTE(��Ŷ �ڵ�), BYTE(��Ŷ ��ĪŰ)
		* @return	void
		**/
		void Encode(BYTE code, BYTE key);

		/**
		* @brief	��Ŷ ���ڵ� ó��
		* @details
		* @param	BYTE(��Ŷ �ڵ�), BYTE(��Ŷ ��ĪŰ)
		* @return	bool(��Ŷ ���ڵ� ���)
		**/
		bool Decode(BYTE code, BYTE key);

		/**
		* @brief	��Ŷ ��ȣȭ ó��
		* @details
		* @param	BYTE(��Ŷ ��ĪŰ)
		* @return	void
		**/
		void Encrypt(BYTE key);

		/**
		* @brief	��Ŷ ��ȣȭ ó��
		* @details
		* @param	BYTE(��Ŷ ��ĪŰ)
		* @return	void
		**/
		void Decrypt(BYTE key);

		/**
		* @brief	��Ŷ üũ�� ���
		* @details
		* @param	void
		* @return	unsigned char(��Ŷ üũ��)
		**/
		unsigned char MakeChecksum(void);
	public:
		NetPacket& operator = (const NetPacket& packet);

		NetPacket& operator << (const char value) throw(...);
		NetPacket& operator << (const unsigned char value) throw(...);

		NetPacket& operator << (const short value) throw(...);
		NetPacket& operator << (const unsigned short value) throw(...);

		NetPacket& operator << (const long value) throw(...);
		NetPacket& operator << (const unsigned long value) throw(...);

		NetPacket& operator << (const long long value) throw(...);
		NetPacket& operator << (const unsigned long long value) throw(...);

		NetPacket& operator << (const int value) throw(...);
		NetPacket& operator << (const unsigned int value) throw(...);

		NetPacket& operator << (const float value) throw(...);
		NetPacket& operator << (const double value) throw(...);

		NetPacket& operator >> (char& value) throw(...);
		NetPacket& operator >> (unsigned char& value) throw(...);

		NetPacket& operator >> (short& value) throw(...);
		NetPacket& operator >> (unsigned short& value) throw(...);

		NetPacket& operator >> (long& value) throw(...);
		NetPacket& operator >> (unsigned long& value) throw(...);

		NetPacket& operator >> (long long& value) throw(...);
		NetPacket& operator >> (unsigned long long& value) throw(...);

		NetPacket& operator >> (int& value) throw(...);
		NetPacket& operator >> (unsigned int& value) throw(...);

		NetPacket& operator >> (float& value) throw(...);
		NetPacket& operator >> (double& value) throw(...);
	private:
		char* _buffer;
		char* _bufferEnd;
		char* _front;
		char* _rear;
		char* _header;
		int _bufferSize;
		long _refCount;
		bool _encode;
		static ObjectPool_TLS<NetPacket> _packetPool;
		friend class ObjectPool_TLS<NetPacket>;
		friend class NetPacketPtr;
		friend class NetServer;
		friend class NetClient;
		friend class LanServer;
		friend class LanClient;
	};
}

#endif
