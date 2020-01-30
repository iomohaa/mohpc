#include <Shared.h>
#include <MOHPC/Script/ScriptException.h>
#include <stdarg.h>

using namespace MOHPC;

int ScriptException::next_abort = 0;
int ScriptException::next_bIsForAnim = 0;

void ScriptException::CreateException( const char *data )
{
	string = data;

	bAbort = next_abort;
	next_abort = 0;
	bIsForAnim = next_bIsForAnim;
	next_bIsForAnim = 0;
}

ScriptException::ScriptException(const str& text)
{
	CreateException( text.c_str() );
}

ScriptException::ScriptException( const char *format, ... )
{
	va_list va;
	char data[4100];

	va_start( va, format );
	vsprintf( data, format, va);
	va_end( va );

	CreateException( data );
}

ScriptException::ScriptException( char *text )
{
	CreateException( text );
}

void Error( const char * format, ... )
{
	va_list va;
	char data[4100];

	va_start( va, format );
	vsprintf( data, format, va);
	va_end( va );

	throw new ScriptException( ( const char * )data );
}
