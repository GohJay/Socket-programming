#ifndef  __OBJECT_POOL__H_
#define  __OBJECT_POOL__H_

#define SECURE_MODE 1
#ifdef _WIN64
#define MEM_GUARD 0xfdfdfdfdfdfdfdfd
#else
#define MEM_GUARD 0xfdfdfdfd
#endif

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
	* @date		2022-08-25
	* @version  1.0.1
	**/
	template <typename T>
	class ObjectPool
	{
	private:
		struct Node
		{
#if SECURE_MODE
			size_t signature;
			size_t underflowGuard;
			T data;
			size_t overflowGuard;
			Node* prev;
#else
			T data;
			Node* prev;
#endif
		};
	public:
		/**
		* @brief	������, �Ҹ���
		* @details
		* @param	int(�ʱ� �� ����), bool(Alloc �� ������ / Free �� �ı��� ȣ�� ����)
		* @return	
		**/
		ObjectPool(int blockNum, bool placementNew = false) 
			: _top(nullptr), _placementNew(placementNew), _capacity(blockNum), _useCount(0)
		{
			while (blockNum > 0)
			{
				Node* block = (Node*)malloc(sizeof(Node));
#if SECURE_MODE
				block->signature = (size_t)this;
				block->underflowGuard = MEM_GUARD;
				block->overflowGuard = MEM_GUARD;
#endif
				block->prev = _top;
				if (!_placementNew)
					new(&block->data) T();
				_top = block;
				blockNum--;
			}
		}
		~ObjectPool()
		{
			while (_top)
			{
				Node* prev = _top->prev;
				if (!_placementNew)
					_top->data.~T();
				free(_top);
				_top = prev;
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
			Node* block;
			if (_top == nullptr)
			{
				block = (Node*)malloc(sizeof(Node));
#if SECURE_MODE
				block->signature = (size_t)this;
				block->underflowGuard = MEM_GUARD;
				block->overflowGuard = MEM_GUARD;
#endif
				block->prev = _top;
				new(&block->data) T();
			}
			else
			{
				block = _top;
				if (_placementNew)
					new(&block->data) T();
				_top = _top->prev;
				_capacity--;
			}
			_useCount++;
			return &block->data;
		}
		
		/**
		* @brief	������̴� ���� �����Ѵ�.
		* @details	
		* @param	T*(������ �� ������)
		* @return	void
		**/
		void Free(T* data) throw()
		{
#if SECURE_MODE
			Node* block = (Node*)((char*)data - sizeof(size_t) - sizeof(size_t));
			if (block->signature != (size_t)this)
				throw std::exception("Incorrect signature");
			if (block->underflowGuard != MEM_GUARD)
				throw std::exception("Memory underflow");
			if (block->overflowGuard != MEM_GUARD)
				throw std::exception("Memory overflow");
#else
			Node* block = (Node*)data;
#endif
			if (_placementNew)
				block->data.~T();
			block->prev = _top;
			_top = block;
			_useCount--;
			_capacity++;
		}
		
		/**
		* @brief	���� Ȯ�� �� �� ������ ��´�.(�޸� Ǯ ������ ��ü ����)
		* @details
		* @param	void
		* @return	int(�޸� Ǯ ���� ��ü �� ����)
		**/
		int GetCapacityCount(void) { return _capacity; }

		/**
		* @brief	���� ������� �� ������ ��´�.
		* @details
		* @param	void
		* @return	int(������� �� ����)
		**/
		int GetUseCount(void) { return _useCount; }
	private:
		Node* _top;
		bool _placementNew;
		int _capacity;
		int _useCount;
	};
}
#endif
