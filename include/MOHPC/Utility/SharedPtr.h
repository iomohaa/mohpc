#pragma once

//#include "ReferenceCounter.h"
#include <memory>

namespace MOHPC
{
#if 0
	template <class T, class = void>
	struct CanEnableShared : std::false_type {};

	template <class T>
	struct CanEnableShared<T, std::void_t<typename T::ThisType>>
		: std::is_convertible<std::remove_cv_t<T>*, typename T::ThisType*>::type
	{
	};

	template<typename T>
	void SetSharedThis(T* ptr, std::false_type);
	template<typename T>
	void SetSharedThis(T* ptr, std::true_type);

	/**
	 * Basic shared pointer class, working like the STL version
	 */
	template<typename T>
	class SharedPtr
	{
		template<typename T>
		friend class WeakPtr;

		template<typename T>
		friend class SharedPtr;

	private:
		//T* ptr;
		//ReferenceCounter* ref;
		std::shared_ptr<T> ptr;

	public:
		SharedPtr() noexcept
			: ptr(nullptr)
			//, ref(new ReferenceCounter())
		{
			AddReference();
		}

		SharedPtr(T* inPtr) noexcept
			: ptr(inPtr)
			//, ref(new ReferenceCounter())
		{
			AddReference();
			//SetSharedThis(inPtr, std::bool_constant<std::conjunction_v<CanEnableShared<T>>>{});
		}

		SharedPtr(nullptr_t) noexcept
			: ptr(nullptr)
			//, ref(new ReferenceCounter())
		{
			//AddReference();
			//SetSharedThis(inPtr, std::bool_constant<std::conjunction_v<CanEnableShared<T>>>{});
		}

		SharedPtr(const SharedPtr& sharedPtr) noexcept
		{
			ptr = sharedPtr.ptr;
			//ref = sharedPtr.ref;
			AddReference();
		}

		SharedPtr(const std::shared_ptr<T>& sharedPtr) noexcept
		{
			ptr = sharedPtr;
		}

		template<typename T2>
		SharedPtr(const SharedPtr<T2>& sharedPtr) noexcept
		{
			ptr = std::static_pointer_cast<T>(sharedPtr.ptr);
			//ref = sharedPtr.ref;
			AddReference();
		}

		SharedPtr(SharedPtr&& sharedPtr) noexcept
		{
			ptr = sharedPtr.ptr;
			//ref = sharedPtr.ref;
			sharedPtr.ptr = NULL;
			//sharedPtr.ref = NULL;
		}

		~SharedPtr()
		{
			RemoveReference();
		}

		void Reset()
		{
			//RemoveReference();
			//ref = new ReferenceCounter();
			//ptr = nullptr;
			ptr.reset();
		}

		/*
		SharedPtr& operator=(T* inPtr) noexcept
		{
			if (inPtr != ptr.get())
			{
				RemoveReference();
				//ref = new ReferenceCounter();
				ptr = inPtr;
				AddReference();
			}
			return *this;
		}
		*/

		template<typename T2>
		SharedPtr& operator=(const SharedPtr<T2>& sharedPtr) noexcept
		{
			RemoveReference();
			ptr = sharedPtr.ptr;
			//ref = sharedPtr.ref;
			AddReference();
			return *this;
		}

		SharedPtr& operator=(const SharedPtr& sharedPtr) noexcept
		{
			RemoveReference();
			ptr = sharedPtr.ptr;
			//ref = sharedPtr.ref;
			AddReference();
			return *this;
		}

		SharedPtr& operator=(SharedPtr&& sharedPtr) noexcept
		{
			ptr = std::move(sharedPtr.ptr);
			//ref = sharedPtr.ref;
			sharedPtr.ptr = NULL;
			//sharedPtr.ref = NULL;
			// No reference change
			return *this;
		}

		bool operator==(const SharedPtr& sharedPtr) noexcept
		{
			return Get() == sharedPtr.Get();
		}

		bool operator==(nullptr_t null) noexcept
		{
			return Get() == nullptr;
		}

		bool operator!=(const SharedPtr& sharedPtr) noexcept
		{
			return !(*this == sharedPtr);
		}

		bool operator!=(nullptr_t null) noexcept
		{
			return !(*this == null);
		}

		explicit operator bool() const noexcept
		{
			return ptr != nullptr;
		}

		operator T* () const noexcept
		{
			return ptr.get();
		}

		T* Get() const noexcept
		{
			return ptr.get();
		}

		T* operator->() const noexcept
		{
			return ptr.get();
		}

		T& operator*() const noexcept
		{
			return *ptr.get();
		}
	private:
		void AddReference() noexcept
		{
			//ref->AddSharedRef();
		}

		void RemoveReference() noexcept
		{
			/*
			if (ref)
			{
				if (ref->ReleaseSharedRef()) {
					if (ptr) delete ptr;
				}
				ref = nullptr;
			}
			*/
		}
	};

	template<typename T>
	class EnableSharedFromThis : public std::enable_shared_from_this<T>
	{
		using ThisType = EnableSharedFromThis;

	//private:
		//WeakPtr<T> ptr;

	protected:
		EnableSharedFromThis()
		{
		}

		SharedPtr<T> SharedFromThis()
		{
			return SharedPtr<T>(shared_from_this());
		}

		SharedPtr<T> WeakFromThis()
		{
			return SharedPtr<T>(weak_from_this());
		}
	};

	template<typename T>
	void SetSharedThis(T* ptr, std::true_type)
	{
		ptr->ptr = ptr;
	}

	template<typename T>
	void SetSharedThis(T* ptr, std::false_type)
	{
	}

	template<typename T, typename ...Args>
	SharedPtr<T> MakeShared(Args&&... args)
	{
		return SharedPtr<T>(new T(args...));
	}

	template<typename T>
	SharedPtr<T> MakeShared(T* ptr)
	{
		assert(ptr);
		return SharedPtr<T>(ptr);
	}
#endif

	template<typename T>
	using SharedPtr = std::shared_ptr<T>;

	template<typename T>
	using EnableSharedFromThis = std::enable_shared_from_this<T>;

	template<typename T, typename... Args>
	inline constexpr auto staticPointerCast(Args&&... args) -> decltype(std::static_pointer_cast<T>(std::forward<Args>(args)...))
	{
		return std::static_pointer_cast<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline constexpr auto constPointerCast(Args&&... args) -> decltype(std::const_pointer_cast<T>(std::forward<Args>(args)...))
	{
		return std::const_pointer_cast<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline constexpr auto reinterpretPointerCast(Args&&... args) -> decltype(std::reinterpret_pointer_cast<T>(std::forward<Args>(args)...))
	{
		return std::reinterpret_pointer_cast<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline constexpr auto dynamicPointerCast(Args&&... args) -> decltype(std::dynamic_pointer_cast<T>(std::forward<Args>(args)...))
	{
		return std::dynamic_pointer_cast<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline constexpr auto makeShared(Args&&... args) -> decltype(std::make_shared<T>(std::forward<Args>(args)...))
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}