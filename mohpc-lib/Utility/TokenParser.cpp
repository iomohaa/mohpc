#include <Shared.h>
#include <MOHPC/Utility/TokenParser.h>

#include <cstring>
#include <cassert>

using namespace MOHPC;

TokenParser::TokenParser()
{
	buffer = NULL;
	script_p = NULL;
	end_p = NULL;
	line = 0;
	length = 0;
	releaseBuffer = false;
	tokenready = false;
	token.clear();
}

TokenParser::TokenParser(const char* s)
	: TokenParser()
{
	Parse(s, strHelpers::len(s));
}

TokenParser::TokenParser(const char* s, size_t len)
	: TokenParser()
{
	Parse(s, len);
}

TokenParser::TokenParser(const str& s)
	: TokenParser()
{
	Parse(s.c_str(), s.length());
}

TokenParser::~TokenParser()
{
	Close();
}

void TokenParser::Close()
{
	if (releaseBuffer && buffer)
	{
		delete[] buffer;
	}

	buffer = NULL;
	script_p = NULL;
	end_p = NULL;
	line = 0;
	releaseBuffer = false;
	tokenready = false;
	token.clear();

	//Loop Through the macro mfuse::con::Container and delete (del33t -hehe) them all
	for (size_t i = 0; i < macrolist.size(); i++)
	{
		if (macrolist[i])
		{
			delete macrolist[i];
			macrolist[i] = NULL;
		}
	}
}

/*
==============
=
= Filename
=
==============
*/

const char* TokenParser::Filename()
{
	return filename.c_str();
}

/*
==============
=
= GetLineNumber
=
==============
*/

int TokenParser::GetLineNumber() const
{
	return line;
}

/*
==============
=
= Reset
=
==============
*/

void TokenParser::Reset()
{
	script_p = buffer;
	line = 1;
	tokenready = false;
	hasError = false;
}

/*
==============
=
= MarkPosition
=
==============
*/

void TokenParser::MarkPosition(scriptmarker_t* mark)
{
	assert(mark);

	mark->tokenready = tokenready;
	mark->offset = script_p - buffer;
	mark->line = line;
	mark->token = token;
}

/*
==============
=
= RestorePosition
=
==============
*/

void TokenParser::RestorePosition(const scriptmarker_t* mark)
{
	assert(mark);

	tokenready = mark->tokenready;
	script_p = buffer + mark->offset;
	line = mark->line;
	token = mark->token;

	assert(script_p <= end_p);
	if (script_p > end_p)
	{
		script_p = end_p;
	}
}

/*
==============
=
= SkipToEOL
=
==============
*/

bool TokenParser::SkipToEOL()
{
	if (script_p >= end_p)
	{
		return true;
	}

	while (*script_p != TOKENEOL)
	{
		if (script_p >= end_p)
		{
			return true;
		}
		script_p++;
	}
	return false;
}

/*
==============
=
= CheckOverflow
=
==============
*/

void TokenParser::CheckOverflow()
{
	if (script_p >= end_p) {
		throw TokenErrors::EndOfFileReached();
	}
}

/*
==============
=
= SkipWhiteSpace
=
==============
*/

void TokenParser::SkipWhiteSpace(bool crossline)
{
	//
	// skip space
	//
	CheckOverflow();

	while (*script_p <= TOKENSPACE || *script_p == TOKENCOMMA)
	{
		if (*script_p++ == TOKENEOL)
		{
			if (!crossline) {
				throw TokenErrors::LineIncomplete(line);
			}

			line++;
		}
		CheckOverflow();
	}
}

bool TokenParser::AtComment()
{
	if (script_p >= end_p)
	{
		return false;
	}

	if (*script_p == TOKENCOMMENT)
	{
		return true;
	}

	if (*script_p == TOKENCOMMENT2)
	{
		return true;
	}

	// Two or more character comment specifiers
	if ((script_p + 1) >= end_p)
	{
		return false;
	}

	if ((*script_p == '/') && (*(script_p + 1) == '/'))
	{
		return true;
	}

	return false;
}

