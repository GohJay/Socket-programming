#ifndef __HASHTABLE__H_
#define __HASHTABLE__H_
#include "Base.h"
#include "List.h"

namespace Jay
{
	/**
	* @file		HashTable.h
	* @brief	HashTable Template Class
	* @details	Iterator 패턴으로 구현한 해시테이블 클래스
	* @author   고재현
	* @date		2022-08-12
	* @version  1.0.0
	**/
	template<typename K, typename V>
	class HashTable
	{
	private:
		struct Map
		{
			K key;
			V value;
			Map* prev;
			Map* next;
		};
		class iterator
		{
		public:
			iterator(Map* ptr = nullptr) : cur(ptr)
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
			Map* cur;
		private:
			friend class HashTable;
		};
	public:
		HashTable() : tableSize(1024), count(0)
		{
			table = new Jay::list<Map*>[tableSize]();
			head.prev = nullptr;
			head.next = &tail;
			tail.prev = &head;
			tail.next = nullptr;
		}
		~HashTable()
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
				Map* pMap = *iter;
				if (key == pMap->key)
					return false;
			}
			Map* pMap = new Map();
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
				Map* pMap = *iter;
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
				Map* pMap = *iter;
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
		Jay::list<Map*> *table;
		Map head;
		Map tail;
		int tableSize;
		int count;
	};
	template<typename K>
	int GetHashCode(K key)
	{		
		return abs(static_cast<int>(key));
	}
	template<>
	int GetHashCode(std::string key)
	{
		int hashcode = 0;
		for each (char s in key)
		{
			hashcode += s;
		}
		return hashcode;
	}
}

#endif
