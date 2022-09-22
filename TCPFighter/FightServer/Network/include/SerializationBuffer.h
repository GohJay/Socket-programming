#ifndef __SERIALIZATIONBUFFER__H_
#define __SERIALIZATIONBUFFER__H_
#include "../../Common/Base.h"

JAYNAMESPACE
/**
* @file		SerializationBuffer.h
* @brief	Network SerializationBuffer Class
* @details	��Ʈ��ũ �ۼ����� ���� ����ȭ���� Ŭ����
* @author   ������
* @date		2022-08-20
* @version  1.0.0
**/
class SerializationBuffer
{
public:
	SerializationBuffer(int bufferSize = 256);
	virtual ~SerializationBuffer();
public:
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
	* @brief	���� ũ�� ����
	* @details
	* @param	int(������ ���� ũ��)
	* @return	void
	**/
	void Resize(int bufferSize);

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
	* @brief	���� ������ ���
	* @details
	* @param	void
	* @return	char*(���� ������)
	**/
	char *GetBufferPtr(void);

	/**
	* @brief	������ �Է�
	* @details
	* @param	const char*(������ ������), int(ũ��)
	* @return	int(�Էµ� ������ ũ��)
	**/
	int	PutData(const char *input, int size);

	/**
	* @brief	������ ���
	* @details
	* @param	char*(������ ������), int(ũ��)
	* @return	int(��µ� ������ ũ��)
	**/
	int	GetData(char *output, int size);
public:
	SerializationBuffer	&operator = (SerializationBuffer &packet);

	SerializationBuffer	&operator << (char value);
	SerializationBuffer	&operator << (unsigned char value);

	SerializationBuffer	&operator << (short value);
	SerializationBuffer	&operator << (unsigned short value);

	SerializationBuffer	&operator << (long value);
	SerializationBuffer	&operator << (unsigned long value);

	SerializationBuffer	&operator << (long long value);
	SerializationBuffer	&operator << (unsigned long long value);

	SerializationBuffer	&operator << (int value);
	SerializationBuffer	&operator << (unsigned int value);

	SerializationBuffer	&operator << (float value);
	SerializationBuffer	&operator << (double value);

	SerializationBuffer	&operator >> (char &value);
	SerializationBuffer	&operator >> (unsigned char &value);

	SerializationBuffer	&operator >> (short &value);
	SerializationBuffer	&operator >> (unsigned short &value);

	SerializationBuffer	&operator >> (long &value);
	SerializationBuffer	&operator >> (unsigned long &value);

	SerializationBuffer	&operator >> (long long &value);
	SerializationBuffer	&operator >> (unsigned long long &value);

	SerializationBuffer	&operator >> (int &value);
	SerializationBuffer	&operator >> (unsigned int &value);

	SerializationBuffer	&operator >> (float &value);
	SerializationBuffer	&operator >> (double &value);
protected:
	char* _buffer;
	char* _bufferEnd;
	char* _front;
	char* _rear;
	int _bufferSize;
};
JAYNAMESPACEEND

#endif