/*
==============
=
= SkipNonToken
=
==============
*/

void TokenParser::SkipNonToken(bool crossline)
{
	//
	// skip space and comments
	//
	SkipWhiteSpace(crossline);
	while (AtComment())
	{
		SkipToEOL();
		SkipWhiteSpace(crossline);
	}
}

/*
=============================================================================
=
= Token section
=
=============================================================================
*/

/*
==============
=
= TokenAvailable
=
==============
*/

bool TokenParser::TokenAvailable(bool crossline)
{
	if (script_p >= end_p)
	{
		return false;
	}

	while (1)
	{
		while (*script_p <= TOKENSPACE)
		{
			if (*script_p == TOKENEOL)
			{
				if (!crossline)
				{
					return(false);
				}
				line++;
			}

			script_p++;
			if (script_p >= end_p)
			{
				return false;
			}
		}

		if (AtComment())
		{
			bool done;

			done = SkipToEOL();
			if (done)
			{
				return false;
			}
		}
		else
		{
			break;
		}
	}

	return true;
}

/*
==============
=
= CommentAvailable
=
==============
*/

bool TokenParser::CommentAvailable(bool crossline)
{
	const char* searchptr;

	searchptr = script_p;

	if (searchptr >= end_p)
	{
		return false;
	}

	while (*searchptr <= TOKENSPACE)
	{
		if ((*searchptr == TOKENEOL) && (!crossline))
		{
			return false;
		}
		searchptr++;
		if (searchptr >= end_p)
		{
			return false;
		}
	}

	return true;
}

/*
==============
=
= UnGet
=
= Signals that the current token was not used, and should be reported
= for the next GetToken.  Note that

GetToken (true);
UnGetToken ();
GetToken (false);

= could cross a line boundary.
=
==============
*/

void TokenParser::UnGetToken()
{
	tokenready = true;
}

/*
==============
=
= Get
=
==============
*/
bool TokenParser::AtString(bool crossline)
{
	//
	// skip space
	//
	SkipNonToken(crossline);

	return (*script_p == '"');
}

bool TokenParser::AtOpenParen(bool crossline)
{
	//
	// skip space
	//
	SkipNonToken(crossline);

	return (*script_p == '(');
}

bool TokenParser::AtCloseParen(bool crossline)
{
	//
	// skip space
	//
	SkipNonToken(crossline);

	return (*script_p == ')');
}

bool TokenParser::AtComma(bool crossline)
{
	//
	// skip space
	//
	SkipNonToken(crossline);

	return (*script_p == ',');
}

bool TokenParser::AtDot(bool crossline)
{
	//
	// skip space
	//
	SkipNonToken(crossline);

	return (*script_p == '.');
}

bool TokenParser::AtAssignment(bool crossline)
{
	//
	// skip space
	//
	SkipNonToken(crossline);

	return	(*script_p == '=') ||
		((*script_p == '+') && (*(script_p + 1) == '=')) ||
		((*script_p == '-') && (*(script_p + 1) == '=')) ||
		((*script_p == '*') && (*(script_p + 1) == '=')) ||
		((*script_p == '/') && (*(script_p + 1) == '='));
}

/*
==============
=
= Get
=
==============
*/

const char* TokenParser::GetToken(bool crossline)
{
	str token_p = token;
	bool is_Macro = false;

	// is a token already waiting?
	if (tokenready)
	{
		tokenready = false;
		return token.c_str();
	}

	is_Macro = isMacro();

	token_p = GrabNextToken(crossline);

	if (is_Macro && (token_p != "$include"))
	{

		//Check to see if we need to add any definitions
		while ((token_p == "$define") || (token_p == "$Define"))
		{
			AddMacroDefinition(crossline);
			is_Macro = isMacro();
			//if ( !is_Macro )
			//	return "";
			token_p = GrabNextToken(crossline);
		}

		//Check to see if we need return any defines strings
		if (is_Macro && (token_p != "$include") && (token_p[token_p.length() - 1] == '$'))
		{
			return GetMacroString(token_p.c_str());
		}

	}

	return token.c_str();
}

