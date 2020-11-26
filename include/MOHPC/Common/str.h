#pragma once

#include "../Global.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <utility>

#ifdef _WIN32
#pragma warning(disable : 4710)     // function 'blah' not inlined
#endif

namespace MOHPC
{
	void TestStringClass();

	class strdata
	{
	public:
		strdata() : data(NULL), refcount(0), alloced(0), len(0) {}
		~strdata()
		{
			if (data)
				delete[] data;
		}

		void AddRef() { refcount++; }
		bool DelRef() // True if killed
		{
			refcount--;
			if (refcount < 0)
			{
				delete this;
				return true;
			}

			return false;
		}

		char* data;
		intptr_t refcount;
		size_t alloced;
		size_t len;
	};

	class MOHPC_EXPORTS str
	{
	protected:

		friend class		Archiver;
		strdata* m_data;
		void EnsureAlloced(size_t, bool keepold = true);
		void EnsureDataWritable();

	public:
		~str();
		str();
		str(const char* text);
		str(const char* text, size_t len);
		str(const str& string);
		str(str&& string) noexcept;
		str(const str& string, size_t start, size_t end);
		str(const char ch);
		str(const float num);
		str(const int num);
		str(const unsigned int num);
		str(const long num);
		str(const unsigned long num);
		str(const long long num);
		str(const unsigned long long num);


		size_t length(void) const;
		const char* c_str(void) const;

		void append(const char* text);
		void append(const str& text);
		void assign(const char* text, size_t sz);
		void resize(size_t len);
		void reserve(size_t len);
		void clear();

		char operator[](intptr_t index) const;
		char& operator[](intptr_t index);

		void operator=(const str& text);
		str& operator=(str&& text) noexcept;
		void operator=(const char* text);

		MOHPC_EXPORTS friend str operator+(const str& a, const str& b);
		MOHPC_EXPORTS friend str operator+(const str& a, const char* b);
		MOHPC_EXPORTS friend str operator+(const char* a, const str& b);

		MOHPC_EXPORTS friend str operator+(const str& a, const float b);
		MOHPC_EXPORTS friend str operator+(const str& a, const int b);
		MOHPC_EXPORTS friend str operator+(const str& a, const unsigned b);
		MOHPC_EXPORTS friend str operator+(const str& a, const bool b);
		MOHPC_EXPORTS friend str operator+(const str& a, const char b);

		str& operator+=(const str& a);
		str& operator+=(const char* a);
		str& operator+=(const float a);
		str& operator+=(const char a);
		str& operator+=(const int a);
		str& operator+=(const unsigned a);
		str& operator+=(const bool a);

		str& operator-=(int c);
		str& operator--(int);

		MOHPC_EXPORTS friend bool operator==(const str& a, const str& b);
		MOHPC_EXPORTS friend bool operator==(const str& a, const char* b);
		MOHPC_EXPORTS friend bool operator==(const char* a, const str& b);

		MOHPC_EXPORTS friend bool operator!=(const str& a, const str& b);
		MOHPC_EXPORTS friend bool operator!=(const str& a, const char* b);
		MOHPC_EXPORTS friend bool operator!=(const char* a, const str& b);

		operator const char* () const;

		int icmpn(const char* text, size_t n) const;
		int icmpn(const str& text, size_t n) const;
		int icmp(const char* text) const;
		int icmp(const str& text) const;
		int cmpn(const char* text, size_t n) const;
		int cmpn(const str& text, size_t n) const;

		void tolower();
		void toupper();

		static char* tolower(char* s1);
		static char* toupper(char* s1);
		static char tolower(char c);
		static char toupper(char c);

		static int icmpn(const char* s1, const char* s2, size_t n);
		static int icmp(const char* s1, const char* s2);
		static int cmpn(const char* s1, const char* s2, size_t n);
		static int cmp(const char* s1, const char* s2);
		static size_t len(const char* s);
		static const char* ifindchar(const char* s, char c);
		static const char* ifindcharn(const char* s, char c, size_t n);
		static const char* ifind(const char* s1, const char* s2);
		static const char* ifindn(const char* s1, const char* s2, size_t n);
		static const char* findchar(const char* s, char c);
		static const char* findcharn(const char* s, char c, size_t n);
		static const char* find(const char* s1, const char* s2);
		static const char* findn(const char* s1, const char* s2, size_t n);

		static   void snprintf(char* dst, int size, const char* fmt, ...);
		void strip(void);

		static bool isNumeric(const char* str);
		bool isNumeric(void) const;
		bool isEmpty() const;

		void CapLength(size_t newlen);

		void BackSlashesToSlashes();
		void SlashesToBackSlashes();
		void DefaultExtension(const char* extension);
		const char* GetExtension() const;
		void StripExtension();
		void SkipFile();
		void SkipPath();

		template<typename... Args>
		static str printf(const char* fmt, Args&&... args)
		{
			return printfImpl(fmt, std::forward<Args>(args)...);
		}

	private:
		static str printfImpl(const char* fmt, ...);
	};

	char* strstrip(char* string);
	char* strlwc(char* string);

	template<typename Archive>
	Archive& operator<<(Archive& ar, str& value)
	{
		if (value.length() > 0)
		{
			ar << (uint32_t)value.length();

			for (size_t i = 0; i < value.length(); ++i)
			{
				ar << value.c_str()[i];
			}
		}
		else {
			ar << (uint32_t)0;
		}

		return ar;
	}

	template<typename Archive>
	Archive& operator>>(Archive& ar, str& value)
	{
		uint32_t len = 0;
		ar >> len;

		if (len)
		{
			value.resize(len);
			for (size_t i = 0; i < len; ++i)
			{
				ar >> value[i];
			}
		}

		return ar;
	}
};
