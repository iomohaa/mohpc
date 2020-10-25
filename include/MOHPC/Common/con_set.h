#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace MOHPC
{
	class Class;
	class Archiver;

	template< typename key, typename value >
	class con_map_enum;

	template< typename key, typename value >
	class con_set_enum;

	template<typename T>
	struct con_set_is_pointer { static const bool con_value = false; };

	template<typename T>
	struct con_set_is_pointer<T*> { static const bool con_value = true; };

	template< typename k, typename v >
	class Entry
	{
	public:
		k						key;
		v						value;
		uintptr_t				index;

		Entry					*next;

	public:
		//void *operator new( size_t size );
		//void operator delete( void *ptr );

		Entry(const k& inKey);

		void			Archive(Archiver& arc);
	};

	template< typename k, typename v >
	class con_set
	{
		friend class con_set_enum < k, v >;

	public:
		//static MEM_BlockAlloc< Entry< k, v >, MEM_BLOCKSIZE > Entry_allocator;

	protected:
		Entry< k, v >				**table;			// hashtable
		uintptr_t					tableLength;
		uintptr_t					threshold;
		uintptr_t					count;				// num of entries
		short unsigned int			tableLengthIndex;
		Entry< k, v >				*defaultEntry;

	protected:
		Entry< k, v >*			findKeyEntry(const k& key);
		const Entry< k, v >*	findKeyEntry(const k& key) const;
		Entry< k, v >*			addKeyEntry(const k& key);
		virtual Entry< k, v >*	addNewKeyEntry(const k& key);

	public:
		con_set();
		virtual ~con_set();

		void						Archive(Archiver& arc);

		virtual void				clear();
		virtual void				resize(int count = 0);

		v*							findKeyValue(const k& key);
		const v*					findKeyValue(const k& key) const;
		k*							firstKeyValue();

		v&							addKeyValue(const k& key);
		v&							addNewKeyValue(const k& key);

		bool						keyExists(const k& key);
		bool						isEmpty();
		bool						remove(const k& key);

		uintptr_t					size();
	};

	template< typename k, typename v >
	const v* MOHPC::con_set<k, v>::findKeyValue(const k& key) const
	{
		const Entry< k, v >* entry = findKeyEntry(key);

		if (entry != nullptr) {
			return &entry->value;
		}
		else {
			return nullptr;
		}
	}

	template< typename key, typename value >
	class con_set_enum
	{
		friend class con_map_enum < key, value >;

	protected:
		con_set< key, value >	*m_Set;
		uintptr_t				m_Index;
		Entry< key, value >		*m_CurrentEntry;
		Entry< key, value >		*m_NextEntry;

	public:

		con_set_enum();
		con_set_enum(con_set< key, value >& set);

		bool					operator=(con_set< key, value >& set);

		Entry< key, value >		*NextElement(void);
		Entry< key, value >		*CurrentElement(void);
	};

	template< typename key, typename value >
	class con_map {
		friend class con_map_enum < key, value >;

	private:
		con_set< key, value > m_con_set;

	public:
		void		Archive(Archiver& arc);

		void			clear();
		virtual void	resize(int count = 0);

		value&		operator[](const key& index);

		value*		find(const key& index);
		bool		remove(const key& index);

		uintptr_t	size();
	};

	template< typename key, typename value >
	class con_map_enum
	{
	private:
		con_set_enum< key, value >	m_Set_Enum;

	public:

		con_map_enum();
		con_map_enum(con_map< key, value >& map);

		bool	operator=(con_map< key, value >& map);

		key		*NextKey(void);
		value	*NextValue(void);
		key		*CurrentKey(void);
		value	*CurrentValue(void);
	};
	/*
	int HashCode( const char& key );
	int HashCode( const unsigned char& key );
	int HashCode( unsigned char * const& key );
	int HashCode( const short& key );
	int HashCode( const unsigned short& key );
	int HashCode( const short3& key );
	int HashCode( const unsigned_short3& key );
	int HashCode( const int& key );
	int HashCode( const unsigned int& key );
	int HashCode( const float& key );
	int HashCode( const char *key );
	int HashCode( const str& key );
	*/

	//template< typename k, typename v >
	//MEM_BlockAlloc< Entry< k, v >, MEM_BLOCKSIZE > con_set< k, v >::Entry_allocator;

	template< typename k >
	intptr_t HashCode(const k& key);

	/*
	template< typename k, typename v >
	void *Entry< k, v >::operator new( size_t size )
	{
		return con_set< k, v >::Entry_allocator.Alloc();
	}

	template< typename k, typename v >
	void Entry< k, v >::operator delete( void *ptr )
	{
		con_set< k, v >::Entry_allocator.Free( ptr );
	}
	*/

	template< typename k, typename v >
	Entry< k, v >::Entry(const k& inKey)
		: key(inKey)
	{
		index = 0;
		next = nullptr;
	}

	template< typename key, typename value >
	con_set<key, value>::con_set()
	{
		tableLength = 1;
		table = &defaultEntry;

		threshold = 1;
		count = 0;
		tableLengthIndex = 0;

		defaultEntry = nullptr;
	}

	template< typename key, typename value >
	con_set<key, value>::~con_set()
	{
		clear();
	}

	template< typename key, typename value >
	void con_set< key, value >::clear()
	{
		Entry< key, value > *entry = nullptr;
		Entry< key, value > *next = nullptr;
		size_t i;

		for (i = 0; i < tableLength; i++)
		{
			for (entry = table[i]; entry != nullptr; entry = next)
			{
				next = entry->next;
				delete entry;
			}
		}

		if (tableLength > 1)
		{
			delete[] table;
		}

		tableLength = 1;
		table = &defaultEntry;

		threshold = 1;
		count = 0;
		tableLengthIndex = 0;

		defaultEntry = nullptr;
	}

	template< typename key, typename value >
	void con_set< key, value >::resize(int newCount)
	{
		Entry< key, value > **oldTable = table;
		Entry< key, value > *e, *old;
		size_t oldTableLength = tableLength;
		intptr_t i;
		intptr_t index;

		if (newCount > 0)
		{
			tableLength += newCount;
			threshold = tableLength;
		}
		else
		{
			//threshold = ( unsigned int )( ( float )tableLength * 0.75f );
			threshold = (unsigned int)((float)tableLength * 0.75);
			if (threshold < 1)
			{
				threshold = 1;
			}

			tableLength += threshold;
		}

		// allocate a new table
		table = new Entry< key, value > *[tableLength]();
		memset(table, 0, tableLength * sizeof(Entry< key, value > *));

		// rehash the table
		for (i = oldTableLength - 1; i >= 0; i--)
		{
			// rehash all entries from the old table
			for (e = oldTable[i]; e != nullptr; e = old)
			{
				old = e->next;

				// insert the old entry to the table hashindex
				index = HashCode< key >(e->key) % tableLength;

				e->next = table[index];
				table[index] = e;
			}
		}

		if (oldTableLength > 1)
		{
			// delete the previous table
			delete[] oldTable;
		}
	}

	template< typename k, typename v >
	Entry< k, v > *con_set< k, v >::findKeyEntry(const k& key)
	{
		Entry< k, v > *entry = table[HashCode< k >(key) % tableLength];

		for (; entry != nullptr; entry = entry->next)
		{
			if (entry->key == key) {
				return entry;
			}
		}

		return nullptr;
	}

	template< typename k, typename v >
	const Entry< k, v >* MOHPC::con_set<k, v>::findKeyEntry(const k& key) const
	{
		const Entry< k, v >* entry = table[HashCode< k >(key) % tableLength];

		for (; entry != nullptr; entry = entry->next)
		{
			if (entry->key == key) {
				return entry;
			}
		}

		return nullptr;
	}

	template< typename k, typename v >
	Entry< k, v > *con_set< k, v >::addKeyEntry(const k& key)
	{
		Entry< k, v > *entry;

		entry = findKeyEntry(key);

		if (entry != nullptr) {
			return entry;
		}
		else {
			return addNewKeyEntry(key);
		}
	}

	template< typename k, typename v >
	Entry< k, v > *con_set< k, v >::addNewKeyEntry(const k& key)
	{
		Entry< k, v > *entry;
		intptr_t index;

		if (count >= threshold) {
			resize();
		}

		index = HashCode< k >(key) % tableLength;

		count++;

		entry = new Entry<k, v>(key);

		if (defaultEntry == nullptr)
		{
			defaultEntry = entry;
			entry->next = nullptr;
		}
		else {
			entry->next = table[index];
		}

		entry->key = key;

		table[index] = entry;

		return entry;
	}

	template< typename key, typename value >
	bool con_set< key, value >::isEmpty(void)
	{
		return count == 0;
	}

	template< typename k, typename v >
	bool con_set< k, v >::remove(const k& key)
	{
		intptr_t index = HashCode< k >(key) % tableLength;
		Entry< k, v > *prev = nullptr;
		Entry< k, v > *entry;

		for (entry = table[index]; entry != nullptr; entry = entry->next)
		{
			// just to make sure we're using the correct overloaded operator
			if (!(entry->key == key))
			{
				prev = entry;
				continue;
			}

			if (defaultEntry == entry)
			{
				defaultEntry = prev;
			}

			if (prev)
			{
				prev->next = entry->next;
			}
			else
			{
				table[index] = entry->next;
			}

			count--;
			delete entry;

			return true;
		}

		return false;
	}

	template< typename k, typename v >
	v *con_set< k, v >::findKeyValue(const k& key)
	{
		Entry< k, v > *entry = findKeyEntry(key);

		if (entry != nullptr) {
			return &entry->value;
		}
		else {
			return nullptr;
		}
	}

	template< typename key, typename value >
	key *con_set< key, value >::firstKeyValue(void)
	{
		if (defaultEntry)
		{
			return &defaultEntry->key;
		}
		else
		{
			return nullptr;
		}
	}

	template< typename k, typename v >
	v& con_set< k, v >::addKeyValue(const k& key)
	{
		Entry< k, v > *entry = addKeyEntry(key);

		return entry->value;
	}

	template< typename k, typename v >
	v& con_set< k, v >::addNewKeyValue(const k& key)
	{
		Entry< k, v > *entry = addNewKeyEntry(key);

		return entry->value;
	}

	template< typename k, typename v >
	bool con_set< k, v >::keyExists(const k& key)
	{
		Entry< k, v > *entry;

		for (entry = table; entry != nullptr; entry = entry->next)
		{
			if (entry->key == key) {
				return true;
			}
		}

		return false;
	}

	template< typename key, typename value >
	uintptr_t con_set< key, value >::size()
	{
		return count;
	}

	template< typename key, typename value >
	con_set_enum< key, value >::con_set_enum()
	{
		m_Set = nullptr;
		m_Index = 0;
		m_CurrentEntry = nullptr;
		m_NextEntry = nullptr;
	}

	template< typename key, typename value >
	con_set_enum< key, value >::con_set_enum(con_set< key, value > &set)
	{
		*this = set;
	}

	template< typename key, typename value >
	bool con_set_enum< key, value >::operator=(con_set< key, value > &set)
	{
		m_Set = &set;
		m_Index = m_Set->tableLength;
		m_CurrentEntry = nullptr;
		m_NextEntry = nullptr;

		return true;
	}

	template< typename key, typename value >
	Entry< key, value >	*con_set_enum< key, value >::CurrentElement(void)
	{
		return m_CurrentEntry;
	}

	template< typename key, typename value >
	Entry< key, value >	*con_set_enum< key, value >::NextElement(void)
	{
		if (!m_NextEntry)
		{
			while (1)
			{
				if (!m_Index) {
					break;
				}

				m_Index--;
				m_NextEntry = m_Set->table[m_Index];

				if (m_NextEntry) {
					break;
				}
			}

			if (!m_NextEntry)
			{
				m_CurrentEntry = nullptr;
				return nullptr;
			}
		}

		m_CurrentEntry = m_NextEntry;
		m_NextEntry = m_NextEntry->next;

		return m_CurrentEntry;
	}

	template< typename key, typename value >
	void con_map< key, value >::clear()
	{
		m_con_set.clear();
	}

	template< typename key, typename value >
	void con_map< key, value >::resize(int count)
	{
		m_con_set.resize(count);
	}

	template< typename key, typename value >
	value& con_map< key, value >::operator[](const key& index)
	{
		return m_con_set.addKeyValue(index);
	}

	template< typename key, typename value >
	value* con_map< key, value >::find(const key& index)
	{
		return m_con_set.findKeyValue(index);
	}

	template< typename key, typename value >
	bool con_map< key, value >::remove(const key& index)
	{
		return m_con_set.remove(index);
	}

	template< typename key, typename value >
	uintptr_t con_map< key, value >::size(void)
	{
		return m_con_set.size();
	}

	template< typename key, typename value >
	con_map_enum< key, value >::con_map_enum()
	{
		m_Set_Enum.m_Set = nullptr;
		m_Set_Enum.m_Index = 0;
		m_Set_Enum.m_CurrentEntry = nullptr;
		m_Set_Enum.m_NextEntry = nullptr;
	}

	template< typename key, typename value >
	con_map_enum< key, value >::con_map_enum(con_map< key, value >& map)
	{
		*this = map;
	}

	template< typename key, typename value >
	bool con_map_enum< key, value >::operator=(con_map< key, value >& map)
	{
		m_Set_Enum = map.m_con_set;

		return true;
	}

	template< typename key, typename value >
	key *con_map_enum< key, value >::CurrentKey(void)
	{
		Entry< key, value > *entry = m_Set_Enum.CurrentElement();

		if (entry)
		{
			return &entry->key;
		}
		else
		{
			return nullptr;
		}
	}

	template< typename key, typename value >
	value *con_map_enum< key, value >::CurrentValue(void)
	{
		Entry< key, value > *entry = m_Set_Enum.CurrentElement();

		if (entry)
		{
			return &entry->value;
		}
		else
		{
			return nullptr;
		}
	}

	template< typename key, typename value >
	key *con_map_enum< key, value >::NextKey(void)
	{
		Entry< key, value > *entry = m_Set_Enum.NextElement();

		if (entry)
		{
			return &entry->key;
		}
		else
		{
			return nullptr;
		}
	}

	template< typename key, typename value >
	value *con_map_enum< key, value >::NextValue(void)
	{
		Entry< key, value > *entry = m_Set_Enum.NextElement();

		if (entry)
		{
			return &entry->value;
		}
		else
		{
			return nullptr;
		}
	}
};
