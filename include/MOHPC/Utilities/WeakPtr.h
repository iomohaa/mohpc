#pragma once

//#include "ReferenceCounter.h"
#//include "SharedPtr.h"
#include <memory>

namespace MOHPC
{
#if 0
	/**
	 * Weak reference pointer, working like the STL version
	 */
	template<typename T>
	class WeakPtr
	{
	private:
		//T* ptr;
		//ReferenceCounter* ref;
		std::weak_ptr<T> ptr;

	public:
		WeakPtr() noexcept
			: ptr()
			//, ref(new ReferenceCounter())
		{
			AddReference();
		}

		WeakPtr(T* inPtr) noexcept
			: ptr(inPtr)
			//, ref(new ReferenceCounter())
		{
			AddReference();
		}

		WeakPtr(const SharedPtr<T>& sharedPtr) noexcept
		{
			ptr = sharedPtr.ptr;
			//ref = sharedPtr.ref;
			AddReference();
		}

		WeakPtr(const WeakPtr& weakPtr) noexcept
		{
			ptr = weakPtr.ptr;
			//ref = weakPtr.ref;
			AddReference();
		}

		WeakPtr(const std::weak_ptr<T>& weakPtr) noexcept
		{
			ptr = weakPtr;
			//ref = weakPtr.ref;
			AddReference();
		}

		WeakPtr(WeakPtr&& weakPtr) noexcept
		{
			ptr = weakPtr.ptr;
			//ref = weakPtr.ref;
			weakPtr.ptr = NULL;
			weakPtr.ref = NULL;
		}

		~WeakPtr() noexcept
		{
			RemoveReference();
		}

		void Reset()
		{
			RemoveReference();
			//ref = nullptr;
			ptr = nullptr;
		}

		SharedPtr<T> Lock() noexcept
		{
			//if (!ref) {
			//	return nullptr;
			//}
			SharedPtr<T> sharedPtr;
			//sharedPtr.ref = ref;
			sharedPtr.ptr = ptr.lock();
			sharedPtr.AddReference();
			return sharedPtr;
		}

		WeakPtr& operator=(const WeakPtr& weakPtr) noexcept
		{
			//RemoveReference();
			//ptr = WeakPtr.ptr;
			//ref = WeakPtr.ref;
			//AddReference();
			ptr = weakPtr.ptr;
			return *this;
		}

		WeakPtr& operator=(WeakPtr&& weakPtr) noexcept
		{
			//ptr = WeakPtr.ptr;
			//ref = WeakPtr.ref;
			//WeakPtr.ptr = NULL;
			//WeakPtr.ref = NULL;
			ptr = weakPtr.ptr;
			// No reference change
			return *this;
		}

		bool operator==(const WeakPtr& weakPtr) noexcept
		{
			return Get() == weakPtr.Get();
		}

		bool operator==(nullptr_t null) noexcept
		{
			return Get() == nullptr;
		}

		bool operator!=(const WeakPtr& weakPtr) noexcept
		{
			return !(*this == weakPtr);
		}

		bool operator!=(nullptr_t null) noexcept
		{
			return !(*this == null);
		}

		explicit operator bool() const noexcept
		{
			//return ref && ref->Count() && ptr != nullptr;
			return !ptr.expired();
		}

		operator T* () const noexcept
		{
			return ptr.lock().get();
		}

		T* Get() const noexcept
		{
			//return ref && ref->Count() ? ptr : nullptr;
			return ptr.lock().get();
		}

		T* operator->() const noexcept
		{
			return ptr;
		}

		T& operator*() const noexcept
		{
			return *ptr;
		}
	private:
		void AddReference() noexcept
		{
			//ref->AddWeakRef();
		}

		void RemoveReference() noexcept
		{
			/*
			if (ref)
			{
				ref->ReleaseWeakRef();
				ref = nullptr;
			}
			*/
		}
	};
#endif
	template<typename T>
	using WeakPtr = std::weak_ptr<T>;
}