/*
==============
=
= GrabNextToken
=
==============
*/
const char* TokenParser::GrabNextToken(bool crossline)
{
	//
	// skip space
	//
	SkipNonToken(crossline);

	//
	// copy token
	//
	if (*script_p == '"')
	{
		return GetString(crossline);
	}

	token.clear();
	while ((*script_p > TOKENSPACE) && (*script_p != TOKENCOMMA) && !AtComment())
	{
		if ((*script_p == '\\') && (script_p < (end_p - 1)))
		{
			script_p++;
			switch (*script_p)
			{
			case 'n':	token.append(1, '\n'); break;
			case 'r':	token.append(1, '\n'); break;
			case '\'':  token.append(1, '\''); break;
			case '\"':  token.append(1, '\"'); break;
			case '\\':  token.append(1, '\\'); break;
			default:	token.append(1, *script_p); break;
			}
			script_p++;
		}
		else
		{
			token.append(1, *script_p++);
		}

		if (script_p == end_p)
		{
			break;
		}
	}

	return token.c_str();
}

/*
==============
=
= AddMacroDefinition
=
==============
*/
void TokenParser::AddMacroDefinition(bool crossline)
{
	macro* theMacro;

	//Create a new macro structure.  This new macro will be deleted in the script close()
	theMacro = new macro;

	//Grab the macro name
	theMacro->macroName = "$";
	theMacro->macroName.append(GrabNextToken(crossline));
	theMacro->macroName.append("$"); //<-- Adding closing ($) to keep formatting consistant

	//Grab the macro string
	str tmpstr;
	tmpstr = GrabNextToken(crossline);
	//Check to see if we need return any defines strings
	if ((tmpstr != "$include") && (tmpstr[tmpstr.length() - 1] == '$'))
		theMacro->macroText = GetMacroString(tmpstr.c_str());
	else
		theMacro->macroText = tmpstr;

	macrolist.push_back(theMacro);

}

/*
==============
=
= GetMacroString
=
==============
*/
const char* TokenParser::GetMacroString(const char* theMacroName)
{

	macro* theMacro = 0; //Initialize this puppy

	for (size_t i = 0; i < macrolist.size(); i++)
	{
		theMacro = macrolist[i];

		if (!strHelpers::cmp(theMacro->macroName.c_str(), theMacroName))
		{
			const char* text = theMacro->macroText.c_str();

			// If our define value is another define...
			if (text[0] == '$')
				return EvaluateMacroString(text);
			else
				return text;
		}

	}

	char tmpstr[255], * sptr = tmpstr;
	strcpy(tmpstr, theMacroName);
	tmpstr[strlen(tmpstr) - 1] = 0;
	sptr++;

	// We didn't find what we were looking for
	throw TokenErrors::NoMacroTextFound(theMacroName);

}

//================================================================
// Name:			AddMacro
// Class:			Script
//
// Description:		Adds a macro to the definitions list.
//
// Parameters:		const char *name -- Name of the macro
//					const char *value -- Value
//
// Returns:			None
//
//================================================================
void TokenParser::AddMacro(const char* name, const char* value)
{

}

/*
==============
=
= EvaluateMacroString
=
==============
*/
char* TokenParser::EvaluateMacroString(const char* theMacroString)
{
	static char evalText[255];
	char buffer[255], * bufferptr = buffer, oper = '+', newoper = '+';
	bool haveoper = false;
	double value = 0.0f, val = 0.0f;
	memset(buffer, 0, 255);

	for (size_t i = 0; i <= strlen(theMacroString); i++)
	{
		if (theMacroString[i] == '+') { haveoper = true; newoper = '+'; }
		if (theMacroString[i] == '-') { haveoper = true; newoper = '-'; }
		if (theMacroString[i] == '*') { haveoper = true; newoper = '*'; }
		if (theMacroString[i] == '/') { haveoper = true; newoper = '/'; }
		if (theMacroString[i] == 0) haveoper = true;

		if (haveoper)
		{
			if (buffer[0] == '$')
				val = atof(GetMacroString(buffer));
			else
				val = atof(buffer);

			value = EvaluateMacroMath(value, val, oper);
			oper = newoper;

			// Reset everything
			haveoper = false;
			memset(buffer, 0, 255);
			bufferptr = buffer;
			continue;
		}

		*bufferptr = theMacroString[i];
		bufferptr++;
	}

	sprintf(evalText, "%lf", value);
	return evalText;
}

