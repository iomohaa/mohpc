#pragma once

#include <functional>

namespace MOHPC
{
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
}