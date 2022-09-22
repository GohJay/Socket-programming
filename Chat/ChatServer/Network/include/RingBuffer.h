#ifndef __RINGBUFFER__H_
#define __RINGBUFFER__H_
#include "../../Common/Base.h"

JAYNAMESPACE
/**
* @file		RingBuffer.h
* @brief	Network RingBuffer Class
* @details	TCP/IP �������� �ۼ����� ���� ������ Ŭ����
* @author   ������
* @date		2022-08-20
* @version  1.0.2
**/
class RingBuffer
{
public:
	RingBuffer(int bufferSize = 1024);
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
	* @brief	������ġ�� ������ �ֱ�
	* @details
	* @param	const char*(������ ������), int(ũ��)
	* @return	int(���� ũ��)
	**/
	int Enqueue(const char *input, int size);

	/**
	* @brief	�б���ġ���� ������ ��������
	* @details
	* @param	char*(������ ������), int(ũ��)
	* @return	int(������ ũ��)
	**/
	int Dequeue(char *output, int size);

	/**
	* @brief	���ϴ� ���̸�ŭ �б���ġ���� ���� / ������ġ �̵�
	* @details
	* @param	char*(������ ������), int(ũ��)
	* @return	int(������ ũ��)
	**/
	int Peek(char *output, int size);

	/**
	* @brief	���ϴ� ���̸�ŭ �б���ġ���� ���� / ������ġ �̵�
	* @details
	* @param	int(���ϴ� ����)
	* @return	void
	**/
	void MoveFront(int size);
	void MoveRear(int size);

	/**
	* @brief	������ ��� ������ ����
	* @details
	* @param	void
	* @return	void
	**/
	void ClearBuffer(void);
	
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
private:
	char* _buffer;
	char* _bufferEnd;
	char* _front;
	char* _rear;
	int _bufferSize;
};
JAYNAMESPACEEND

#endif
