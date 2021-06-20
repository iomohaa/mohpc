#pragma once

#include <type_traits>

namespace MOHPC
{
	namespace LinkList
	{
		using namespace std;

		template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
		bool Empty(T rootnode)
		{
			return ((rootnode->*next) == (rootnode)) &&
				((rootnode->*prev) == (rootnode));
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void Reset(T rootnode)
		{
			(rootnode->*next) = (rootnode->*prev) = rootnode;
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void Add(T rootnode, const T& newnode)
		{
			(newnode->*next) = rootnode;
			(newnode->*prev) = (rootnode->*prev);
			(rootnode->*prev->*next) = newnode;
			(rootnode->*prev) = newnode;
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void AddFirst(T rootnode, const T& newnode)
		{
			(newnode->*prev) = rootnode;
			(newnode->*next) = (rootnode->*next);
			(rootnode->*next->*prev) = newnode;
			(rootnode->*next) = newnode;
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void Remove(const T& node)
		{
			(node->*prev->*next) = (node->*next);
			(node->*next->*prev) = (node->*prev);
			(node->*next) = node;
			(node->*prev) = node;
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void Transfer(T oldroot, T newroot, const T& newnode)
		{
			if (oldroot->*prev != oldroot)
			{
				(oldroot->*prev->*next) = newroot;
				(oldroot->*next->*prev) = (newroot->*prev);
				(newroot->*prev->*next) = (oldroot->*next);
				(newroot->*prev) = (oldroot->*prev);
				(oldroot->*next) = oldroot;
				(oldroot->*prev) = oldroot;
			}
		}

		template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
		void Move(const T& node, T newroot)
		{
			Remove<T, next, prev>(node);
			Add<T, next, prev>(newroot, node);
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void Reverse(T root)
		{
			T* newend, * trav, * tprev;

			newend = root->next;
			for (trav = root->prev; trav != newend; trav = tprev)
			{
				tprev = trav->prev;
				Move<T, next, prev>(trav, newend);
			}
		}

		template<typename T>
		void SafeAdd(T& rootnode, const T& newnode, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev)
		{
			(newnode->*next) = T();
			(newnode->*prev) = rootnode;
			if (rootnode) (rootnode->*next) = newnode;
			rootnode = newnode;
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void SafeAddFirst(T& rootnode, const T& newnode)
		{
			(newnode->*prev) = T();
			(newnode->*next) = rootnode;
			if (rootnode) (rootnode->*prev) = newnode;
			rootnode = newnode;
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void SafeRemove(const T& node)
		{
			if ((node->*prev)) (node->*prev->*next) = (node->*next);
			if ((node->*next)) (node->*next->*prev) = (node->*prev);
		}

		template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
		void SafeRemoveRoot(T& rootnode, const T& node)
		{
			if ((node->*prev)) (node->*prev->*next) = (node->*next);
			if ((node->*next)) (node->*next->*prev) = (node->*prev);
			if (rootnode == node) rootnode = (node->*prev);
		}
	}
	namespace LL = LinkList;
}

#define LL_SortedInsertion(rootnode,insertnode,next,prev,type,sortparm) \
   {                                                                    \
   type *hoya;                                                          \
                                                                        \
   hoya = rootnode->next;                                               \
   while((hoya != rootnode) && (insertnode->sortparm > hoya->sortparm)) \
      {                                                                 \
      hoya = hoya->next;                                                \
      }                                                                 \
   LL_Add(hoya,insertnode,next,prev);                               \
   }
