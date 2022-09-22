#ifndef __RINGBUFFER__H_
#define __RINGBUFFER__H_
#include "../../Common/Base.h"

JAYNAMESPACE
/**
* @file		RingBuffer.h
* @brief	Network RingBuffer Class
* @details	TCP/IP �������� �ۼ����� ���� ������ Ŭ����
* @author   ������
* @date		2022-08-04
* @version  1.0.0
**/
class RingBuffer
{
public:
	RingBuffer(int bufferSize = 1000);
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
	void MoveRear(int size);
	void MoveFront(int size);

	/**
	* @brief	������ ��� ������ ����
	* @details
	* @param	void
	* @return	void
	**/
	void ClearBuffer(void);
private:
	char* _buffer;
	char* _bufferEnd;
	int _bufferSize;
	char* _front;
	char* _rear;
};
JAYNAMESPACEEND

#endif
