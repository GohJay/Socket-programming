#ifndef  __OBJECT_POOL_TLS__H_
#define  __OBJECT_POOL_TLS__H_
#include "ObjectPool.h"

#define CHUNK_UNIT		500

namespace Jay
{
	/**
	* @file		ObjectPool_TLS.h
	* @brief	������Ʈ �޸� Ǯ Ŭ����(������Ʈ Ǯ / ��������Ʈ) TLS ����
	* @details	Ư�� �����͸�(����ü, Ŭ����, ����) ������ �����庰�� �Ҵ� �� ��������.
	* @usage	Jay::ObjectPool_TLS<T> MemPool(300, false);
				T *pData = MemPool.Alloc();
				pData ���
				MemPool.Free(pData);
	* @author   ������
	* @date		2022-12-31
	* @version  1.0.0
	**/
	template <typename T>
	class ObjectPool_TLS
	{
	private:
		struct NODE
		{
			NODE() {}
			~NODE() {}
			T data;
#if SECURE_MODE
			size_t signature;
#endif
			PVOID chunk;
		};
		struct CHUNK
		{
			NODE nodeArray[CHUNK_UNIT];
			LONG allocCount;
			LONG freeCount;
		};
	public:
		/**
		* @brief	������, �Ҹ���
		* @details
		* @param	int(�ʱ� ûũ ����), bool(Alloc �� ������ / Free �� �ı��� ȣ�� ����)
		* @return	
		**/
		ObjectPool_TLS(int chunkNum, bool placementNew = false) 
			: _chunkPool(chunkNum, placementNew)
		{
			_tlsChunk = TlsAlloc();
		}
		~ObjectPool_TLS()
		{
			TlsFree(_tlsChunk);
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
			NODE* node;
			CHUNK* chunk;
			
			chunk = (CHUNK*)TlsGetValue(_tlsChunk);
			if (chunk == NULL)
			{
				chunk = _chunkPool.Alloc();
				chunk->allocCount = 0;
				chunk->freeCount = 0;
				TlsSetValue(_tlsChunk, chunk);
			}

			node = &chunk->nodeArray[chunk->allocCount];
			node->chunk = chunk;
#if SECURE_MODE
			node->signature = (size_t)this;
#endif
			chunk->allocCount++;
			if (chunk->allocCount == CHUNK_UNIT)
				TlsSetValue(_tlsChunk, NULL);

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
			CHUNK* chunk;
			LONG count;

			node = (NODE*)data;
#if SECURE_MODE
			if (node->signature != (size_t)this)
				throw;
#endif
			chunk = (CHUNK*)node->chunk;
			count = InterlockedIncrement(&chunk->freeCount);
			if (count == CHUNK_UNIT)
				_chunkPool.Free(chunk);
		}

		/**
		* @brief	���� Ȯ�� �� �� ������ ��´�.(�޸� Ǯ ������ ��ü ����)
		* @details
		* @param	void
		* @return	int(�޸� Ǯ ���� ��ü �� ����)
		**/
		inline int GetCapacityCount(void)
		{
			return _chunkPool.GetCapacityCount() * CHUNK_UNIT;
		}

		/**
		* @brief	���� ������� �� ������ ��´�.
		* @details
		* @param	void
		* @return	int(������� �� ����)
		**/
		inline int GetUseCount(void)
		{
			return _chunkPool.GetUseCount() * CHUNK_UNIT;
		}
	private:
		DWORD _tlsChunk;
		ObjectPool<CHUNK> _chunkPool;
	};
}

#endif //!__OBJECT_POOL_TLS__H_
