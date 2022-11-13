#ifndef __CONFIG_PARSER__H_
#define __CONFIG_PARSER__H_

namespace Jay
{
	/**
	* @file		ConfigParser.h
	* @brief	���� ���� �ļ� Ŭ����
	* @details	UTF-16LE ���ڵ��� �����ϴ� ���� ���� �ļ�
	* @author   ������
	* @date		2022-09-01
	* @version  1.0.0
	**/
	class ConfigParser
	{
	public:
		ConfigParser();
		~ConfigParser();
	public:
		bool LoadFile(const wchar_t* filepath);
		bool GetValue(const wchar_t* section, const wchar_t* key, int* value);
		bool GetValue(const wchar_t* section, const wchar_t* key, wchar_t* value);
	private:
		bool SkipNonCommand();
		bool GetNextWord(wchar_t** offset, int* len);
	private:
		wchar_t* _buffer;
		wchar_t* _bufferEnd;
		wchar_t* _bufferPos;
	};
}

#endif
