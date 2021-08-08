#pragma once

#include "Asset.h"
#include "../Common/Vector.h"
#include "../Common/str.h"

#include <vector>

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
		struct scriptmarker_t
		{
			bool tokenready;
			size_t offset;
			int32_t line;
			str token;
		};


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
		std::vector<macro *> macrolist;

		int32_t line;
		str token;

		bool		releaseBuffer;

		bool		AtComment(void);
		void			CheckOverflow(void);

	public:
		const char	*buffer;
		uint64_t	length;

		MOHPC_ASSETS_EXPORTS ~Script();
		MOHPC_ASSETS_EXPORTS Script(const char* filename);
		MOHPC_ASSETS_EXPORTS Script();

		MOHPC_ASSETS_EXPORTS void Load() override;
		MOHPC_ASSETS_EXPORTS void Close(void);
		MOHPC_ASSETS_EXPORTS const char* Filename(void);
		MOHPC_ASSETS_EXPORTS int GetLineNumber(void);
		MOHPC_ASSETS_EXPORTS void Reset(void);
		MOHPC_ASSETS_EXPORTS void MarkPosition(scriptmarker_t* mark);
		MOHPC_ASSETS_EXPORTS void RestorePosition(const scriptmarker_t* mark);
		MOHPC_ASSETS_EXPORTS bool SkipToEOL(void);
		MOHPC_ASSETS_EXPORTS void SkipWhiteSpace(bool crossline);
		MOHPC_ASSETS_EXPORTS void SkipNonToken(bool crossline);
		MOHPC_ASSETS_EXPORTS bool TokenAvailable(bool crossline);
		MOHPC_ASSETS_EXPORTS bool CommentAvailable(bool crossline);
		MOHPC_ASSETS_EXPORTS void UnGetToken(void);
		MOHPC_ASSETS_EXPORTS bool AtString(bool crossline);
		MOHPC_ASSETS_EXPORTS bool AtOpenParen(bool crossline);
		MOHPC_ASSETS_EXPORTS bool AtCloseParen(bool crossline);
		MOHPC_ASSETS_EXPORTS bool AtComma(bool crossline);
		MOHPC_ASSETS_EXPORTS bool AtDot(bool crossline);
		MOHPC_ASSETS_EXPORTS bool AtAssignment(bool crossline);
		MOHPC_ASSETS_EXPORTS const char* GetToken(bool crossline);
		MOHPC_ASSETS_EXPORTS const char* GetLine(bool crossline);
		MOHPC_ASSETS_EXPORTS const char* GetRaw(void);
		MOHPC_ASSETS_EXPORTS const char* GetString(bool crossline, bool allowMultiLines = false);
		MOHPC_ASSETS_EXPORTS bool GetSpecific(const char* string);
		MOHPC_ASSETS_EXPORTS bool GetBoolean(bool crossline);
		MOHPC_ASSETS_EXPORTS int GetInteger(bool crossline);
		MOHPC_ASSETS_EXPORTS double GetDouble(bool crossline);
		MOHPC_ASSETS_EXPORTS float GetFloat(bool crossline);
		MOHPC_ASSETS_EXPORTS void GetVector(bool crossline, vec3r_t out);
		MOHPC_ASSETS_EXPORTS int LinesInFile(void);
		MOHPC_ASSETS_EXPORTS void Parse(const char* data, uintmax_t length, const char* name = "");
		MOHPC_ASSETS_EXPORTS void LoadFile(const char* name);
		MOHPC_ASSETS_EXPORTS void LoadFile(const char* name, int length, const char* buf);
		MOHPC_ASSETS_EXPORTS const char* Token(void);
		MOHPC_ASSETS_EXPORTS void AddMacroDefinition(bool crossline);
		MOHPC_ASSETS_EXPORTS const char* GetMacroString(const char* theMacroName);
		MOHPC_ASSETS_EXPORTS char* EvaluateMacroString(const char* theMacroString);
		MOHPC_ASSETS_EXPORTS double EvaluateMacroMath(double value, double newval, char oper);
		MOHPC_ASSETS_EXPORTS const char* GetExprToken(const char* ptr, char* token);
		MOHPC_ASSETS_EXPORTS const char* GrabNextToken(bool crossline);
		MOHPC_ASSETS_EXPORTS bool isMacro(void);

		MOHPC_ASSETS_EXPORTS bool EndOfFile();
		MOHPC_ASSETS_EXPORTS bool isValid(void);

		MOHPC_ASSETS_EXPORTS std::vector<macro *> *GetMacroList() { return &macrolist; }
		MOHPC_ASSETS_EXPORTS void AddMacro(const char *name, const char *value);
	};
};
