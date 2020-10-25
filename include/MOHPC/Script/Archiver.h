#pragma once

#include "../Common/con_set.h"

namespace MOHPC
{
	class Archiver
	{

	};


	template< class Type >
	inline void Container<Type>::Archive(Archiver& arc, void(*ArchiveFunc)(Archiver& arc, Type *obj))
	{
		/*
		int num;
		int i;

		if (arc.Loading())
		{
			arc.ArchiveInteger(&num);
			Resize(num);
		}
		else
		{
			num = numobjects;
			arc.ArchiveInteger(&num);
		}

		for (i = 1; i <= num; i++)
		{
			if (num > numobjects) {
				numobjects = num;
			}

			ArchiveFunc(arc, &objlist[i]);
		}
		*/
	}

	template< typename key, typename value >
	void con_set< key, value >::Archive(Archiver& arc)
	{
		/*
		Entry< key, value > *e;
		int hash;
		int i;

		arc.ArchiveUnsigned(&tableLength);
		arc.ArchiveUnsigned(&threshold);
		arc.ArchiveUnsigned(&count);
		arc.ArchiveUnsignedShort(&tableLengthIndex);

		if (arc.Loading())
		{
			if (tableLength != 1)
			{
				table = new Entry< key, value > *[tableLength]();
				memset(table, 0, tableLength * sizeof(Entry< key, value > *));
			}

			for (i = 0; i < count; i++)
			{
				e = new Entry< key, value >;
				e->Archive(arc);

				hash = HashCode< key >(e->key) % tableLength;

				e->index = i;
				e->next = table[hash];
				table[hash] = e;
			}
		}
		else
		{
			for (i = tableLength - 1; i >= 0; i--)
			{
				for (e = table[i]; e != NULL; e = e->next)
				{
					e->Archive(arc);
				}
			}
		}
		*/
	}

	template< typename key, typename value >
	void con_map< key, value >::Archive(Archiver& arc)
	{
		m_con_set.Archive(arc);
	}

}
