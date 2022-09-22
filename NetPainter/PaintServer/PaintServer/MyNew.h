#ifndef __MYNEW__H_
#define __MYNEW__H_
void* operator new (size_t size, const char* File, int Line);
void* operator new[](size_t size, const char* File, int Line);
void operator delete (void* ptr, const char* File, int Line);
void operator delete[](void* ptr, const char* File, int Line);
void operator delete (void* ptr);
void operator delete[](void* ptr);
#define new new(__FILE__, __LINE__)
#endif // !__MYNEW__H_
