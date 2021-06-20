#pragma once

#include "Function.h"

namespace MOHPC
{
#define MOHPC_HANDLERLIST_DEFINITIONS() \
	template<typename T> fnHandle_t set(typename T::Type&& handler) = delete; \
	template<typename T> void unset(fnHandle_t handle) = delete; \
	template<typename T, typename...Args> void notify(Args...args) const = delete

#define MOHPC_HANDLERLIST_SETTER(c, h) \
private: \
FunctionList<c::Type> h; \
public: \
template<> MOHPC_UTILITY_EXPORTS fnHandle_t set<c>(c::Type&& handler) { return h.add(std::forward<c::Type>(handler)); }\
template<> MOHPC_UTILITY_EXPORTS void unset<c>(fnHandle_t handle) { h.remove(handle); }

#define MOHPC_HANDLERLIST_NOTIFY0() template<typename T> void notify() const;
#define MOHPC_HANDLERLIST_NOTIFY1(t1) template<typename T> void notify(t1) const;
#define MOHPC_HANDLERLIST_NOTIFY2(t1, t2) template<typename T> void notify(t1, t2) const;
#define MOHPC_HANDLERLIST_NOTIFY3(t1, t2, t3) template<typename T> void notify(t1, t2, t3) const;
#define MOHPC_HANDLERLIST_NOTIFY4(t1, t2, t3, t4) template<typename T> void notify(t1, t2, t3, t4) const;
#define MOHPC_HANDLERLIST_NOTIFY5(t1, t2, t3, t4, t5) template<typename T> void notify(t1, t2, t3, t4, t5) const;
#define MOHPC_HANDLERLIST_NOTIFY6(t1, t2, t3, t4, t5, t6) template<typename T> void notify(t1, t2, t3, t4, t5, t6) const;
#define MOHPC_HANDLERLIST_NOTIFY7(t1, t2, t3, t4, t5, t6, t7) template<typename T> void notify(t1, t2, t3, t4, t5, t6, t7) const;

#define MOHPC_HANDLERLIST_HANDLER0(c, h) MOHPC_HANDLERLIST_SETTER(c, h); \
MOHPC_HANDLERLIST_NOTIFY0() \
template<> void notify<c>() const { h.broadcast(); }

#define MOHPC_HANDLERLIST_HANDLER0_NODEF(c, h) MOHPC_HANDLERLIST_SETTER(c, h); \
template<> void notify<c>() const { h.broadcast(); }

#define MOHPC_HANDLERLIST_HANDLER1(c, h, t1) MOHPC_HANDLERLIST_SETTER(c, h); \
MOHPC_HANDLERLIST_NOTIFY1(t1) \
template<> void notify<c>(t1 a1) const { h.broadcast(a1); }

#define MOHPC_HANDLERLIST_HANDLER1_NODEF(c, h, t1) MOHPC_HANDLERLIST_SETTER(c, h); \
template<> void notify<c>(t1 a1) const { h.broadcast(a1); }

#define MOHPC_HANDLERLIST_HANDLER2(c, h, t1, t2) MOHPC_HANDLERLIST_SETTER(c, h); \
MOHPC_HANDLERLIST_NOTIFY2(t1, t2) \
template<> void notify<c>(t1 a1, t2 a2) const { h.broadcast(a1, a2); }

#define MOHPC_HANDLERLIST_HANDLER2_NODEF(c, h, t1, t2) MOHPC_HANDLERLIST_SETTER(c, h); \
template<> void notify<c>(t1 a1, t2 a2) const { h.broadcast(a1, a2); }

#define MOHPC_HANDLERLIST_HANDLER3(c, h, t1, t2, t3) MOHPC_HANDLERLIST_SETTER(c, h); \
MOHPC_HANDLERLIST_NOTIFY3(t1, t2, t3) \
template<> void notify<c>(t1 a1, t2 a2, t3 a3) const { h.broadcast(a1, a2, a3); }

#define MOHPC_HANDLERLIST_HANDLER3_NODEF(c, h, t1, t2, t3) MOHPC_HANDLERLIST_SETTER(c, h); \
template<> void notify<c>(t1 a1, t2 a2, t3 a3) const { h.broadcast(a1, a2, a3); }

#define MOHPC_HANDLERLIST_HANDLER4(c, h, t1, t2, t3, t4) MOHPC_HANDLERLIST_SETTER(c, h); \
MOHPC_HANDLERLIST_NOTIFY4(t1, t2, t3, t4) \
template<> void notify<c>(t1 a1, t2 a2, t3 a3, t4 a4) const { h.broadcast(a1, a2, a3, a4); }

#define MOHPC_HANDLERLIST_HANDLER5(c, h, t1, t2, t3, t4, t5) MOHPC_HANDLERLIST_SETTER(c, h); \
MOHPC_HANDLERLIST_NOTIFY5(t1, t2, t3, t4, t5) \
template<> void notify<c>(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) const { h.broadcast(a1, a2, a3, a4, a5); }

#define MOHPC_HANDLERLIST_HANDLER6(c, h, t1, t2, t3, t4, t5, t6) MOHPC_HANDLERLIST_SETTER(c, h); \
MOHPC_HANDLERLIST_NOTIFY6(t1, t2, t3, t4, t5, t6) \
template<> void notify<c>(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6) const { h.broadcast(a1, a2, a3, a4, a5, a6); }

#define MOHPC_HANDLERLIST_HANDLER7(c, h, t1, t2, t3, t4, t5, t6, t7) MOHPC_HANDLERLIST_SETTER(c, h); \
MOHPC_HANDLERLIST_NOTIFY7(t1, t2, t3, t4, t5, t6, t7) \
template<> void notify<c>(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7) const { h.broadcast(a1, a2, a3, a4, a5, a6, a7); }

	/**
	 * Base notification class
	 */
	template<typename T>
	struct HandlerNotifyBase { using Type = std::function<T>; };

	class HandlerList
	{
	public:
		template<typename T> fnHandle_t set(typename T::Type&& handler) = delete;
		template<typename T> void unset(fnHandle_t handle) = delete;
		template<typename T, typename...Args> void notify(Args...args) = delete;
	};
}
