#pragma once

#include <stdint.h>
#include <cassert>
#include <utility>
#include <exception>
#include "Memory.h"

#undef new

namespace MOHPC
{
	class Archiver;

	class ContainerException : public std::exception
	{
	};

	class OutOfRangeContainerException : public ContainerException
	{
	private:
		size_t badIndex;

	public:
		OutOfRangeContainerException(size_t inBadIndex) noexcept
			: badIndex(inBadIndex)
		{}

		size_t getBadIndex() const noexcept { return badIndex; }
	};

	template< class Type >
	class Container
	{
	private:
		Type * objlist;
		size_t numobjects;
		size_t maxobjects;

	private:
		void				Copy(const Container<Type>& container);

	public:
		Container() noexcept;
		Container(size_t initialSize);
		Container(const Container<Type>& container);
		Container(Container<Type>&& container) noexcept;
		~Container();

		void				Archive(Archiver& arc);
		void				Archive(Archiver& arc, void(*ArchiveFunc)(Archiver &arc, Type *obj));

		uintptr_t			AddObject(const Type& obj);
		uintptr_t			AddObject();
		uintptr_t			AddObjectUninitialized();
		uintptr_t			AddUniqueObject(const Type& obj);
		void				AddObjectAt(size_t index, const Type& obj);
		Type				*AddressOfObjectAt(size_t index);
		Type				*Data() noexcept;
		const Type			*Data() const noexcept;
		//	void				Archive( Archiver &arc );
		void				ClearObjectList();
		void				FreeObjectList();
		uintptr_t			IndexOfObject(const Type& obj) noexcept;
		void				InsertObjectAt(size_t index, const Type& obj);
		uintptr_t			MaxObjects() const noexcept;
		uintptr_t			NumObjects() const noexcept;
		Type&				ObjectAt(const size_t index) const;
		bool				ObjectInList(const Type& obj);
		void				RemoveObjectAt(uintptr_t index);
		void				RemoveObject(const Type& obj);
		void				RemoveObject(const Type* obj);
		void				Resize(size_t maxelements);
		void				SetNumObjects(size_t numelements);
		void				SetNumObjectsUninitialized(size_t numelements);
		void				SetObjectAt(size_t index, const Type& obj);
		void				Shrink();
		void				Sort(int(*compare)(const void *elem1, const void *elem2));
		Type&				operator[](const size_t index) const;
		Container<Type>&	operator=(const Container<Type>& container);
		Container<Type>&	operator=(Container<Type>&& container) noexcept;

		// STL functions
		Type* begin() noexcept { return objlist; }
		const Type* begin() const noexcept { return objlist; }
		Type* end() noexcept { return objlist + numobjects; }
		const Type* end() const noexcept { return objlist + numobjects; }
		Type* data()  noexcept { return objlist; }
		const Type* data() const noexcept { return objlist; }
		size_t size() const noexcept { return numobjects; }
		const Type& at(const size_t index) const { return ObjectAt(index + 1); }
		Type& at(const size_t index) { return ObjectAt(index + 1); }
		void push_back(const Type& obj) { AddObject(obj); }
		void clear() noexcept { ClearObjectList(); }
		void reserve(size_t newSize) { Resize(newSize); }
		void resize(size_t newSize) { SetNumObjects(newSize); }
		void shrink_to_fit() { Shrink(); }
	};

	template< class Type >
	Container<Type>::Container() noexcept
	{
		objlist = NULL;
		numobjects = 0;
		maxobjects = 0;
	}

	template< class Type >
	Container<Type>::Container(size_t initialSize)
		: Container<Type>()
	{
		Resize(initialSize);
	}

	template< class Type >
	Container<Type>::Container(const Container<Type>& container)
	{
		objlist = NULL;

		Copy(container);
	}

	template< class Type >
	Container<Type>::Container(Container<Type>&& container) noexcept
	{
		*this = std::move_if_noexcept(container);
	}

	template< class Type >
	Container<Type>::~Container()
	{
		FreeObjectList();
	}

	template< class Type >
	uintptr_t Container<Type>::AddObject(const Type& obj)
	{
		if (!objlist) {
			Resize(10);
		}

		if (numobjects >= maxobjects) {
			Resize(numobjects * 2);
		}

		new(&objlist[numobjects++]) Type(obj);

		return numobjects;
	}

	template< class Type >
	uintptr_t Container<Type>::AddObject()
	{
		const uintptr_t index = AddObjectUninitialized() - 1;
		new (objlist + index) Type();
		return index;
	}

	template< class Type >
	uintptr_t Container<Type>::AddObjectUninitialized()
	{
		if (!objlist) {
			Resize(10);
		}

		if (numobjects >= maxobjects) {
			Resize(numobjects * 2);
		}

		++numobjects;
		return numobjects;
	}

	template< class Type >
	uintptr_t Container<Type>::AddUniqueObject(const Type& obj)
	{
		uintptr_t index = IndexOfObject(obj);

		if (!index)
		{
			index = AddObject(obj);
		}

		return index;
	}