/*
==============
=
= EvaluateMacroMath
=
==============
*/
double TokenParser::EvaluateMacroMath(double value, double newval, char oper)
{
	switch (oper)
	{
	case '+': value += newval; break;
	case '-': value -= newval; break;
	case '*': value *= newval; break;
	case '/': value /= newval; break;
	}

	return value;
}

/*
==============
=
= isMacro
=
==============
*/
bool TokenParser::isMacro()
{
	if (!TokenAvailable(true))
		return false;

	SkipNonToken(true);
	if (*script_p == TOKENSPECIAL)
	{
		return true;
	}

	return false;
}

/*
==============
=
= GetLine
=
==============
*/

const char* TokenParser::GetLine(bool crossline)
{
	// is a token already waiting?
	if (tokenready)
	{
		tokenready = false;
		return token.c_str();
	}

	//
	// skip space
	//
	SkipNonToken(crossline);

	//
	// copy token
	//
	const char* start = script_p;
	SkipToEOL();
	size_t size = script_p - start;
	token.assign(start, size);

	return token.c_str();
}

/*
==============
=
= GetRaw
=
==============
*/

const char* TokenParser::GetRaw()
{
	const char* start;

	//
	// skip white space
	//
	SkipWhiteSpace(true);

	//
	// copy token
	//
	start = script_p;
	SkipToEOL();
	size_t size = script_p - start;
	token.assign(start, size);

	return token.c_str();
}

/*
==============
=
= GetString
=
==============
*/

const char* TokenParser::GetString(bool crossline, bool escape, bool allowMultiLines)
{
	int startline;

	// is a token already waiting?
	if (tokenready)
	{
		tokenready = false;
		return token.c_str();
	}

	//
	// skip space
	//
	SkipNonToken(crossline);

	if (*script_p != '"') {
		throw TokenErrors::ExpectedToken("string", line);
	}

	script_p++;

	startline = line;
	token = "";
	if(escape)
	{
		while (*script_p != '"')
		{
			if (*script_p == TOKENEOL)
			{
				if (!allowMultiLines) {
					throw TokenErrors::LineIncomplete(line);
				}

				++script_p;
				continue;
			}

			if ((*script_p == '\\') && (script_p < (end_p - 1)))
			{
				script_p++;
				switch (*script_p)
				{
				case 'n':	token.append(1, '\n'); break;
				case 'r':	token.append(1, '\n'); break;
				case '\'':  token.append(1, '\''); break;
				case '\"':  token.append(1, '\"'); break;
				case '\\':  token.append(1, '\\'); break;
				default: break;
				}
				script_p++;
			}
			else
			{
				token.append(1, *script_p++);
			}

			CheckOverflow();
		}
	}
	else
	{
		while (*script_p != '"')
		{
			token.append(1, *script_p++);

			CheckOverflow();
		}
	}

	// skip last quote
	script_p++;

	return token.c_str();
}

/*
==============
=
= GetSpecific
=
==============
*/

bool TokenParser::GetSpecific(const char* string)
{
	do
	{
		if (!TokenAvailable(true))
		{
			return false;
		}
		GetToken(true);
	} while (token != string);

	return true;
}

//===============================================================
// Name:		GetBoolean
// Class:		Script
//
// Description: Retrieves the next boolean value in the token
//				stream.  If the next token is either "true"
//				or "1", then it returns true.  Otherwise, it
//				returns false.
// 
// Parameters:	bool -- determines if token parsing can cross newlines
//
// Returns:		bool -- true if next token was "true" (or "1")
// 
//===============================================================
bool TokenParser::GetBoolean(bool crossline)
{
	GetToken(crossline);
	if (stricmp(token.c_str(), "true") == 0)
		return true;
	else if (stricmp(token.c_str(), "1") == 0)
		return true;

	return false;
}

