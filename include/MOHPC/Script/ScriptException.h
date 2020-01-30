#pragma once

#include "str.h"

namespace MOHPC
{
	class ScriptException
	{
	public:
		str		string;
		int		bAbort;
		int		bIsForAnim;

	private:
		void CreateException(const char *data);

	public:
		ScriptException(const str& text);
		ScriptException(const char *format, ...);
		ScriptException(char *text);

		static int next_abort;
		static int next_bIsForAnim;
	};

	void Error(const char * format, ...);

#define ScriptDeprecated( function ) throw ScriptException( function ": DEPRECATED. DON'T USE IT ANYMORE" )
#define ScriptError throw ScriptException
};
