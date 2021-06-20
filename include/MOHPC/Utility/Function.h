#pragma once

#include <morfuse/Container/Container.h>
#include <functional>

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
			FnStorage(uint32_t inId, FunctionType&& inFunc)
				: func(std::forward<FunctionType>(inFunc))
				, id(inId)
			{}

			FnStorage(FnStorage&& other) = default;
			FnStorage& operator=(FnStorage&& other) = default;
			// Non-copyable
			FnStorage(const FnStorage& other) = delete;
			FnStorage& operator=(const FnStorage& other) = delete;
		};

	private:
		mfuse::con::Container<FnStorage> functionList;
		fnHandle_t cid;

	public:
		FunctionList()
			: cid(0)
		{}

		/**
		 * Add a function to the function list.
		 *
		 * @param func The function to add.
		 */
		fnHandle_t add(FunctionType&& func)
		{
			// Add the function and return the id
			new(functionList) FnStorage(++cid, std::forward<FunctionType>(func));

			// Return the handle
			return cid;
		}

		/**
		 * Remove a function from the function list.
		 *
		 * @param handle Handle of the function that was returned by add().
		 */
		void remove(fnHandle_t handle)
		{
			for (size_t i = functionList.NumObjects(); i > 0; i--)
			{
				FnStorage& fn = functionList[i - 1];
				if (fn.id == handle)
				{
					functionList.RemoveObjectAt(i);
					return;
				}
			}
		}

		/**
		 * Broadcast to all functions.
		 *
		 * @param ...args List of arguments
		 */
		template<typename...Args>
		void broadcast(Args&&... args) const
		{
			// Call all registered functions
			for (size_t i = functionList.NumObjects(); i > 0; i--)
			{
				const FnStorage& fn = functionList[i - 1];
				fn.func(std::forward<Args>(args)...);
			}
		}
	};
}
