#pragma once

#include "Global.h"
#include "Linklist.h"
#include <assert.h>
#include <mutex>
#include <stdint.h>

class SafePtrClass
{
	friend class SafePtrBase;

private:
	class SafePtrBase *SafePtrList;

public:
	MOHPC_EXPORTS SafePtrClass();
	MOHPC_EXPORTS ~SafePtrClass();
};

class SafePtrBase
{
private:
	//std::mutex m_mutex;

private:
	void AddReference(SafePtrClass *ptr);
	void RemoveReference(SafePtrClass *ptr);

protected:
	SafePtrBase	*prev;
	SafePtrBase	*next;
	SafePtrClass *ptr;
	int32_t refCount;

public:
	MOHPC_EXPORTS SafePtrBase();
	MOHPC_EXPORTS virtual ~SafePtrBase();
	MOHPC_EXPORTS void InitSafePtr(SafePtrClass *newptr);
	MOHPC_EXPORTS SafePtrClass *Pointer();
	MOHPC_EXPORTS void Clear(void);
	MOHPC_EXPORTS int32_t GetRefCount() const;
};

template<class T>
class SafePtr : public SafePtrBase
{
public:
	SafePtr(T* objptr = 0);
	SafePtr(const SafePtr& obj);

	SafePtr& operator=(const SafePtr& obj);
	SafePtr& operator=(T * const obj);

	T *Pointer();

	template<class U> friend bool operator==(SafePtr<U> a, U *b);
	template<class U> friend bool operator!=(SafePtr<U> a, U *b);
	template<class U> friend bool operator==(U *a, SafePtr<U> b);
	template<class U> friend bool operator!=(U *a, SafePtr<U> b);
	template<class U> friend bool operator==(SafePtr<U> a, SafePtr<U> b);
	template<class U> friend bool operator!=(SafePtr<U> a, SafePtr<U> b);

	bool operator !() const;
	operator T*() const;
	T* operator->() const;
	T& operator*() const;
};

template<class T>
inline SafePtr<T>::SafePtr(T* objptr)
{
	InitSafePtr((SafePtrClass *)objptr);
}

template<class T>
inline SafePtr<T>::SafePtr(const SafePtr& obj)
{
	InitSafePtr(obj.ptr);
}

template<class T>
inline T *SafePtr<T>::Pointer()
{
	return (T*)SafePtrBase::Pointer();
}

template<class T>
inline SafePtr<T>& SafePtr<T>::operator=(const SafePtr& obj)
{
	InitSafePtr(obj.ptr);
	return *this;
}

template<class T>
inline SafePtr<T>& SafePtr<T>::operator=(T * const obj)
{
	InitSafePtr((SafePtrClass *)obj);
	return *this;
}

template<class T>
inline bool operator==(SafePtr<T> a, T *b)
{
	return a.ptr == b;
}

template<class T>
inline bool operator!=(SafePtr<T> a, T* b)
{
	return a.ptr != b;
}

template<class T>
inline bool operator==(T* a, SafePtr<T> b)
{
	return a == b.ptr;
}

template<class T>
inline bool operator!=(T* a, SafePtr<T> b)
{
	return a != b.ptr;
}

template<class T>
inline bool operator==(SafePtr<T> a, SafePtr<T> b)
{
	return a.ptr == b.ptr;
}

template<class T>
inline bool operator!=(SafePtr<T> a, SafePtr<T> b)
{
	return a.ptr != b.ptr;
}

template<class T>
inline bool SafePtr<T>::operator !() const
{
	return ptr == NULL;
}

template<class T>
inline SafePtr<T>::operator T*() const
{
	return (T *)ptr;
}

template<class T>
inline T* SafePtr<T>::operator->() const
{
	assert(ptr);
	return (T *)ptr;
}

template<class T>
inline T& SafePtr<T>::operator*() const
{
	assert(ptr);
	return *(T *)ptr;
}

