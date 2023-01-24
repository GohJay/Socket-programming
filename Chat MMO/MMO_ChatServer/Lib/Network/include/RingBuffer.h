#ifndef __RINGBUFFER__H_
#define __RINGBUFFER__H_
#include "Base.h"

namespace Jay
{
	class LanClient;
	class LanServer;
	class NetClient;
	class NetServer;
	class RingBuffer
	{
		/**
		* @file		RingBuffer.h
		* @brief	Network RingBuffer Class
		* @details	TCP/IP �������� �ۼ����� ���� ������ Ŭ����
		* @author   ������
		* @date		2022-11-26
		* @version  1.0.5
		**/
	public:
		RingBuffer(int bufferSize = 8192);
		~RingBuffer();
	public:
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
		* @brief	���� �����ͷ� �ܺο��� �ѹ濡 �а� �� �� �ִ� ����
		* @details	���� ť�� ������ ������ ���ܿ� �ִ� �����ʹ� �� -> ó������ ���ư��� 2���� �����͸� ��ų� ���� �� ����.
		*			�� �κп��� �������� ���� ���̸� �ǹ�.
		* @param	void
		* @return	int(��밡�� �뷮)
		**/
		int	DirectEnqueueSize(void);
		int	DirectDequeueSize(void);

		/**
		* @brief	������ ��ü �뷮 ���
		* @details
		* @param	void
		* @return	int(���� ��ü �뷮)
		**/
		int GetBufferSize(void);

		/**
		* @brief	������ġ�� ������ �ֱ�
		* @details
		* @param	const char*(������ ������), int(ũ��)
		* @return	int(���� ũ��)
		**/
		int Enqueue(const char* input, int size);

		/**
		* @brief	�б���ġ���� ������ ��������
		* @details
		* @param	char*(������ ������), int(ũ��)
		* @return	int(������ ũ��)
		**/
		int Dequeue(char* output, int size);

		/**
		* @brief	���ϴ� ���̸�ŭ �б���ġ���� ���� / ������ġ �̵�
		* @details
		* @param	char*(������ ������), int(ũ��)
		* @return	int(������ ũ��)
		**/
		int Peek(char* output, int size);

		/**
		* @brief	������ ��� ������ ����
		* @details
		* @param	void
		* @return	void
		**/
		void ClearBuffer(void);
	private:
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
		* @brief	������ ���� ������ ����
		* @details
		* @param	void
		* @return	char*(���� ������)
		**/
		char* GetBufferPtr(void);
	private:
		char* _buffer;
		char* _bufferEnd;
		char* _front;
		char* _rear;
		int _bufferSize;
		friend class NetServer;
		friend class NetClient;
		friend class LanServer;
		friend class LanClient;
	};
}

#endif