	template< class Type >
	void Container<Type>::AddObjectAt(size_t index, const Type& obj)
	{
		if (index > maxobjects)
			Resize(index);

		if (index > numobjects)
			numobjects = index;

		SetObjectAt(index, obj);
	}

	template< class Type >
	Type *Container<Type>::AddressOfObjectAt(size_t index)
	{
		assert(index <= maxobjects);

		if (index > maxobjects) {
			//CONTAINER_Error(ERR_DROP, "Container::AddressOfObjectAt : index is greater than maxobjects");
		}

		if (index > numobjects) {
			numobjects = index;
		}

		return &objlist[index - 1];
	}

	/*template< class Type >
	void Container<Type>::Archive( Archiver &arc )
	{

	}*/

	template< class Type >
	void Container<Type>::ClearObjectList()
	{
		if (objlist && numobjects)
		{
			for (size_t i = 0; i < numobjects; ++i) {
				objlist[i].~Type();
			}

			freeMemory(objlist);

			if (maxobjects == 0)
			{
				objlist = NULL;
				return;
			}

			objlist = (Type*)allocateMemory(sizeof(Type) * maxobjects);
			numobjects = 0;
		}
	}

	template< class Type >
	Type* Container<Type>::Data() noexcept
	{
		return objlist;
	}

	template< class Type >
	const Type* Container<Type>::Data() const noexcept
	{
		return objlist;
	}

	template< class Type >
	void Container<Type>::FreeObjectList()
	{
		if (objlist)
		{
			for (size_t i = 0; i < numobjects; ++i) {
				objlist[i].~Type();
			}

			freeMemory(objlist);
		}

		objlist = NULL;
		numobjects = 0;
		maxobjects = 0;
	}

	template< class Type >
	uintptr_t Container<Type>::IndexOfObject(const Type& obj) noexcept
	{
		if (!objlist) {
			return 0;
		}

		const Type* start = objlist;
		const Type* end = objlist + numobjects;
		for (const Type* data = start; data != end; ++data)
		{
			if (*data == obj) {
				return static_cast<uintptr_t>(data - start) + 1;
			}
		}

		return 0;
	}

	template< class Type >
	void Container<Type>::InsertObjectAt(size_t index, const Type& obj)
	{
		if ((index <= 0) || (index > numobjects + 1))
		{
			//CONTAINER_Error(ERR_DROP, "Container::InsertObjectAt : index out of range");
			return;
		}

		numobjects++;
		intptr_t arrayIndex = index - 1;

		if (numobjects > maxobjects)
		{
			maxobjects = numobjects;
			if (!objlist)
			{
				objlist = allocateMemory(sizeof(Type) * maxobjects);
				for (size_t i = 0; i < arrayIndex; ++i) {
					new(objlist + i) Type();
				}

				objlist[arrayIndex] = obj;
				return;
			}
			else
			{
				Type *temp = objlist;
				if (maxobjects < numobjects) {
					maxobjects = numobjects;
				}

				objlist = allocateMemory(sizeof(Type) * maxobjects);

				for (intptr_t i = 0; i < arrayIndex; ++i) {
					new(objlist + i) Type(std::move_if_noexcept(temp[i]));
				}

				new(objlist + arrayIndex) Type(obj);
				for (intptr_t i = arrayIndex + 1; i < numobjects; ++i) {
					new(objlist + i) Type(std::move_if_noexcept(temp[i]));
				}

				freeMemory(temp);
			}
		}
		else
		{
			for (int i = numobjects - 1; i > arrayIndex; i--) {
				objlist[i] = objlist[i - 1];
			}
			objlist[arrayIndex] = obj;
		}
	}

	template< class Type >
	uintptr_t Container<Type>::MaxObjects() const noexcept
	{
		return maxobjects;
	}

	template< class Type >
	uintptr_t Container<Type>::NumObjects() const noexcept
	{
		return numobjects;
	}

	template< class Type >
	Type& Container<Type>::ObjectAt(const size_t index) const
	{
		assert(index > 0 && index <= numobjects);
		if ((index <= 0) || (index > numobjects)) {
			throw OutOfRangeContainerException(index);
		}

		return objlist[index - 1];
	}

	template< class Type >
	bool Container<Type>::ObjectInList(const Type& obj)
	{
		if (!IndexOfObject(obj)) {
			return false;
		}

		return true;
	}

	template< class Type >
	void Container<Type>::RemoveObjectAt(uintptr_t index)
	{
		uintptr_t i;

		if ((index <= 0) || (index > numobjects)) {
			throw OutOfRangeContainerException(index);
		}

		i = index - 1;
		numobjects--;

		for (i = index - 1; i < numobjects; i++) {
			objlist[i] = std::move_if_noexcept(objlist[i + 1]);
		}

		// Destroy the last object as it's now useless
		objlist[numobjects].~Type();
	}

