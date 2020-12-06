#pragma once

#include "Asset.h"
#include "Vector.h"
#include "Common/Container.h"
#include "Common/str.h"

#define TOKENCOMMENT		(';')
#define TOKENCOMMENT2		('#')
#define TOKENEOL			('\n')
//#define TOKENNULL			('\0')
#define TOKENSPACE			(' ')
#define TOKENSPECIAL		('$')

#define	MAXTOKEN	512

namespace MOHPC
{
	class Script : public Asset
	{
	public:
		typedef struct
		{
			bool tokenready;
			size_t offset;
			int32_t line;
			str token;
		} scriptmarker_t;


		struct macro
		{
			str macroName;
			str macroText;
		};
	protected:
		bool		tokenready;

		str filename;
		const char	*script_p;
		const char	*end_p;
		Container<macro *> macrolist;

		int32_t line;
		str token;

		bool		releaseBuffer;

		bool		AtComment(void);
		void			CheckOverflow(void);

	public:
		const char	*buffer;
		uint64_t	length;

		MOHPC_EXPORTS ~Script();
		MOHPC_EXPORTS Script(const char* filename);
		MOHPC_EXPORTS Script();

		MOHPC_EXPORTS void Load() override;
		MOHPC_EXPORTS void Close(void);
		MOHPC_EXPORTS const char* Filename(void);
		MOHPC_EXPORTS int GetLineNumber(void);
		MOHPC_EXPORTS void Reset(void);
		MOHPC_EXPORTS void MarkPosition(scriptmarker_t* mark);
		MOHPC_EXPORTS void RestorePosition(const scriptmarker_t* mark);
		MOHPC_EXPORTS bool SkipToEOL(void);
		MOHPC_EXPORTS void SkipWhiteSpace(bool crossline);
		MOHPC_EXPORTS void SkipNonToken(bool crossline);
		MOHPC_EXPORTS bool TokenAvailable(bool crossline);
		MOHPC_EXPORTS bool CommentAvailable(bool crossline);
		MOHPC_EXPORTS void UnGetToken(void);
		MOHPC_EXPORTS bool AtString(bool crossline);
		MOHPC_EXPORTS bool AtOpenParen(bool crossline);
		MOHPC_EXPORTS bool AtCloseParen(bool crossline);
		MOHPC_EXPORTS bool AtComma(bool crossline);
		MOHPC_EXPORTS bool AtDot(bool crossline);
		MOHPC_EXPORTS bool AtAssignment(bool crossline);
		MOHPC_EXPORTS const char* GetToken(bool crossline);
		MOHPC_EXPORTS const char* GetLine(bool crossline);
		MOHPC_EXPORTS const char* GetRaw(void);
		MOHPC_EXPORTS const char* GetString(bool crossline, bool allowMultiLines = false);
		MOHPC_EXPORTS bool GetSpecific(const char* string);
		MOHPC_EXPORTS bool GetBoolean(bool crossline);
		MOHPC_EXPORTS int GetInteger(bool crossline);
		MOHPC_EXPORTS double GetDouble(bool crossline);
		MOHPC_EXPORTS float GetFloat(bool crossline);
		MOHPC_EXPORTS Vector GetVector(bool crossline);
		MOHPC_EXPORTS int LinesInFile(void);
		MOHPC_EXPORTS void Parse(const char* data, uintmax_t length, const char* name = "");
		MOHPC_EXPORTS void LoadFile(const char* name);
		MOHPC_EXPORTS void LoadFile(const char* name, int length, const char* buf);
		MOHPC_EXPORTS const char* Token(void);
		MOHPC_EXPORTS void AddMacroDefinition(bool crossline);
		MOHPC_EXPORTS const char* GetMacroString(const char* theMacroName);
		MOHPC_EXPORTS char* EvaluateMacroString(const char* theMacroString);
		MOHPC_EXPORTS double EvaluateMacroMath(double value, double newval, char oper);
		MOHPC_EXPORTS const char* GetExprToken(const char* ptr, char* token);
		MOHPC_EXPORTS const char* GrabNextToken(bool crossline);
		MOHPC_EXPORTS bool isMacro(void);

		MOHPC_EXPORTS bool EndOfFile();
		MOHPC_EXPORTS bool isValid(void);

		MOHPC_EXPORTS Container<macro *> *GetMacroList() { return &macrolist; }
		MOHPC_EXPORTS void AddMacro(const char *name, const char *value);
	};
};
