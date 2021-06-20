#pragma once

namespace MOHPC
{
	template<typename T>
	class ClassList
	{
	public:
		ClassList()
		{
			prev = tail;
			next = nullptr;
			if (!head) head = this;
			if (tail) tail->next = this;
			tail = this;
		}

		~ClassList()
		{
			if (next) next->prev = prev;
			if (prev) prev->next = next;
			if (head == this) head = next;
			if (tail == this) tail = prev;
		}

		static T* getHead()
		{
			return static_cast<T*>(head);
		}

		static T* getTail()
		{
			return static_cast<T*>(tail);
		}

		T* getNext() const
		{
			return static_cast<T*>(next);
		}

		T* getPrev() const
		{
			return static_cast<T*>(prev);
		}

	private:
		static ClassList* head;
		static ClassList* tail;
		ClassList* next;
		ClassList* prev;
	};

	template<typename T> ClassList<T>* ClassList<T>::head;
	template<typename T> ClassList<T>* ClassList<T>::tail;
}