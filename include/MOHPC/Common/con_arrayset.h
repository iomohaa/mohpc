#pragma once

#include "con_set.h"

namespace MOHPC
{
	template< typename k, typename v >
	class con_arrayset : public con_set< k, v >
	{
	private:
		Entry< k, v >				**reverseTable;		// the index table

	protected:
		virtual Entry< k, v >		*addNewKeyEntry(const k& key);

	public:
		con_arrayset();
		~con_arrayset();

		virtual void				clear();
		virtual void				resize(int count);

		uintptr_t					findKeyIndex(const k& key);
		uintptr_t					addKeyIndex(const k& key);
		uintptr_t					addNewKeyIndex(const k& key);
		bool						remove(const k& key);

		v&							operator[](uintptr_t index);
	};

	template< typename key, typename value >
	con_arrayset<key, value>::con_arrayset()
	{
		reverseTable = (&this->defaultEntry) - 1;
	}

	template< typename key, typename value >
	con_arrayset<key, value>::~con_arrayset()
	{
		clear();
	}

	template< typename key, typename value >
	void con_arrayset<key, value>::resize(int count)
	{
		Entry< key, value > **oldReverseTable = reverseTable;
		size_t oldTableLength = this->tableLength;
		size_t i;

		con_set< key, value >::resize(count);

		// allocate a bigger reverse table
		reverseTable = (new Entry< key, value > *[this->tableLength]()) - 1;

		for (i = 1; i <= oldTableLength; i++)
		{
			reverseTable[i] = oldReverseTable[i];
		}

		if (oldTableLength > 1)
		{
			oldReverseTable++;
			delete[] oldReverseTable;
		}
	}

	template< typename key, typename value >
	void con_arrayset<key, value>::clear()
	{
		if (this->tableLength > 1)
		{
			reverseTable++;
			delete[] reverseTable;
			reverseTable = (&this->defaultEntry) - 1;
		}

		con_set< key, value >::clear();
	}


	template< typename k, typename v >
	Entry< k, v > *con_arrayset< k, v >::addNewKeyEntry(const k& key)
	{
		Entry< k, v > *entry = con_set< k, v >::addNewKeyEntry(key);

		entry->index = this->count;

		reverseTable[this->count] = entry;

		return entry;
	}

	template< typename k, typename v >
	uintptr_t con_arrayset< k, v >::addKeyIndex(const k& key)
	{
		Entry< k, v > *entry = this->addKeyEntry(key);

		return entry->index;
	}

	template< typename k, typename v >
	uintptr_t con_arrayset< k, v >::addNewKeyIndex(const k& key)
	{
		Entry< k, v > *entry = this->addNewKeyEntry(key);

		return entry->index;
	}

	template< typename k, typename v >
	uintptr_t con_arrayset< k, v >::findKeyIndex(const k& key)
	{
		Entry< k, v > *entry = this->findKeyEntry(key);

		if (entry != NULL) {
			return entry->index;
		}
		else {
			return 0;
		}
	}

	template< typename k, typename v >
	bool con_arrayset< k, v >::remove(const k& key)
	{
		for (uintptr_t i = 1; i <= this->tableLength; i++)
		{
			if (reverseTable[i] &&
				reverseTable[i]->key == key)
			{
				reverseTable[i] = NULL;
			}
		}

		return con_set< k, v >::remove(key);
	}

	template< typename key, typename value >
	value& con_arrayset< key, value >::operator[](uintptr_t index)
	{
		return reverseTable[index]->key;
	}
};
