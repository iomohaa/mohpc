#include <MOHPC/SafePtr.h>

void SafePtrBase::AddReference(SafePtrClass* classPtr)
{
	//std::unique_lock<std::mutex> lock(m_mutex);

	if (!classPtr->SafePtrList)
	{
		classPtr->SafePtrList = this;
		next = prev = this;
	}
	else
	{
		next = classPtr->SafePtrList;
		prev = classPtr->SafePtrList->prev;
		classPtr->SafePtrList->prev->next = this;
		classPtr->SafePtrList->prev = this;
	}

	refCount++;
}

void SafePtrBase::RemoveReference(SafePtrClass* classPtr)
{
	//std::unique_lock<std::mutex> lock(m_mutex);

	if (classPtr->SafePtrList == this)
	{
		if (classPtr->SafePtrList->next == this)
		{
			classPtr->SafePtrList = NULL;
		}
		else
		{
			classPtr->SafePtrList = next;
			prev->next = next;
			next->prev = prev;
			next = this;
			prev = this;
		}
	}
	else
	{
		prev->next = next;
		next->prev = prev;
		next = this;
		prev = this;
	}

	refCount--;
}

void SafePtrBase::Clear(void)
{
	if (ptr)
	{
		RemoveReference(ptr);
		ptr = NULL;
	}
}

int32_t SafePtrBase::GetRefCount() const
{
	return refCount;
}

SafePtrBase::SafePtrBase()
{
	prev = NULL;
	next = NULL;
	ptr = NULL;
	refCount = 0;
}

SafePtrBase::~SafePtrBase()
{
	Clear();
}

SafePtrClass* SafePtrBase::Pointer(void)
{
	return ptr;
}

void SafePtrBase::InitSafePtr(SafePtrClass* newptr)
{
	if (ptr != newptr)
	{
		if (ptr)
		{
			RemoveReference(ptr);
		}

		ptr = newptr;
		if (ptr == NULL)
		{
			return;
		}

		AddReference(ptr);
	}
}

SafePtrClass::SafePtrClass()
{
	SafePtrList = nullptr;
}

SafePtrClass::~SafePtrClass()
{
	while (SafePtrList)
	{
		SafePtrList->Clear();
	}
}
