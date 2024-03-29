#pragma once

#include "UtilityGlobal.h"
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
	class TokenParser
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
		const char* script_p;
		const char* end_p;
		std::vector<macro*> macrolist;

		int32_t line;
		str token;

		bool		releaseBuffer;
		bool		hasError;

		bool		AtComment();
		void			CheckOverflow();

	public:
		const char* buffer;
		uintmax_t length;

		MOHPC_UTILITY_EXPORTS TokenParser();
		MOHPC_UTILITY_EXPORTS TokenParser(const char* s);
		MOHPC_UTILITY_EXPORTS TokenParser(const char* s, size_t len);
		MOHPC_UTILITY_EXPORTS TokenParser(const str& s);
		MOHPC_UTILITY_EXPORTS ~TokenParser();

		MOHPC_UTILITY_EXPORTS void Close();
		MOHPC_UTILITY_EXPORTS const char* Filename();
		MOHPC_UTILITY_EXPORTS int GetLineNumber() const;
		MOHPC_UTILITY_EXPORTS void Reset();
		MOHPC_UTILITY_EXPORTS void MarkPosition(scriptmarker_t* mark);
		MOHPC_UTILITY_EXPORTS void RestorePosition(const scriptmarker_t* mark);
		MOHPC_UTILITY_EXPORTS bool SkipToEOL();
		MOHPC_UTILITY_EXPORTS void SkipWhiteSpace(bool crossline);
		MOHPC_UTILITY_EXPORTS void SkipNonToken(bool crossline);
		MOHPC_UTILITY_EXPORTS bool TokenAvailable(bool crossline);
		MOHPC_UTILITY_EXPORTS bool CommentAvailable(bool crossline);
		MOHPC_UTILITY_EXPORTS void UnGetToken();
		MOHPC_UTILITY_EXPORTS bool AtString(bool crossline);
		MOHPC_UTILITY_EXPORTS bool AtOpenParen(bool crossline);
		MOHPC_UTILITY_EXPORTS bool AtCloseParen(bool crossline);
		MOHPC_UTILITY_EXPORTS bool AtComma(bool crossline);
		MOHPC_UTILITY_EXPORTS bool AtDot(bool crossline);
		MOHPC_UTILITY_EXPORTS bool AtAssignment(bool crossline);
		MOHPC_UTILITY_EXPORTS const char* GetToken(bool crossline);
		MOHPC_UTILITY_EXPORTS const char* GetLine(bool crossline);
		MOHPC_UTILITY_EXPORTS const char* GetRaw();
		MOHPC_UTILITY_EXPORTS const char* GetString(bool crossline, bool escape = true, bool allowMultiLines = false);
		MOHPC_UTILITY_EXPORTS const char* GetCurrentScript() const;
		MOHPC_UTILITY_EXPORTS bool GetSpecific(const char* string);
		MOHPC_UTILITY_EXPORTS bool GetBoolean(bool crossline);
		MOHPC_UTILITY_EXPORTS int GetInteger(bool crossline);
		MOHPC_UTILITY_EXPORTS uint64_t GetInteger64(bool crossline);
		MOHPC_UTILITY_EXPORTS double GetDouble(bool crossline);
		MOHPC_UTILITY_EXPORTS float GetFloat(bool crossline);
		MOHPC_UTILITY_EXPORTS void GetVector(bool crossline, vec3r_t out);
		MOHPC_UTILITY_EXPORTS int LinesInFile();
		MOHPC_UTILITY_EXPORTS void Parse(const char* data, size_t length, const char* name = "");
		MOHPC_UTILITY_EXPORTS const char* Token();
		MOHPC_UTILITY_EXPORTS void AddMacroDefinition(bool crossline);
		MOHPC_UTILITY_EXPORTS const char* GetMacroString(const char* theMacroName);
		MOHPC_UTILITY_EXPORTS char* EvaluateMacroString(const char* theMacroString);
		MOHPC_UTILITY_EXPORTS double EvaluateMacroMath(double value, double newval, char oper);
		MOHPC_UTILITY_EXPORTS const char* GetExprToken(const char* ptr, char* token);
		MOHPC_UTILITY_EXPORTS const char* GrabNextToken(bool crossline);
		MOHPC_UTILITY_EXPORTS bool isMacro();

		MOHPC_UTILITY_EXPORTS bool EndOfFile();
		MOHPC_UTILITY_EXPORTS bool isValid();

		MOHPC_UTILITY_EXPORTS std::vector<macro*>* GetMacroList() { return &macrolist; }
		MOHPC_UTILITY_EXPORTS void AddMacro(const char* name, const char* value);
	};

	namespace TokenErrors
	{
		class Base : public std::exception {};

		/**
		 * End of token file reached prematurely while reading string.
		 */
		class EndOfFileReached : public Base
		{
		public:
			EndOfFileReached();
			~EndOfFileReached();

			MOHPC_UTILITY_EXPORTS const char* what() const noexcept override;
		};

		/**
		 * No Macro Text found.
		 */
		class NoMacroTextFound : public Base
		{
		public:
			NoMacroTextFound(const char* macroNamePtr);
			~NoMacroTextFound();

			MOHPC_UTILITY_EXPORTS const char* getMacroName() const;
			MOHPC_UTILITY_EXPORTS const char* what() const noexcept override;

		private:
			std::string macroName;
		};

		/**
		 * Line is incomplete while reading string.
		 */
		class LineIncomplete : public Base
		{
		public:
			LineIncomplete(uint32_t lineNum);
			~LineIncomplete();

			MOHPC_UTILITY_EXPORTS uint32_t getLineNum() const;
			MOHPC_UTILITY_EXPORTS const char* what() const noexcept override;

		private:
			uint32_t lineNum;
		};

		/**
		 * Expecting string on line
		 */
		class ExpectedToken : public Base
		{
		public:
			ExpectedToken(const char* expected, uint32_t lineNum);
			~ExpectedToken();

			MOHPC_UTILITY_EXPORTS const char* getExpected() const;
			MOHPC_UTILITY_EXPORTS uint32_t getLineNum() const;
			MOHPC_UTILITY_EXPORTS const char* what() const noexcept override;

		private:
			const char* expected;
			uint32_t lineNum;
		};
	}
};