/*
==============
=
= GetInteger
=
==============
*/

int TokenParser::GetInteger(bool crossline)
{
	GetToken(crossline);
	return atoi(token.c_str());
}

uint64_t TokenParser::GetInteger64(bool crossline)
{
	GetToken(crossline);
	return atoll(token.c_str());
}

/*
==============
=
= GetDouble
=
==============
*/

double TokenParser::GetDouble(bool crossline)
{
	GetToken(crossline);
	return atof(token.c_str());
}

/*
==============
=
= GetFloat
=
==============
*/

float TokenParser::GetFloat(bool crossline)
{
	return (float)GetDouble(crossline);
}

/*
==============
=
= GetVector
=
==============
*/

void TokenParser::GetVector(bool crossline, vec3r_t out)
{
	if (AtString(crossline))
	{
		const char* xyz = GetToken(crossline);
		VectorFromString(xyz, out);
	}
	else
	{
		out[0] = GetFloat(crossline);
		out[1] = GetFloat(crossline);
		out[2] = GetFloat(crossline);
	}
}

/*
===================
=
= LinesInFile
=
===================
*/
int TokenParser::LinesInFile()
{
	bool temp_tokenready;
	const char* temp_script_p;
	int temp_line;
	str temp_token;
	int numentries;

	temp_tokenready = tokenready;
	temp_script_p = script_p;
	temp_line = line;
	temp_token = token;

	numentries = 0;

	Reset();
	while (TokenAvailable(true))
	{
		GetLine(true);
		numentries++;
	}

	tokenready = temp_tokenready;
	script_p = temp_script_p;
	line = temp_line;
	token = temp_token;

	return numentries;
}

/*
==============
=
= Parse
=
==============
*/

void TokenParser::Parse(const char* data, size_t length, const char* name)
{
	Close();

	buffer = data;
	Reset();
	this->length = length;
	end_p = script_p + length;
	filename = name;
}

bool TokenParser::isValid()
{
	return !hasError;
}

bool TokenParser::EndOfFile()
{
	return script_p >= end_p;
}

const char* TokenParser::Token()
{
	return token.c_str();
}

const char* MOHPC::TokenParser::GetCurrentScript() const
{
	return script_p;
}
TokenErrors::EndOfFileReached::EndOfFileReached()
{
}

TokenErrors::EndOfFileReached::~EndOfFileReached()
{
}

const char* TokenErrors::EndOfFileReached::what() const noexcept
{
	return "End of token file reached prematurely while reading string";
}

TokenErrors::NoMacroTextFound::NoMacroTextFound(const char* macroNamePtr)
	: macroName(macroNamePtr)
{
}

TokenErrors::NoMacroTextFound::~NoMacroTextFound()
{
}

const char* TokenErrors::NoMacroTextFound::getMacroName() const
{
	return macroName.c_str();
}

const char* TokenErrors::NoMacroTextFound::what() const noexcept
{
	return "No Macro Text found";
}

TokenErrors::LineIncomplete::LineIncomplete(uint32_t lineNumValue)
	: lineNum(lineNumValue)
{
}

TokenErrors::LineIncomplete::~LineIncomplete()
{
}

uint32_t TokenErrors::LineIncomplete::getLineNum() const
{
	return lineNum;
}

const char* TokenErrors::LineIncomplete::what() const noexcept
{
	return "Line is incomplete while reading string";
}

TokenErrors::ExpectedToken::ExpectedToken(const char* expectedPtr, uint32_t lineNumValue)
	: expected(expectedPtr)
	, lineNum(lineNumValue)
{
}

TokenErrors::ExpectedToken::~ExpectedToken()
{
}

const char* TokenErrors::ExpectedToken::getExpected() const
{
	return expected;
}

uint32_t TokenErrors::ExpectedToken::getLineNum() const
{
	return lineNum;
}

const char* TokenErrors::ExpectedToken::what() const noexcept
{
	return "Expected another token";
}
