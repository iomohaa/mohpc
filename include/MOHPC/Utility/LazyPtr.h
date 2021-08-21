#pragma once

#include "SharedPtr.h"

#include <tuple>
#include <cassert>

namespace MOHPC
{
	template<typename T>
	using LazyType = SharedPtr<T>;

	template<typename T>
	class ILazyPtr_ConstructorBase
	{
	public:
		virtual ~ILazyPtr_ConstructorBase() = default;

		virtual T* create() const = 0;
	};

	template<typename T, typename...Args>
	class LazyPtr_ConstructorTemplate : public ILazyPtr_ConstructorBase<T>
	{
	private:
		// Arguments storage
		std::tuple<Args...> data;

	public:
		LazyPtr_ConstructorTemplate(Args&&...args)
			: data(std::forward<Args>(args)...)
		{}

		virtual T* create() const override
		{
			return std::apply([](Args&&... args) -> T*
			{
				return new T(std::forward<Args>(args)...);
			}, data);
		}
	};

	template<typename T>
	class LazyPtr_ObjectBase
	{
	public:
		virtual ~LazyPtr_ObjectBase() = default;
		virtual T* create() const = 0;
	};

	template<typename T, typename...Args>
	class LazyPtr_Unconstructed : public LazyPtr_ObjectBase<T>
	{
	private:
		LazyPtr_ConstructorTemplate<T, Args...> constructor;

	public:
		LazyPtr_Unconstructed(Args&&... args)
			: constructor(std::forward<Args>(args)...)
		{}

		// This is the only virtual function to avoid template instantiation with empty arguments
		virtual T* create() const override
		{
			return constructor.create();
		}
	};

	template<typename T>
	class LazyPtr_Constructed : public LazyPtr_ObjectBase<T>
	{
	private:
		LazyType<T> obj;

	public:
		LazyPtr_Constructed(LazyType<T>&& inObj)
			: obj(std::move(inObj))
		{}

		virtual T* create() const override
		{
			assert(!"This function must not be called");
			return nullptr;
		}

		// Not virtual for faster access
		const LazyType<T>& get() const
		{
			return obj;
		}
	};

	template<typename T, typename FuncType>
	class LazyPtr_RefPtr
	{
	public:
		LazyPtr_ObjectBase<T>* baseObj;
		FuncType get_pf;
		size_t ref;

	public:
		~LazyPtr_RefPtr()
		{
			delete baseObj;
		}

		void addRef()
		{
			++ref;
		}

		void delRef()
		{
			--ref;
			if (!ref) {
				delete this;
			}
		}
	};

	template<typename T>
	class LazyPtr
	{
	private:
		using get_f = const LazyType<T>& (LazyPtr::*)();

	private:
		LazyPtr_RefPtr<T, get_f>* refObj;

	public:
		template<typename...Args>
		LazyPtr(Args&&... args)
		{
			refObj = new LazyPtr_RefPtr<T, get_f>();
			refObj->addRef();
			refObj->baseObj = new LazyPtr_Unconstructed<T, Args...>(std::forward<Args>(args)...);
			refObj->get_pf = &LazyPtr::constructObject;
		}

		LazyPtr(const LazyPtr& other)
		{
			refObj = other.refObj;
			refObj->addRef();
		}

		LazyPtr(LazyPtr&& other)
		{
			refObj = other.refObj;
			other.refObj = nullptr;
		}

		~LazyPtr()
		{
			if(refObj) refObj->delRef();
		}

		const LazyType<T>& get()
		{
			return (this->*refObj->get_pf)();
		}

		T& operator*()
		{
			return *get();
		}

		T* operator->()
		{
			return get().get();
		}

	private:
		const LazyType<T>& constructObject()
		{
			LazyType<T> obj = LazyType<T>(refObj->baseObj->create());
			delete refObj->baseObj;

			LazyPtr_Constructed<T>* objCon = new LazyPtr_Constructed<T>(std::move(obj));
			refObj->baseObj = objCon;

			refObj->get_pf = &LazyPtr::getObject;
			return objCon->get();
		}

		const LazyType<T>& getObject()
		{
			const LazyPtr_Constructed<T>* objCon = (LazyPtr_Constructed<T>*)refObj->baseObj;
			return objCon->get();
		}
	};
}
