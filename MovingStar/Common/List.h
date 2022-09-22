#ifndef __LIST__H_
#define __LIST__H_

namespace Jay
{
	/**
	* @file		List.h
	* @brief	Doubly linked list Template Class
	* @details	Iterator 패턴으로 구현한 이중연결 리스트 클래스
	* @author   고재현
	* @date		2022-06-11
	* @version  1.0.0
	**/
	template<typename T>
	class list
	{
	public:
		struct Node
		{
			T data;
			Node* prev;
			Node* next;
		};
		class iterator
		{
		public:
			iterator(Node* ptr = nullptr) : cur(ptr)
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
			T& operator *()
			{
				return cur->data;
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
			Node* cur;
		private:
			friend class list;
		};
	public:
		list() : count(0)
		{
			head.prev = nullptr;
			head.next = &tail;
			tail.prev = &head;
			tail.next = nullptr;
		}
		~list()
		{
			clear();
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
		void push_front(T data)
		{
			Node* pNode = new Node();
			pNode->data = data;
			pNode->prev = &head;
			pNode->next = head.next;
			head.next->prev = pNode;
			head.next = pNode;
			count++;
		}
		void push_back(T data)
		{
			Node* pNode = new Node();
			pNode->data = data;
			pNode->prev = tail.prev;
			pNode->next = &tail;
			tail.prev->next = pNode;
			tail.prev = pNode;
			count++;
		}
		void pop_front()
		{
			Node* pNode = head.next;
			head.next->next->prev = &head;
			head.next = head.next->next;
			delete pNode;
			count--;
		}
		void pop_back()
		{
			Node* pNode = tail.prev;
			tail.prev->prev->next = &tail;
			tail.prev = tail.prev->prev;
			delete pNode;
			count--;
		}
		iterator erase(iterator iter)
		{
			iterator temp(iter.cur->next);
			iter.cur->prev->next = iter.cur->next;
			iter.cur->next->prev = iter.cur->prev;
			delete iter.cur;
			count--;
			return temp;
		}
		void remove(T data)
		{
			iterator iter;
			for (iter = begin(); iter != end();)
			{
				if (*iter == data)
					iter = erase(iter);
				else
					++iter;
			}
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
		Node head;
		Node tail;
		int count;
	};
}

#endif // !__LIST__H_
