#pragma once

#include "UtilityGlobal.h"
#include <vector>
#include <functional>

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
	/*
	template <class>
	// false value attached to a dependent name (for static_assert)
	constexpr bool AlwaysFalse = false;

	template<typename RetType, typename... Types>
	class FunctionBase
	{
	protected:
		using Type = RetType(Types...);

	private:
		std::function<Type>* func;

	public:
		template<typename...Args>
		FunctionBase(Args&&...args) {
			func = new std::function<Type>(std::forward<Args>(args)...);
		}

		FunctionBase(FunctionBase&& other) {
			func = other.func;
			other.func = nullptr;
		}

		explicit operator bool() const noexcept {
			return func->target();
		}

		RetType operator()(Types... args) const {
			func(std::forward<Types>(args)...);
		}
	};

	template<typename T>
	struct GetFunctionArgs {
		static_assert(AlwaysFalse<T>, "must be function template");
	};

	template<typename RetType, typename... Types>
	struct GetFunctionArgs<RetType(Types...)> {
		using Type = FunctionBase<RetType, Types...>;
	};

	template<typename T>
	class Function : public GetFunctionArgs<T>::Type
	{
	public:
		Function(std::function<Type>&& inFunc)
			: FunctionBase(std::move(inFunc))
		{}
	};
	*/

	template<typename T>
	using Function = std::function<T>;

	using fnHandle_t = uint32_t;

	template<typename T>
	class FunctionList
	{
		using FunctionType = typename T::Type;

	private:
		struct FnStorage
		{
			FunctionType func;
			fnHandle_t id;

		public:
			FnStorage(uint32_t inId, FunctionType&& inFunc);

			FnStorage(FnStorage&& other);
			FnStorage& operator=(FnStorage&& other);
			// Non-copyable
			FnStorage(const FnStorage& other) = delete;
			FnStorage& operator=(const FnStorage& other) = delete;
		};
	public:
		FunctionList();
		~FunctionList();

		/**
		 * Add a function to the function list.
		 *
		 * @param func The function to add.
		 */
		fnHandle_t add(FunctionType&& func);

		/**
		 * Remove a function from the function list.
		 *
		 * @param handle Handle of the function that was returned by add().
		 */
		void remove(fnHandle_t handle);

		/**
		 * Broadcast to all functions.
		 *
		 * @param ...args List of arguments
		 */
		template<typename...Args>
		void broadcast(Args&&... args) const;

	private:
		std::vector<FnStorage> functionList;
		fnHandle_t cid;
	};

	template<typename T>
	FunctionList<T>::FunctionList()
		: cid(0)
	{}

	template<typename T>
	FunctionList<T>::~FunctionList()
	{}

	template<typename T>
	FunctionList<T>::FnStorage::FnStorage(uint32_t inId, FunctionType&& inFunc)
		: func(std::forward<FunctionType>(inFunc))
		, id(inId)
	{}

	template<typename T>
	FunctionList<T>::FnStorage::FnStorage(FnStorage&& other)
		: func(std::move(other.func))
		, id(other.id)
	{}

	template<typename T>
	typename FunctionList<T>::FnStorage& FunctionList<T>::FnStorage::operator=(FnStorage&& other)
	{
		func = std::move(other.func);
		id = other.id;
		return *this;
	}

	template<typename T>
	fnHandle_t FunctionList<T>::add(FunctionType&& func)
	{
		// Add the function and return the id
		functionList.emplace_back(++cid, std::forward<FunctionType>(func));

		// Return the handle
		return cid;
	}

	template<typename T>
	void FunctionList<T>::remove(fnHandle_t handle)
	{
		for (auto it = functionList.begin(); it != functionList.end(); ++it)
		{
			const FnStorage& fn = *it;
			if (fn.id == handle)
			{
				functionList.erase(it);
				return;
			}
		}
	}

	template<typename T>
	template<typename...Args>
	void FunctionList<T>::broadcast(Args&&... args) const
	{
		// Call all registered functions
		for (auto it = functionList.begin(); it != functionList.end(); ++it)
		{
			it->func(std::forward<Args>(args)...);
		}
	}

#define MOHPC_FUNCTIONLIST_TEMPLATE(template_def, exports_def, type) \
	template_def template exports_def fnHandle_t FunctionList<type>::add(type::Type&& other); \
	template_def template exports_def void FunctionList<type>::remove(fnHandle_t handle)
}
