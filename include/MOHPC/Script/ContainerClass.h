#pragma once

#include "Container.h"

namespace MOHPC
{
	class Archiver;

	template< class Type >
	class ContainerClass : public Class {
		Container< Type > value;

	public:
		virtual ~ContainerClass() { value.FreeObjectList(); }

		virtual void		Archive(Archiver& arc);

		uintptr_t			AddObject(const Type& obj) { return value.AddObject(obj); }
		uintptr_t			AddUniqueObject(const Type& obj) { return value.AddUniqueObject(obj); }
		void				AddObjectAt(int index, const Type& obj) { return value.AddObjectAt(index, obj); }
		Type				*AddressOfObjectAt(int index) { return value.AddressOfObjectAt(index); }

		void				ClearObjectList(void) { return value.ClearObjectList(); }
		void				Fix(void) { return value.Fix(); }
		void				FreeObjectList(void) { return value.FreeObjectList(); }
		uintptr_t			IndexOfObject(const Type& obj) { return value.IndexOfObject(obj); }
		void				InsertObjectAt(int index, const Type& obj) { return value.InsertObjectAt(index, obj); }
		size_t				NumObjects(void) const { return value.NumObjects(); }
		Type&				ObjectAt(const size_t index) const { return value.ObjectAt(index); }
		bool				ObjectInList(const Type& obj) { return value.ObjectInList(obj); }
		void				RemoveObjectAt(uintptr_t index) { return value.RemoveObjectAt(index); }
		void				RemoveObject(const Type& obj) { return value.RemoveObject(obj); }
		void				Reset(void) { return value.Reset(); }
		void				Resize(size_t maxelements) { return value.Resize(maxelements); }
		void				SetObjectAt(int index, const Type& obj) { return value.SetObjectAt(index, obj); }
		void				Sort(int(*compare)(const void *elem1, const void *elem2)) { return value.Sort(compare); }
		Type&				operator[](const int index) const { return value[index]; }
		Container<Type>&	operator=(const Container<Type>& container) { return value = container; }
	};
};
