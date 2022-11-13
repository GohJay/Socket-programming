#ifndef __STL_WRAPPER__H_
#define __STL_WRAPPER__H_
#include <list>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>

template <typename V>
class List
{
public:
	typedef typename std::list<V>::iterator Iterator;
public:
	inline
		void Insert(V value)
	{
		_container.push_back(value);
	}
	inline
		void Remove(V value)
	{
		_container.remove(value);
	}
	inline
		Iterator Erase(Iterator iter)
	{
		return _container.erase(iter);
	}
	inline
		Iterator Find(V value)
	{
		return std::find(_container.begin(), _container.end(), value);
	}
	inline
		Iterator Begin()
	{ 
		return _container.begin();
	}
	inline
		Iterator End()
	{
		return _container.end();
	}
	inline
		size_t Size()
	{
		return _container.size();
	}
	inline
		bool Empty()
	{
		return _container.empty();
	}
private:
	std::list<V> _container;
};

template <typename K, typename V>
class HashMap
{
public:
	typedef typename std::unordered_map<K, V>::iterator Iterator;
public:
	inline
		void Insert(K key, V value)
	{
		_container.insert({key, value});
	}
	inline
		void Remove(K key)
	{
		_container.erase(key);
	}
	inline
		Iterator Erase(Iterator iter)
	{
		return _container.erase(iter);
	}
	inline
		Iterator Find(K key)
	{
		return _container.find(key);
	}
	inline
		Iterator Begin()
	{
		return _container.begin();
	}
	inline
		Iterator End()
	{
		return _container.end();
	}
	inline
		size_t Size()
	{
		return _container.size();
	}
	inline
		bool Empty()
	{
		return _container.empty();
	}
private:
	std::unordered_map<K, V> _container;
};

template <typename K, typename V>
class Map
{
public:
	typedef typename std::map<K, V>::iterator Iterator;
public:
	inline
		void Insert(K key, V value)
	{
		_container.insert({ key, value });
	}
	inline
		void Remove(K key)
	{
		_container.erase(key);
	}
	inline
		Iterator Erase(Iterator iter)
	{
		return _container.erase(iter);
	}
	inline
		Iterator Find(K key)
	{
		return _container.find(key);
	}
	inline
		Iterator Begin()
	{
		return _container.begin();
	}
	inline
		Iterator End()
	{
		return _container.end();
	}
	inline
		size_t Size()
	{
		return _container.size();
	}
	inline
		bool Empty()
	{
		return _container.empty();
	}
private:
	std::map<K, V> _container;
};

template <typename K>
class HashSet
{
public:
	typedef typename std::unordered_set<K>::iterator Iterator;
public:
	inline
		void Insert(K key)
	{
		_container.insert(key);
	}
	inline
		void Remove(K key)
	{
		_container.erase(key);
	}
	inline
		Iterator Erase(Iterator iter)
	{
		return _container.erase(iter);
	}
	inline
		Iterator Find(K key)
	{
		return _container.find(key);
	}
	inline
		Iterator Begin()
	{
		return _container.begin();
	}
	inline
		Iterator End()
	{
		return _container.end();
	}
	inline
		size_t Size()
	{
		return _container.size();
	}
	inline
		bool Empty()
	{
		return _container.empty();
	}
private:
	std::unordered_set<K> _container;
};

template <typename K>
class Set
{
public:
	typedef typename std::set<K>::iterator Iterator;
public:
	inline
		void Insert(K key)
	{
		_container.insert(key);
	}
	inline
		void Remove(K key)
	{
		_container.erase(key);
	}
	inline
		Iterator Erase(Iterator iter)
	{
		return _container.erase(iter);
	}
	inline
		Iterator Find(K key)
	{
		return _container.find(key);
	}
	inline
		Iterator Begin()
	{
		return _container.begin();
	}
	inline
		Iterator End()
	{
		return _container.end();
	}
	inline
		size_t Size()
	{
		return _container.size();
	}
	inline
		bool Empty()
	{
		return _container.empty();
	}
private:
	std::set<K> _container;
};

#endif
