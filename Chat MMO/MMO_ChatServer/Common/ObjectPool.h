#ifndef  __OBJECT_POOL__H_
#define  __OBJECT_POOL__H_
#ifdef _WIN64

#define SECURE_MODE					1

#define MAXIMUM_ADDRESS_RANGE		0x00007ffffffeffff
#define MAXIMUM_ADDRESS_MASK		0x00007fffffffffff
#define NODE_STAMP_MASK				0xffff800000000000
#define NODE_STAMP_OFFSET			47

#define GET_NODE_STAMP(node)		(LONG64)((ULONG_PTR)(node) & NODE_STAMP_MASK)
#define GET_NODE_ADDRESS(node)		(PVOID)((ULONG_PTR)(node) & MAXIMUM_ADDRESS_MASK)
#define NEXT_NODE_STAMP(basenode)	(LONG64)((((LONG64)(basenode) >> NODE_STAMP_OFFSET) + 1) << NODE_STAMP_OFFSET)
#define MAKE_NODE(address, stamp)	(PVOID)((ULONG_PTR)(address) | (stamp))

#include <new.h>

namespace Jay
{
	/**
	* @file		ObjectPool.h
	* @brief	������Ʈ �޸� Ǯ Ŭ����(������Ʈ Ǯ / ��������Ʈ)
	* @details	Ư�� �����͸�(����ü, Ŭ����, ����) ������ �Ҵ� �� ��������.
	* @usage	Jay::ObjectPool<T> MemPool(300, false);
				T *pData = MemPool.Alloc();
				pData ���
				MemPool.Free(pData);
	* @author   ������
	* @date		2022-12-08
	* @version  1.1.3
	**/
	template <typename T>
	class ObjectPool
	{
	private:
		struct NODE
		{
			T data;
#if SECURE_MODE
			size_t signature;
#endif
			NODE* prev;
		};
	public:
		/**
		* @brief	������, �Ҹ���
		* @details
		* @param	int(�ʱ� �� ����), bool(Alloc �� ������ / Free �� �ı��� ȣ�� ����)
		* @return	
		**/
		ObjectPool(int blockNum, bool placementNew = false) 
			: _top(nullptr), _placementNew(placementNew), _capacity(0), _useCount(0)
		{
			NODE* node; 
			LONG64 nodeStamp;
			while (blockNum > 0)
			{
				node = (NODE*)malloc(sizeof(NODE));
				node->prev = (NODE*)GET_NODE_ADDRESS(_top);
#if SECURE_MODE
				node->signature = (size_t)this;
				_capacity++;
#endif
				if (!_placementNew)
					new(&node->data) T();

				nodeStamp = NEXT_NODE_STAMP(_top);
				_top = (NODE*)MAKE_NODE(node, nodeStamp);

				blockNum--;
			}
		}
		~ObjectPool()
		{
			NODE* prev;
			_top = (NODE*)GET_NODE_ADDRESS(_top);
			while (_top)
			{
				if (!_placementNew)
					_top->data.~T();

				prev = _top->prev;
				free(_top);
				_top = prev;
#if SECURE_MODE
				_capacity--;
#endif
			}
		}
	public:
		/**
		* @brief	�� �ϳ��� �Ҵ�޴´�.
		* @details
		* @param	void
		* @return	T*(������ �� ������)
		**/
		T* Alloc(void)
		{
			NODE* top;
			NODE* prev;
			NODE* node;
			LONG64 nodeStamp;

			do
			{
				top = _top;
				node = (NODE*)GET_NODE_ADDRESS(top);
				if (node == nullptr)
				{
					node = (NODE*)malloc(sizeof(NODE));
					new(&node->data) T();
#if SECURE_MODE
					node->signature = (size_t)this;
					InterlockedIncrement(&_capacity);
					InterlockedIncrement(&_useCount);
#endif
					return &node->data;
				}
				nodeStamp = GET_NODE_STAMP(top);
				prev = (NODE*)MAKE_NODE(node->prev, nodeStamp);
			} while (InterlockedCompareExchangePointer((PVOID*)&_top, prev, top) != top);

			if (_placementNew)
				new(&node->data) T();
#if SECURE_MODE
			InterlockedIncrement(&_useCount);
#endif
			return &node->data;
		}
		
		/**
		* @brief	������̴� ���� �����Ѵ�.
		* @details	
		* @param	T*(������ �� ������)
		* @return	void
		**/
		void Free(T* data) throw(...)
		{
			NODE* node;
			NODE* top;
			NODE* prev;
			LONG64 nodeStamp;

			node = (NODE*)data;
#if SECURE_MODE
			if (node->signature != (size_t)this)
				throw;
#endif
			if (_placementNew)
				node->data.~T();

			do
			{
				top = _top;
				node->prev = (NODE*)GET_NODE_ADDRESS(top);
				nodeStamp = NEXT_NODE_STAMP(top);
				prev = (NODE*)MAKE_NODE(node, nodeStamp);
			} while (InterlockedCompareExchangePointer((PVOID*)&_top, prev, top) != top);

#if SECURE_MODE
			InterlockedDecrement(&_useCount);
#endif
		}
		
		/**
		* @brief	���� Ȯ�� �� �� ������ ��´�.(�޸� Ǯ ������ ��ü ����)
		* @details
		* @param	void
		* @return	int(�޸� Ǯ ���� ��ü �� ����)
		**/
		inline int GetCapacityCount(void)
		{
			return _capacity;
		}

		/**
		* @brief	���� ������� �� ������ ��´�.
		* @details
		* @param	void
		* @return	int(������� �� ����)
		**/
		inline int GetUseCount(void)
		{
			return _useCount;
		}
	private:
		NODE* _top;
		bool _placementNew;
		long _capacity;
		long _useCount;
	};
}

#endif //!_WIN64
#endif //!__OBJECT_POOL__H_
