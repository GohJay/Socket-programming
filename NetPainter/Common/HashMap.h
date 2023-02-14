#ifndef __HASHMAP__H_
#define __HASHMAP__H_
#include "List.h"
#include <string>

namespace Jay
{
	template<typename K>
	int GetHashCode(K key);
	template<>
	int GetHashCode(std::string key);
	template<>
	int GetHashCode(std::wstring key);

	/**
	* @file		HashMap.h
	* @brief	HashMap Template Class
	* @details	Iterator 패턴으로 구현한 해시테이블 클래스
	* @author   고재현
	* @date		2023-01-08
	* @version  1.0.1
	**/
	template<typename K, typename V>
	class HashMap
	{
	private:
		struct MAP
		{
			K key;
			V value;
			MAP* prev;
			MAP* next;
		};
		class iterator
		{
		public:
			iterator(MAP* ptr = nullptr) : cur(ptr)
			{
			}
		public:
			iterator& operator++()
			{
				cur = cur->next;
				return *this;
			}
			iterator operator ++(int)
			{
				iterator temp(cur);
				cur = cur->next;
				return temp;
			}
			iterator& operator--()
			{
				cur = cur->prev;
				return *this;
			}
			iterator operator --(int)
			{
				iterator temp(cur);
				cur = cur->prev;
				return temp;
			}
			K first()
			{
				return cur->key;
			}
			V second()
			{
				return cur->value;
			}
			bool operator ==(const iterator& ref)
			{
				return cur == ref.cur;
			}
			bool operator !=(const iterator& ref)
			{
				return cur != ref.cur;
			}
		private:
			MAP* cur;
			friend class HashMap;
		};
	public:
		HashMap() : tableSize(1024), count(0)
		{
			table = new List<MAP*>[tableSize]();
			head.prev = nullptr;
			head.next = &tail;
			tail.prev = &head;
			tail.next = nullptr;
		}
		~HashMap()
		{
			clear();
			delete[] table;
		}
	public:
		iterator begin()
		{
			return iterator(head.next);
		}
		iterator end()
		{
			return iterator(&tail);
		}
		bool insert(K key, V value)
		{
			int hashcode = GetHashCode(key);
			int index = ConvertToIndex(hashcode);
			for (auto iter = table[index].begin(); iter != table[index].end(); ++iter)
			{
				MAP* pMap = *iter;
				if (key == pMap->key)
					return false;
			}
			MAP* pMap = new MAP();
			pMap->key = key;
			pMap->value = value;
			pMap->prev = tail.prev;
			pMap->next = &tail;
			tail.prev->next = pMap;
			tail.prev = pMap;
			table[index].push_front(pMap);
			count++;
			return true;
		}
		iterator erase(iterator input)
		{
			K key = input.first();
			int hashcode = GetHashCode(key);
			int index = ConvertToIndex(hashcode);
			for (auto iter = table[index].begin(); iter != table[index].end(); ++iter)
			{
				MAP* pMap = *iter;
				if (key == pMap->key)
				{
					iterator output(pMap->next);
					pMap->prev->next = pMap->next;
					pMap->next->prev = pMap->prev;
					delete pMap;
					table[index].erase(iter);
					count--;
					return output;
				}
			}
			return end();
		}
		iterator find(K key)
		{
			int hashcode = GetHashCode(key);
			int index = ConvertToIndex(hashcode);
			for (auto iter = table[index].begin(); iter != table[index].end(); ++iter)
			{
				MAP* pMap = *iter;
				if (key == pMap->key)
					return iterator(pMap);
			}
			return end();
		}
		void clear()
		{
			iterator iter;
			for (iter = begin(); iter != end();)
			{
				iter = erase(iter);
			}
		}
		int size()
		{
			return count;
		}
		bool empty()
		{
			return count == 0;
		}
	private:
		int ConvertToIndex(int hashcode)
		{
			return hashcode % tableSize;
		}
	private:
		List<MAP*> *table;
		MAP head;
		MAP tail;
		int tableSize;
		long count;
	};

	template<typename K>
	inline int GetHashCode(K key)
	{
		return abs(static_cast<int>(key));
	}
	template<>
	inline int GetHashCode(std::string key)
	{
		int hashcode = 0;
		for (int i = 0; i < key.size(); i++)
			hashcode += key[i];
		return hashcode;
	}
	template<>
	inline int GetHashCode(std::wstring key)
	{
		int hashcode = 0;
		for (int i = 0; i < key.size(); i++)
			hashcode += key[i];
		return hashcode;
	}
}

#endif
