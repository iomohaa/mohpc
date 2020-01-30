#pragma once

#include "ScriptClass.h"
#include "con_set.h"

namespace MOHPC
{
	typedef struct {
		unsigned int		sourcePos;
		int					column;
		int					line;
	} sourceinfo_t;

	class AbstractScript : public BaseScriptClass
	{
	public:
		const_str m_Filename;
		char* m_SourceBuffer;
		size_t m_SourceLength;

		con_set<unsigned char *, sourceinfo_t> *m_ProgToSource;

	public:
		AbstractScript();

		const str& Filename();
		const_str ConstFilename();
		bool GetSourceAt(size_t sourcePos, str &sourceLine, int &column, int &line);
		void PrintSourcePos(sourceinfo_t* sourcePos, bool dev);
		void PrintSourcePos(size_t sourcePos, bool dev);
		void PrintSourcePos(unsigned char* m_pCodePos, bool dev);
		void PrintSourcePos(str sourceLine, int column, int line, bool dev);
	};
}