	template< class Type >
	void Container<Type>::RemoveObject(const Type& obj)
	{
		uintptr_t index = IndexOfObject(obj);
		if (!index) {
			return;
		}

		RemoveObjectAt(index);
	}

	template< class Type >
	void Container<Type>::RemoveObject(const Type* obj)
	{
		uintptr_t index = obj - objlist;
		if (index > numobjects) {
			return;
		}

		RemoveObjectAt(index + 1);
	}

	template< class Type >
	void Container<Type>::Resize(size_t maxelements)
	{
		if (maxelements <= 0)
		{
			FreeObjectList();
			return;
		}

		if (!objlist)
		{
			maxobjects = maxelements;
			// allocate without initializing
			objlist = (Type*)allocateMemory(sizeof(Type) * maxobjects);
		}
		else
		{
			Type* temp = objlist;

			maxobjects = maxelements;

			if (maxobjects < numobjects) {
				maxobjects = numobjects;
			}

			// allocate without initializing
			objlist = (Type*)allocateMemory(maxobjects * sizeof(Type));

			for (size_t i = 0; i < numobjects; i++)
			{
				// move the older type
				new(objlist + i) Type(std::move_if_noexcept(temp[i]));

				// destruct the older type
				temp[i].~Type();
			}

			freeMemory(temp);
		}
	}

	template< class Type >
	void Container<Type>::SetNumObjects(size_t numelements)
	{
		Resize(numelements);

		const size_t startNum = numobjects;
		numobjects = numelements;
		for (size_t i = startNum; i < numobjects; ++i) {
			new(objlist + i) Type();
		}
	}

	template< class Type >
	void Container<Type>::SetNumObjectsUninitialized(size_t numelements)
	{
		Resize(numelements);
		numobjects = numelements;
	}

	template< class Type >
	void Container<Type>::SetObjectAt(size_t index, const Type& obj)
	{
		assert(index > 0 && index <= numobjects);
		if ((index <= 0) || (index > numobjects)) {
			throw OutOfRangeContainerException(index);
		}

		objlist[index - 1] = obj;
	}

	template< class Type >
	void Container<Type>::Shrink()
	{
		if (!objlist || !numobjects) {
			return;
		}

		Type* newlist = (Type*)allocateMemory(sizeof(Type) * numobjects);

		for (size_t i = 0; i < numobjects; i++)
		{
			new(newlist + i) Type(std::move_if_noexcept(objlist[i]));
			objlist[i].~Type();
		}

		freeMemory(objlist);
		objlist = newlist;
		maxobjects = numobjects;
	}

	template< class Type >
	void Container<Type>::Sort(int(*compare)(const void *elem1, const void *elem2))

	{
		if (!objlist) {
			return;
		}

		qsort((void *)objlist, (size_t)numobjects, sizeof(Type), compare);
	}

	template< class Type >
	Type& Container<Type>::operator[](const size_t index) const
	{
		return ObjectAt(index + 1);
	}

	template< class Type >
	void Container<Type>::Copy(const Container<Type>& container)
	{
		if (&container == this) {
			return;
		}

		FreeObjectList();

		numobjects = container.numobjects;
		maxobjects = container.maxobjects;
		objlist = NULL;

		if (container.objlist == NULL || !container.maxobjects) {
			return;
		}

		Resize(maxobjects);

		if (!container.numobjects) {
			return;
		}

		for (size_t i = 0; i < container.numobjects; i++) {
			new(objlist + i) Type(container.objlist[i]);
		}

		return;
	}

	template< class Type >
	Container<Type>& Container<Type>::operator=(const Container<Type>& container)
	{
		Copy(container);

		return *this;
	}

	template< class Type >
	Container<Type>& Container<Type>::operator=(Container<Type>&& container) noexcept
	{
		objlist = container.objlist;
		numobjects = container.numobjects;
		maxobjects = container.maxobjects;

		// reset the old container
		container.objlist = NULL;
		container.numobjects = 0;
		container.maxobjects = 0;
		return *this;
	}

	template<typename Archive, typename T>
	Archive& operator<<(Archive& ar, const Container<T>& container)
	{
		const size_t numObjects = container.NumObjects();
		ar << numObjects;
		
		for (size_t i = 0; i < numObjects; i++)
		{
			T& obj = container[i];
			ar << obj;
		}

		return ar;
	}

	template<typename Archive, typename T>
	Archive& operator>>(Archive& ar, Container<T>& container)
	{
		size_t numObjects;
		ar >> numObjects;

		container.SetNumObjectsUninitialized(numObjects);

		for (size_t i = 0; i < numObjects; i++)
		{
			T* obj = new(container) T();
			ar >> *obj;
		}

		return ar;
	}
};

template<typename T>
void* operator new(size_t count, MOHPC::Container<T>& container)
{
	assert(count == sizeof(T));
	return &container.ObjectAt(
		container.AddObjectUninitialized()
	);
}

template<typename T>
void operator delete(void* ptr, MOHPC::Container<T>& container)
{
	container.RemoveObject((T*)ptr);
}

