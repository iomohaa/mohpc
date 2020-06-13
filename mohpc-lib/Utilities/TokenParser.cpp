#include <Shared.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Managers/FileManager.h>
#include <string.h>

using namespace MOHPC;

TokenParser::~TokenParser()
{
	Close();
}

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

void TokenParser::Close(void)
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

	//Loop Through the macro container and delete (del33t -hehe) them all
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

const char* TokenParser::Filename(void)
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

int TokenParser::GetLineNumber(void)
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

void TokenParser::Reset(void)
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

bool TokenParser::SkipToEOL(void)
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

void TokenParser::CheckOverflow(void)
{
	if (script_p >= end_p)
	{
		//glbs.Error(ERR_DROP, "End of token file reached prematurely reading %s\n", filename.c_str());
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

	while (*script_p <= TOKENSPACE)
	{
		if (*script_p++ == TOKENEOL)
		{
			if (!crossline)
			{
				//glbs.Error(ERR_DROP, "Line %i is incomplete in file %s\n", line, filename.c_str());
				return;
			}

			line++;
		}
		CheckOverflow();
	}
}

bool TokenParser::AtComment(void)
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

void TokenParser::UnGetToken(void)
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
	while ((*script_p > TOKENSPACE) && !AtComment())
	{
		if ((*script_p == '\\') && (script_p < (end_p - 1)))
		{
			script_p++;
			switch (*script_p)
			{
			case 'n':	token.append('\n'); break;
			case 'r':	token.append('\n'); break;
			case '\'': token.append('\''); break;
			case '\"': token.append('\"'); break;
			case '\\': token.append('\\'); break;
			default:	token.append(*script_p); break;
			}
			script_p++;
		}
		else
		{
			token.append(*script_p++);
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

	macrolist.AddObject(theMacro);

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

		if (!str::cmp(theMacro->macroName, theMacroName))
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
	//glbs.Error( ERR_DROP, "No Macro Text found for %s in file %s\n", theMacroName, filename.c_str() );
	return 0;

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
bool TokenParser::isMacro(void)
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

const char* TokenParser::GetRaw(void)
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

const char* TokenParser::GetString(bool crossline, bool escape)
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

	if (*script_p != '"')
	{
		//glbs.Error( ERR_DROP, "Expecting string on line %i in file %s\n", line, filename.c_str() );
		return "";
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
				//glbs.Error( ERR_DROP, "Line %i is incomplete while reading string in file %s\n", line, filename.c_str() );
				return "";
			}

			if ((*script_p == '\\') && (script_p < (end_p - 1)))
			{
				script_p++;
				switch (*script_p)
				{
				case 'n':	token.append('\n'); break;
				case 'r':	token.append('\n'); break;
				case '\'': token.append('\''); break;
				case '\"': token.append('\"'); break;
				case '\\': token.append('\\'); break;
				default: break;
				}
				script_p++;
			}
			else
			{
				token.append(*script_p++);
			}

			if (script_p >= end_p)
			{
				//glbs.Error( ERR_DROP, "End of token file reached prematurely while reading string on\n"
				//	"line %d in file %s\n", startline, filename.c_str() );
				return "";
			}
		}
	}
	else
	{
		while (*script_p != '"')
		{
			token.append(*script_p++);

			if (script_p >= end_p)
			{
				//glbs.Error( ERR_DROP, "End of token file reached prematurely while reading string on\n"
				//	"line %d in file %s\n", startline, filename.c_str() );
				return "";
			}
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

Vector TokenParser::GetVector(bool crossline)
{
	if (AtString(crossline))
	{
		const char* xyz = GetToken(crossline);
		return Vector(xyz);
	}
	else
	{
		float	x = GetFloat(crossline);
		float	y = GetFloat(crossline);
		float	z = GetFloat(crossline);
		return Vector(x, y, z);
	}
}

/*
===================
=
= LinesInFile
=
===================
*/
int TokenParser::LinesInFile(void)
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

bool TokenParser::EndOfFile(void)
{
	return script_p >= end_p;
}

const char* TokenParser::Token(void)
{
	return token.c_str();
}
