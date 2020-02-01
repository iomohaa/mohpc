#include <Shared.h>
#include <MOHPC/Script/ScriptThread.h>
#include <MOHPC/Script/ScriptContainer.h>
#include <MOHPC/Script/ScriptVM.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/Level.h>
#include <MOHPC/Script/SimpleEntity.h>
#include <MOHPC/Script/ConstStr.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EventManager.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace MOHPC
{
Event EV_ScriptThread_Abs
(
	"abs",
	EV_DEFAULT,
	"f",
	"arg",
	"Absolute value of int or float",
	EV_RETURN
);

Event EV_ScriptThread_AnglesPointAt
(
	"angles_pointat",
	EV_DEFAULT,
	"eee",
	"parent_entity entity target_entity",
	"Returns the angles that points at the target_entity given the base orientation of the parent_entity and the position of the entity.",
	EV_RETURN
);

Event EV_ScriptThread_AnglesToForward
(
	"angles_toforward",
	EV_DEFAULT,
	"v",
	"angles",
	"Returns the forward vector of the specified angles",
	EV_RETURN
);

Event EV_ScriptThread_AnglesToLeft
(
	"angles_toleft",
	EV_DEFAULT,
	"v",
	"angles",
	"Returns the left vector of the specified angles",
	EV_RETURN
);

Event EV_ScriptThread_AnglesToUp
(
	"angles_toup",
	EV_DEFAULT,
	"v",
	"angles",
	"Returns the up vector of the specified angles",
	EV_RETURN
);

Event EV_ScriptThread_Assert
(
	"assert",
	EV_DEFAULT,
	"f",
	"value",
	"Assert if value is 0.",
	EV_NORMAL
);

Event EV_ScriptThread_CastBoolean
(
	"bool",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to a bool.",
	EV_RETURN
);

Event EV_ScriptThread_CastEntity
(
	"entity",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to an entity.",
	EV_RETURN
);

Event EV_ScriptThread_CastFloat
(
	"float",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to a float.",
	EV_RETURN
);

Event EV_ScriptThread_CastInt
(
	"int",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to an int.",
	EV_RETURN
);

Event EV_ScriptThread_CastString
(
	"string",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to a string.",
	EV_RETURN
);

Event EV_ScriptThread_CreateListener
(
	"CreateListener",
	EV_DEFAULT,
	NULL,
	NULL,
	"Creates a Listener instance.",
	EV_RETURN
);

Event EV_ScriptThread_End
(
	"end",
	EV_DEFAULT,
	NULL,
	NULL,
	"Ends the script",
	EV_NORMAL
);

Event EV_ScriptThread_Timeout
(
	"timeout",
	EV_DEFAULT,
	"f",
	"time",
	"specifies script timeout time",
	EV_NORMAL
);

Event EV_ScriptThread_Error
(
	"error",
	EV_DEFAULT,
	NULL,
	NULL,
	"Ends the script",
	EV_NORMAL
);

Event EV_ScriptThread_GetSelf
(
	"self",
	EV_DEFAULT,
	NULL,
	NULL,
	"self",
	EV_GETTER
);

Event EV_ScriptThread_Goto
(
	"goto",
	EV_DEFAULT,
	"s",
	"label",
	"Goes to the specified label.",
	EV_NORMAL
);

Event EV_ScriptThread_MPrint
(
	"mprint",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string.",
	EV_NORMAL
);

Event EV_ScriptThread_MPrintln
(
	"mprintln",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string. followed by a newline",
	EV_NORMAL
);

Event EV_ScriptThread_Print
(
	"print",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string.",
	EV_NORMAL
);

Event EV_ScriptThread_Println
(
	"println",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string. followed by a newline.",
	EV_NORMAL
);

Event EV_ScriptThread_RandomFloat
(
	"randomfloat",
	EV_DEFAULT,
	"f",
	"max",
	"randomfloat",
	EV_RETURN
);

Event EV_ScriptThread_RandomInt
(
	"randomint",
	EV_DEFAULT,
	"i",
	"max",
	"randomint",
	EV_RETURN
);

Event EV_ScriptThread_RegisterCommand
(
	"registercmd",
	EV_DEFAULT,
	"ss",
	"name label",
	"Registers a command.",
	EV_NORMAL
);
Event EV_ScriptThread_KillClass
(
	"killclass",
	EV_CHEAT,
	"sI",
	"class_name except",
	"Kills everything in the specified class except for the specified entity (optional)."
);
Event EV_ScriptThread_RemoveClass
(
	"removeclass",
	EV_CHEAT,
	"sI",
	"class_name except",
	"Removes everything in the specified class except for the specified entity (optional)."
);

Event EV_ScriptThread_Trigger
(
	"trigger",
	EV_DEFAULT,
	"s",
	"name",
	"Trigger the specified target or entity."
);

Event EV_ScriptThread_Spawn
(
	"spawn",
	EV_DEFAULT,
	"sSSSSSSSS",
	"entityname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
	"Spawns an entity.",
	EV_NORMAL
);

Event EV_ScriptThread_SpawnReturn
(
	"spawn",
	EV_DEFAULT,
	"sSSSSSSSS",
	"entityname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
	"Spawns an entity.",
	EV_RETURN
);

Event EV_ScriptThread_VectorAdd
(
	"vector_add",
	EV_DEFAULT,
	"vv",
	"vector1 vector2",
	"Returns vector1 + vector2.",
	EV_RETURN
);

Event EV_ScriptThread_VectorCloser
(
	"vector_closer",
	EV_DEFAULT,
	"vvv",
	"vec_a vec_b vec_c",
	"returns 1 if the first vector is closer than the second vector to the third vector.",
	EV_RETURN
);

Event EV_ScriptThread_VectorCross
(
	"vector_cross",
	EV_DEFAULT,
	"vv",
	"vector1 vector2",
	"Returns vector1 x vector2.",
	EV_RETURN
);

Event EV_ScriptThread_VectorDot
(
	"vector_dot",
	EV_DEFAULT,
	"vv",
	"vector1 vector2",
	"Returns vector1 * vector2.",
	EV_RETURN
);

Event EV_ScriptThread_VectorLength
(
	"vector_length",
	EV_DEFAULT,
	"v",
	"vector",
	"Returns the length of the specified vector.",
	EV_RETURN
);

Event EV_ScriptThread_VectorNormalize
(
	"vector_normalize",
	EV_DEFAULT,
	"v",
	"vector",
	"Returns the normalized vector of the specified vector.",
	EV_RETURN
);

Event EV_ScriptThread_VectorScale
(
	"vector_scale",
	EV_DEFAULT,
	"vf",
	"vector1 scale_factor",
	"Returns vector1 * scale_factor.",
	EV_RETURN
);

Event EV_ScriptThread_VectorSubtract
(
	"vector_subtract",
	EV_DEFAULT,
	"vv",
	"vector1 vector2",
	"Returns vector1 - vector2.",
	EV_RETURN
);

Event EV_ScriptThread_VectorToAngles
(
	"vector_toangles",
	EV_DEFAULT,
	"v",
	"vector1",
	"Returns vector1 converted to angles.",
	EV_RETURN
);

Event EV_ScriptThread_VectorWithin
(
	"vector_within",
	EV_DEFAULT,
	"vvf",
	"position1 position2 distance",
	"returns 1 if the two points are <= distance apart, or 0 if they are greater than distance apart.",
	EV_RETURN
);

Event EV_ScriptThread_Wait
(
	"wait",
	EV_DEFAULT,
	"f",
	"wait_time",
	"Wait for the specified amount of time.",
	EV_NORMAL
);

Event EV_ScriptThread_WaitFrame
(
	"waitframe",
	EV_DEFAULT,
	NULL,
	NULL,
	"Wait for one server frame.",
	EV_NORMAL
);

//
// world stuff
//
Event EV_Cache
(
	"cache",
	EV_CACHE,
	"s",
	"resourceName",
	"pre-cache the given resource."
);

Event EV_ScriptThread_IsArray
(
	"isarray",
	EV_DEFAULT,
	"s",
	"object",
	"Checks whether this variable is an array",
	EV_RETURN
);

Event EV_ScriptThread_IsDefined
(
	"isdefined",
	EV_DEFAULT,
	"s",
	"object",
	"Checks whether this entity/variable is defined",
	EV_RETURN
);

Event EV_ScriptThread_FileOpen
(
	"fopen",
	EV_DEFAULT,
	"ss",
	"filename accesstype",
	"Opens file, returning it's handle",
	EV_RETURN
);

Event EV_ScriptThread_FileWrite
(
	"fwrite",
	EV_DEFAULT,
	"eii",
	"player index virtual",
	"Writes binary buffer to file",
	EV_NORMAL
);

Event EV_ScriptThread_FileRead
(
	"fread",
	EV_DEFAULT,
	"eii",
	"player index virtual",
	"Reads binary buffer from file",
	EV_NORMAL
);

Event EV_ScriptThread_FileClose
(
	"fclose",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Closes file of given file handle",
	EV_RETURN
);

Event EV_ScriptThread_FileEof
(
	"feof",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Checks for end of file",
	EV_RETURN
);

Event EV_ScriptThread_FileSeek
(
	"fseek",
	EV_DEFAULT,
	"iii",
	"filehandle offset startpos",
	"Sets file carret at given position",
	EV_RETURN
);

Event EV_ScriptThread_FileTell
(
	"ftell",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Gets current file carret position",
	EV_RETURN
);

Event EV_ScriptThread_FileRewind
(
	"frewind",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Rewinds file carret to files beginning",
	EV_NORMAL
);

Event EV_ScriptThread_FilePutc
(
	"fputc",
	EV_DEFAULT,
	"ii",
	"filehandle character",
	"Writes single character to file",
	EV_RETURN
);

Event EV_ScriptThread_FilePuts
(
	"fputs",
	EV_DEFAULT,
	"is",
	"filehandle text",
	"Writes string line to file",
	EV_RETURN
);

Event EV_ScriptThread_FileGetc
(
	"fgetc",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Reads single character from file",
	EV_RETURN
);

Event EV_ScriptThread_FileGets
(
	"fgets",
	EV_DEFAULT,
	"ii",
	"filehandle maxbuffsize",
	"Reads string line from file",
	EV_RETURN
);

Event EV_ScriptThread_FileError
(
	"ferror",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Checks for last file i/o error",
	EV_RETURN
);

Event EV_ScriptThread_FileFlush
(
	"fflush",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Flushes given stream. Writes all unsaved data from stream buffer to stream",
	EV_RETURN
);

Event EV_ScriptThread_FlagClear
(
	"flag_clear",
	EV_DEFAULT,
	"s",
	"name",
	"Clears and deletes a flag.",
	EV_NORMAL
);

Event EV_ScriptThread_FlagInit
(
	"flag_init",
	EV_DEFAULT,
	"s",
	"name",
	"Initializes a flag so it can be set. Flags MUST be initialized before they can be used for the first time.",
	EV_NORMAL
);

Event EV_ScriptThread_FlagSet
(
	"flag_set",
	EV_DEFAULT,
	"s",
	"name",
	"Sets a flag which alerts all flag_wait()'s and changes are reflected in flag() checks thereafter.",
	EV_NORMAL
);

Event EV_ScriptThread_FlagWait
(
	"flag_wait",
	EV_DEFAULT,
	"s",
	"name",
	"Pauses execution flow until a flag has been set.",
	EV_NORMAL
);

Event EV_ScriptThread_Lock
(
	"lock",
	EV_DEFAULT,
	"l",
	"lock",
	"Lock the thread."
);

Event EV_ScriptThread_UnLock
(
	"unlock",
	EV_DEFAULT,
	"l",
	"lock",
	"Unlock the thread."
);

Event EV_ScriptThread_GetArrayKeys
(
	"getarraykeys",
	EV_DEFAULT,
	"s",
	"array",
	"Retrieves a full list containing the name of arrays",
	EV_RETURN
);

Event EV_ScriptThread_GetArrayValues
(
	"getarrayvalues",
	EV_DEFAULT,
	"s",
	"array",
	"Retrieves the full list of an array that was set-up with name, such as local.array[ \"name\" ] and return their values",
	EV_RETURN
);

Event EV_ScriptThread_GetTime
(
	"gettime",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets current time",
	EV_RETURN
);

Event EV_ScriptThread_GetTimeZone
(
	"gettimezone",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets current time zone",
	EV_RETURN
);

Event EV_ScriptThread_PregMatch
(
	"preg_match",
	EV_DEFAULT,
	"ss",
	"pattern subject",
	"Searches subject for a match to the regular expression given in pattern.",
	EV_RETURN
);

Event EV_ScriptThread_GetDate
(
	"getdate",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets current date",
	EV_RETURN
);

Event EV_ScriptThread_CharToInt
(
	"chartoint",
	EV_DEFAULT,
	"s",
	"character",
	"Converts char to int",
	EV_RETURN
);


Event EV_ScriptThread_FileExists
(
	"fexists",
	EV_DEFAULT,
	"s",
	"filename",
	"Checks if file exists",
	EV_RETURN
);

Event EV_ScriptThread_FileReadAll
(
	"freadall",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Reads whole file and returns it as string",
	EV_RETURN
);

Event EV_ScriptThread_FileSaveAll
(
	"fsaveall",
	EV_DEFAULT,
	"is",
	"filehandle text",
	"Saves whole text to file",
	EV_RETURN
);

Event EV_ScriptThread_FileRemove
(
	"fremove",
	EV_DEFAULT,
	"s",
	"filename",
	"Removes the file",
	EV_RETURN
);

Event EV_ScriptThread_FileRename
(
	"frename",
	EV_DEFAULT,
	"ss",
	"oldfilename newfilename",
	"Renames the file",
	EV_RETURN
);

Event EV_ScriptThread_FileCopy
(
	"fcopy",
	EV_DEFAULT,
	"ss",
	"filename copyfilename",
	"Copies the file",
	EV_RETURN
);

Event EV_ScriptThread_FileReadPak
(
	"freadpak",
	EV_DEFAULT,
	"s",
	"filename",
	"Reads file from pak file",
	EV_RETURN
);

Event EV_ScriptThread_FileList
(
	"flist",
	EV_DEFAULT,
	"ssi",
	"path extension wantSubs",
	"Returns the list of files with given extension",
	EV_RETURN
);

Event EV_ScriptThread_FileNewDirectory
(
	"fnewdir",
	EV_DEFAULT,
	"s",
	"path",
	"Creates a new directory",
	EV_RETURN
);

Event EV_ScriptThread_FileRemoveDirectory
(
	"fremovedir",
	EV_DEFAULT,
	"s",
	"path",
	"Removes a directory",
	EV_RETURN
);


Event EV_ScriptThread_MathCos
(
	"cos",
	EV_DEFAULT,
	"f",
	"x",
	"Compute cosine",
	EV_RETURN
);

Event EV_ScriptThread_MathSin
(
	"sin",
	EV_DEFAULT,
	"f",
	"x",
	"Compute sine",
	EV_RETURN
);

Event EV_ScriptThread_MathTan
(
	"tan",
	EV_DEFAULT,
	"f",
	"x",
	"Compute tangent",
	EV_RETURN
);

Event EV_ScriptThread_MathACos
(
	"acos",
	EV_DEFAULT,
	"f",
	"x",
	"Compute arc cosine",
	EV_RETURN
);

Event EV_ScriptThread_MathASin
(
	"asin",
	EV_DEFAULT,
	"f",
	"x",
	"Compute arc sine",
	EV_RETURN
);

Event EV_ScriptThread_MathATan
(
	"atan",
	EV_DEFAULT,
	"f",
	"x",
	"Compute arc tangent",
	EV_RETURN
);

Event EV_ScriptThread_MathATan2
(
	"atan2",
	EV_DEFAULT,
	"ff",
	"x y",
	"Compute arc tangent with two parameters",
	EV_RETURN
);

Event EV_ScriptThread_MathCosH
(
	"cosh",
	EV_DEFAULT,
	"f",
	"x",
	"Compute hyperbolic cosine",
	EV_RETURN
);

Event EV_ScriptThread_MathSinH
(
	"sinh",
	EV_DEFAULT,
	"f",
	"x",
	"Compute hyperbolic sine",
	EV_RETURN
);

Event EV_ScriptThread_MathTanH
(
	"tanh",
	EV_DEFAULT,
	"f",
	"x",
	"Compute hyperbolic tangent",
	EV_RETURN
);

Event EV_ScriptThread_MathExp
(
	"exp",
	EV_DEFAULT,
	"f",
	"x",
	"Compute exponential function",
	EV_RETURN
);

Event EV_ScriptThread_MathFrexp
(
	"frexp",
	EV_DEFAULT,
	"f",
	"x",
	"Get significand and exponent",
	EV_RETURN
);

Event EV_ScriptThread_MathLdexp
(
	"ldexp",
	EV_DEFAULT,
	"f",
	"x",
	"Generate number from significand and exponent",
	EV_RETURN
);

Event EV_ScriptThread_MathLog
(
	"log",
	EV_DEFAULT,
	"f",
	"x",
	"Compute natural logarithm",
	EV_RETURN
);

Event EV_ScriptThread_MathLog10
(
	"log10",
	EV_DEFAULT,
	"f",
	"x",
	"Compute common logarithm",
	EV_RETURN
);

Event EV_ScriptThread_MathModf
(
	"modf",
	EV_DEFAULT,
	"f",
	"x",
	"Break into fractional and integral parts",
	EV_RETURN
);

Event EV_ScriptThread_MathPow
(
	"pow",
	EV_DEFAULT,
	"ff",
	"x y",
	"Raise to power",
	EV_RETURN
);

Event EV_ScriptThread_MathSqrt
(
	"sqrt",
	EV_DEFAULT,
	"f",
	"x",
	"Compute square root",
	EV_RETURN
);

Event EV_ScriptThread_MathCeil
(
	"ceil",
	EV_DEFAULT,
	"f",
	"x",
	"Round up value",
	EV_RETURN
);

Event EV_ScriptThread_MathFloor
(
	"floor",
	EV_DEFAULT,
	"f",
	"x",
	"Round down value",
	EV_RETURN
);

Event EV_ScriptThread_MathFmod
(
	"fmod",
	EV_DEFAULT,
	"f",
	"x",
	"Compute remainder of division",
	EV_RETURN
);

Event EV_ScriptThread_strncpy
(
	"strncpy",
	EV_DEFAULT,
	"is",
	"bytes source",
	"Returns the copied string with the specified bytes",
	EV_RETURN
);

Event EV_ScriptThread_TypeOf
(
	"typeof",
	EV_DEFAULT,
	"i",
	"variable",
	"Returns the type of variable",
	EV_RETURN
);

Event EV_ScriptThread_Md5String
(
	"md5string",
	EV_DEFAULT,
	"s",
	"text",
	"generates MD5 hash of given text",
	EV_RETURN
);

Event EV_ScriptThread_SetTimer
(
	"settimer",
	EV_DEFAULT,
	"is",
	"interval script",
	"Sets timer that will execute script after given interval.",
	EV_RETURN
);

Event EV_ScriptThread_RemoveArchivedClass
(
	"removearchivedclass",
	EV_DEFAULT,
	"sI",
	"class except_entity_number",
	"Removes all of the simple archived entities in the specified class.",
	EV_NORMAL
);

Event EV_ScriptThread_CancelWaiting
(
	"_cancelwaiting",
	EV_CODEONLY,
	NULL,
	NULL,
	"internal event"
);
};

using namespace MOHPC;

ScriptThread::ScriptThread()
{
	m_ScriptVM = NULL;
}

ScriptThread::ScriptThread(ScriptContainer *scriptClass, unsigned char *pCodePos)
{
	m_ScriptVM = new ScriptVM(scriptClass, pCodePos, this);
	m_ScriptVM->m_ThreadState = THREAD_RUNNING;
}

ScriptThread::~ScriptThread()
{
	ScriptVM* vm = m_ScriptVM;
	if (!vm)
	{
		//ScriptError("Attempting to delete a dead thread.");
		return;
	}

	m_ScriptVM = NULL;
	if (vm->ThreadState() == THREAD_WAITING)
	{
		vm->m_ThreadState = THREAD_RUNNING;
		GetScriptManager()->RemoveTiming(this);
	}
	else if (vm->ThreadState() == THREAD_SUSPENDED)
	{
		vm->m_ThreadState = THREAD_RUNNING;
		CancelWaitingAll();
	}

	vm->NotifyDelete();
}

void ScriptThread::CreateReturnThread(Event *ev)
{
	m_ScriptVM->GetScriptContainer()->CreateReturnThread(ev);
}

void ScriptThread::CreateThread(Event *ev)
{
	m_ScriptVM->GetScriptContainer()->CreateThread(ev);
}

void ScriptThread::ExecuteReturnScript(Event* ev)
{
	m_ScriptVM->GetScriptContainer()->ExecuteReturnScript(ev);
}

void ScriptThread::ExecuteScript(Event* ev)
{
	m_ScriptVM->GetScriptContainer()->ExecuteScript(ev);
}

void ScriptThread::EventCreateListener(Event* ev)
{
	ev->AddListener(new Listener());
}

void ScriptThread::CharToInt(Event* ev)
{
	str c = ev->GetString(1);

	ev->AddInteger(c[0]);
}

void ScriptThread::FileOpen(Event* ev)
{
	int numArgs = -1;
	str filename;
	str accesstype;
	FILE *f = NULL;
	char buf[16] = { 0 };


	numArgs = ev->NumArgs();

	if (numArgs != 2)
		ScriptError("Wrong arguments count for fopen!\n");

	filename = ev->GetString(1);

	accesstype = ev->GetString(2);

	f = fopen(filename, accesstype);

	if (f == NULL)
	{
		ev->AddInteger(0);
		return;
	}
	else
	{
		ev->AddInteger((int)(int64_t)f);
		return;
	}


}

void ScriptThread::FileWrite(Event* ev)
{

}

void ScriptThread::FileRead(Event* ev)
{

}

void ScriptThread::FileClose(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	FILE *f = NULL;
	char buf[16] = { 0 };

	numArgs = ev->NumArgs();

	if (numArgs != 1)
		ScriptError("Wrong arguments count for fclose!\n");

	id = ev->GetInteger(1);

	/*if( (int)scriptFiles[0].f != id && (int)scriptFiles[1].f != id )
	{
	gi.Printf("Wrong file handle for fclose!\n");
	return;
	}

	if( (int)scriptFiles[0].f == id )
	{
	scriptFiles[0].inUse = 0;
	fclose( scriptFiles[0].f );
	return;
	}
	else if( (int)scriptFiles[1].f == id )
	{
	scriptFiles[1].inUse = 0;
	fclose( scriptFiles[1].f );
	return;
	}
	else
	{
	gi.Printf("Unknown error while closing file - fclose!\n");
	return;
	}*/

	f = (FILE *)id;

	if (f == NULL) {
		ScriptError("File handle is NULL for fclose!\n");
	}

	ret = fclose(f);

	if (ret == 0)
	{
		ev->AddInteger(0);
		return;
	}
	else
	{
		ev->AddInteger(ret);
		return;
	}


}

void ScriptThread::FileEof(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	FILE *f = NULL;

	numArgs = ev->NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for feof!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	ret = feof(f);

	ev->AddInteger(ret);
}

void ScriptThread::FileSeek(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int pos = 0;
	long int offset = 0;
	int ret = 0;
	FILE *f = NULL;

	numArgs = ev->NumArgs();

	if (numArgs != 3) {
		ScriptError("Wrong arguments count for fseek!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	offset = ev->GetInteger(2);

	if (offset < 0) {
		ScriptError("Wrong file offset! Should be starting from 0. - fseek\n");
	}

	pos = ev->GetInteger(3);

	if (pos != 0 && pos != 1 && pos != 2) {
		ScriptError("Wrong file offset start! Should be between 0 - 2! - fseek\n");
	}

	ret = fseek(f, offset, pos);

	ev->AddInteger(ret);


}

void ScriptThread::FileTell(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	long int ret = 0;
	FILE *f = NULL;

	numArgs = ev->NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for ftell!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	ret = ftell(f);

	ev->AddInteger(ret);
}

void ScriptThread::FileRewind(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	long int ret = 0;
	FILE *f = NULL;

	numArgs = ev->NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for frewind!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	rewind(f);

}

void ScriptThread::FilePutc(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	FILE *f = NULL;
	int c = 0;

	numArgs = ev->NumArgs();

	if (numArgs != 2) {
		ScriptError("Wrong arguments count for fputc!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	c = ev->GetInteger(2);

	ret = fputc((char)c, f);

	ev->AddInteger(ret);
}

void ScriptThread::FilePuts(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	FILE *f = NULL;
	str c;

	numArgs = ev->NumArgs();

	if (numArgs != 2) {
		ScriptError("Wrong arguments count for fputs!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	c = ev->GetString(2);
	//gi.Printf("Putting line into a file\n");
	ret = fputs(c, f);
	//gi.Printf("Ret val: %i\n", ret);
	ev->AddInteger(ret);
}

void ScriptThread::FileGetc(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	FILE *f = NULL;

	numArgs = ev->NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for fgetc!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	ret = fgetc(f);

	ev->AddInteger(ret);
}

void ScriptThread::FileGets(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int maxCount = 0;
	FILE *f = NULL;
	char *c = NULL;
	char *buff = NULL;

	numArgs = ev->NumArgs();

	if (numArgs != 2) {
		ScriptError("Wrong arguments count for fgets!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	maxCount = ev->GetInteger(2);

	if (maxCount <= 0) {
		ScriptError("Maximum buffer size should be higher than 0! - fgets\n");
	}

	buff = (char*)malloc(maxCount + 1);

	if (buff == NULL)
	{
		ScriptError("Failed to allocate memory during fputs scriptCommand text buffer initialization! Try setting maximum buffer length lower.\n");
		ev->AddInteger(-1);
	}

	memset(buff, 0, maxCount + 1);

	c = fgets(buff, maxCount, f);

	if (c == NULL)
		ev->AddString("");
	else
		ev->AddString(c);

	free(buff);
}

void ScriptThread::FileError(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	FILE *f = NULL;

	numArgs = ev->NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for ferror!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	ret = ferror(f);

	ev->AddInteger(ret);
}

void ScriptThread::FileFlush(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	FILE *f = NULL;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		ScriptError("Wrong arguments count for fflush!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	ret = fflush(f);

	ev->AddInteger(ret);

}

void ScriptThread::FileExists(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	FILE *f = 0;
	str filename;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		ScriptError("Wrong arguments count for fexists!\n");
	}

	filename = ev->GetString(1);

	if (filename == NULL)
	{
		ScriptError("Empty file name passed to fexists!\n");
	}

	f = fopen(filename, "r");
	if (f)
	{
		fclose(f);
		ev->AddInteger(1);
	}
	else
	{
		ev->AddInteger(0);
	}

}

void ScriptThread::FileReadAll(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	FILE *f = NULL;
	char *ret = NULL;
	long currentPos = 0;
	size_t size = 0;
	size_t sizeRead = 0;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		ScriptError("Wrong arguments count for freadall!\n");
	}

	id = ev->GetInteger(1);

	f = (FILE *)id;

	currentPos = ftell(f);
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, currentPos, SEEK_SET);

	ret = (char*)malloc(sizeof(char) * size + 1);

	if (ret == NULL)
	{
		ev->AddInteger(-1);
		ScriptError("Error while allocating memory buffer for file content - freadall!\n");
	}

	sizeRead = fread(ret, 1, size, f);
	ret[sizeRead] = '\0';

	ev->AddString(ret);

	free(ret);
}

void ScriptThread::FileSaveAll(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	FILE *f = NULL;
	size_t sizeWrite = 0;
	str text;

	numArgs = ev->NumArgs();

	if (numArgs != 2)
	{
		ScriptError("Wrong arguments count for fsaveall!\n");
	}

	id = ev->GetInteger(1);
	f = (FILE *)id;

	text = ev->GetString(2);

	if (text == NULL)
	{
		ev->AddInteger(-1);
		ScriptError("Text to be written is NULL - fsaveall!\n");
	}

	sizeWrite = fwrite(text, 1, strlen(text), f);

	ev->AddInteger((int)sizeWrite);
}

void ScriptThread::FileRemove(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	str filename;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		ScriptError("Wrong arguments count for fremove!\n");
	}

	filename = ev->GetString(1);

	if (filename == NULL)
	{
		ScriptError("Empty file name passed to fremove!\n");
	}

	ret = remove(filename);

	ev->AddInteger(ret);

}

void ScriptThread::FileRename(Event* ev)
{
	int id = 0;
	int numArgs = 0;
	int ret = 0;
	str oldfilename, newfilename;

	numArgs = ev->NumArgs();

	if (numArgs != 2)
	{
		ScriptError("Wrong arguments count for frename!\n");
	}

	oldfilename = ev->GetString(1);
	newfilename = ev->GetString(2);

	if (!oldfilename)
	{
		ScriptError("Empty old file name passed to frename!\n");
	}

	if (!newfilename)
	{
		ScriptError("Empty new file name passed to frename!\n");
	}

	ret = rename(oldfilename, newfilename);

	ev->AddInteger(ret);

}

void ScriptThread::FileCopy(Event* ev)
{
	size_t n = 0;
	int numArgs = 0;
	unsigned int ret = 0;
	str filename, copyfilename;
	FILE *f = NULL, *fCopy = NULL;
	char buffer[4096];

	numArgs = ev->NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for fcopy!\n");
		return;
	}

	filename = ev->GetString(1);
	copyfilename = ev->GetString(2);

	if (!filename)
	{
		//glbs.Printf("Empty file name passed to fcopy!\n");
		return;
	}

	if (copyfilename)
	{
		//glbs.Printf("Empty copy file name passed to fcopy!\n");
		return;
	}

	f = fopen(filename, "rb");

	if (f == NULL)
	{
		//glbs.Printf("Could not open \"%s\" for copying - fcopy!\n", filename.c_str());
		ev->AddInteger(-1);
		return;
	}

	fCopy = fopen(copyfilename, "wb");

	if (fCopy == NULL)
	{
		fclose(f);
		//glbs.Printf("Could not open \"%s\" for copying - fcopy!\n", copyfilename.c_str());
		ev->AddInteger(-2);
		return;
	}

	while ((n = fread(buffer, sizeof(char), sizeof(buffer), f)) > 0)
	{
		if (fwrite(buffer, sizeof(char), n, fCopy) != n)
		{
			fclose(f);
			fflush(fCopy);
			fclose(fCopy);
			//glbs.Printf("There was an error while copying files - fcopy!\n");
			ev->AddInteger(-3);
			return;
		}
	}

	fclose(f);
	fflush(fCopy);
	fclose(fCopy);

	ev->AddInteger(0);
}

void ScriptThread::FileReadPak(Event* ev)
{
	str filename;
	const char *content = NULL;
	int numArgs = 0;
	int ret = 0;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for freadpak!\n");
		return;
	}

	filename = ev->GetString(1);

	if (filename == NULL)
	{
		//glbs.Printf("Filename is NULL - freadpak!\n");
		return;
	}

	FilePtr file = GetFileManager()->OpenFile(filename);
	if (file)
	{
		file->ReadBuffer((void**)&content);
	}

	if (content == NULL)
	{
		//glbs.Printf("Error while reading pak file content - freadpak!\n");
		ev->AddInteger(-1);
		return;
	}

	ev->AddString(content);
}

void ScriptThread::FileList(Event* ev)
{
	int i = 0, numArgs = 0;
	const char *path = NULL;
	str extension;
	bool wantSubs = 0;
	size_t numFiles = 0;
	FileEntryList list;
	ScriptVariable *ref = new ScriptVariable;
	ScriptVariable *array = new ScriptVariable;

	numArgs = ev->NumArgs();

	if (numArgs != 3)
	{
		//glbs.Printf("Wrong arguments count for flist!\n");
		return;
	}

	path = ev->GetString(1);
	extension = ev->GetString(2);
	wantSubs = ev->GetBoolean(3);

	FileManager* FM = GetFileManager();

	list = FM->ListFilteredFiles(path, extension, wantSubs);
	numFiles = list.GetNumFiles();

	if (numFiles == 0)
	{
		return;
	}

	ref->setRefValue(array);

	for (i = 0; i < numFiles; i++)
	{
		ScriptVariable *indexes = new ScriptVariable;
		ScriptVariable *values = new ScriptVariable;

		indexes->setIntValue(i);
		values->setStringValue(list.GetFileEntry(i)->GetRawName());

		ref->setArrayAt(*indexes, *values);
	}

	ev->AddValue(*array);

	return;

}

void ScriptThread::FileNewDirectory(Event* ev)
{
	str path;
	int numArgs = 0;
	int ret = 0;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for fnewdir!\n");
		return;
	}

	path = ev->GetString(1);

	if (path == NULL)
	{
		//glbs.Printf("Path is NULL - fnewdir!\n");
		return;
	}

#ifdef WIN32
	//ret = _mkdir(path);
#else
	//ret = mkdir(path, 0777);
#endif

	ev->AddInteger(ret);
	return;
}

void ScriptThread::FileRemoveDirectory(Event* ev)
{
	str path;
	int numArgs = 0;
	int ret = 0;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for fremovedir!\n");
		return;
	}

	path = ev->GetString(1);

	if (path == NULL)
	{
		//glbs.Printf("Path is NULL - fremovedir!\n");
		return;
	}

#ifdef _WIN32
	ret = _rmdir(path);
#else
	ret = rmdir(path);
#endif

	ev->AddInteger(ret);
	return;
}

void ScriptThread::GetArrayKeys(Event* ev)
{
	Entity *ent = NULL;
	ScriptVariable array;
	ScriptVariable *value;
	size_t arraysize;

	/* Retrieve the array */
	array = ev->GetValue(1);

	/* Cast the array */
	array.CastConstArrayValue();
	arraysize = array.arraysize();

	if (arraysize < 1) {
		return;
	}

	ScriptVariable *ref = new ScriptVariable, *newArray = new ScriptVariable;

	ref->setRefValue(newArray);

	for (uintptr_t i = 1; i <= arraysize; i++)
	{
		value = array[i];

		/* Get the array's name */
		//str name = value->getName();

		//glbs.Printf("name = %s\n", value->GetTypeName());

		ScriptVariable *newIndex = new ScriptVariable, *newValue = new ScriptVariable;

		newIndex->setIntValue((int)i);
		newValue->setStringValue("NIL");

		//name.removeRef();

		ref->setArrayAt(*newIndex, *newValue);
	}

	ev->AddValue(*newArray);
}

void ScriptThread::GetArrayValues(Event* ev)
{
	Entity *ent = NULL;
	ScriptVariable array;
	ScriptVariable *value;
	size_t arraysize;

	/* Retrieve the array */
	array = ev->GetValue(1);

	if (array.GetType() == VARIABLE_NONE) {
		return;
	}

	/* Cast the array */
	array.CastConstArrayValue();
	arraysize = array.arraysize();

	if (arraysize < 1) {
		return;
	}

	ScriptVariable *ref = new ScriptVariable, *newArray = new ScriptVariable;

	ref->setRefValue(newArray);

	for (uintptr_t i = 1; i <= arraysize; i++)
	{
		value = array[i];

		ScriptVariable *newIndex = new ScriptVariable;

		newIndex->setIntValue((int)(i - 1));

		ref->setArrayAt(*newIndex, *value);
	}

	ev->AddValue(*newArray);
}

void ScriptThread::GetDate(Event* ev)
{
	char buff[1024];
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buff, 64, "%d.%m.%Y %r", timeinfo);

	ev->AddString(buff);
}

void ScriptThread::GetTimeZone(Event* ev)
{
	int gmttime;
	int local;

	time_t rawtime;
	struct tm * timeinfo, *ptm;

	int timediff;
	int tmp;

	tmp = ev->GetInteger(1);

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	local = timeinfo->tm_hour;

	ptm = gmtime(&rawtime);

	gmttime = ptm->tm_hour;

	timediff = local - gmttime;

	ev->AddInteger(timediff);
}

void ScriptThread::PregMatch(Event* ev)
{
	/*
	slre_cap sl_cap[32];
	int i, j;
	size_t iMaxLength;
	size_t iLength;
	size_t iFoundLength = 0;
	str pattern, subject;
	ScriptVariable index, value, subindex, subvalue;
	ScriptVariable array, subarray;

	memset(sl_cap, 0, sizeof(sl_cap));

	pattern = ev->GetString(1);
	subject = ev->GetString(2);

	iMaxLength = strlen(subject);
	iLength = 0;
	i = 0;

	while (iLength < iMaxLength &&
		(iFoundLength = slre_match(pattern, subject.c_str() + iLength, iMaxLength - iLength, sl_cap, sizeof(sl_cap) / sizeof(sl_cap[0]), 0)) > 0)
	{
		subarray.Clear();

		for (j = 0; sl_cap[j].ptr != NULL; j++)
		{
			char *buffer;

			buffer = (char *)glbs.Malloc(sl_cap[j].len + 1);
			buffer[sl_cap[j].len] = 0;
			strncpy(buffer, sl_cap[j].ptr, sl_cap[j].len);

			subindex.setIntValue(j);
			subvalue.setStringValue(buffer);
			subarray.setArrayAtRef(subindex, subvalue);

			glbs.Free(buffer);

			iLength += sl_cap[j].ptr - subject.c_str();
		}

		index.setIntValue(i);
		array.setArrayAtRef(index, subarray);

		i++;
	}

	ev->AddValue(array);
	*/
}

void ScriptThread::EventIsArray(Event* ev)
{
	ScriptVariable * value = &ev->GetValue(1);

	if (value == NULL)
	{
		return ev->AddInteger(0);
	}

	ev->AddInteger(value->type == VARIABLE_ARRAY || value->type == VARIABLE_CONSTARRAY || value->type == VARIABLE_SAFECONTAINER);
}

void ScriptThread::EventIsDefined(Event* ev)
{
	ev->AddInteger(!ev->IsNilAt(1));
}

void ScriptThread::FlagClear(Event* ev)
{
	str name;
	Flag *flag;

	name = ev->GetString(1);

	flag = GetScriptManager()->flags.FindFlag(name);

	if (flag == NULL)
	{
		ScriptError("Invalid flag '%s'\n", name.c_str());
	}

	delete flag;
}

void ScriptThread::FlagInit(Event* ev)
{
	str name;
	Flag *flag;

	name = ev->GetString(1);

	flag = GetScriptManager()->flags.FindFlag(name);

	if (flag != NULL)
	{
		flag->Reset();
		return;
	}

	flag = new Flag;
	flag->bSignaled = false;
	flag->flagName = name;
}

void ScriptThread::FlagSet(Event* ev)
{
	str name;
	Flag *flag;

	name = ev->GetString(1);

	flag = GetScriptManager()->flags.FindFlag(name);

	if (flag == NULL)
	
	{
		ScriptError("Invalid flag '%s'.\n", name.c_str());
	}

	flag->Set();
}

void ScriptThread::FlagWait(Event* ev)
{
	str name;
	Flag *flag;

	name = ev->GetString(1);

	flag = GetScriptManager()->flags.FindFlag(name);

	if (flag == NULL)
	{
		ScriptError("Invalid flag '%s'.\n", name.c_str());
	}

	flag->Wait(this);
}

void ScriptThread::Lock(Event* ev)
{
	ScriptMutex *pMutex = (ScriptMutex *)ev->GetListener(1);

	if (!pMutex)
	{
		ScriptError("Invalid mutex.");
	}

	pMutex->Lock();
}

void ScriptThread::UnLock(Event* ev)
{
	ScriptMutex *pMutex = (ScriptMutex *)ev->GetListener(1);

	if (!pMutex)
	{
		ScriptError("Invalid mutex.");
	}

	pMutex->Unlock();
}

void ScriptThread::MathCos(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for cos!\n");
		return;
	}

	x = (double)ev->GetFloat(1);
	res = cos(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathSin(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for sin!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = sin(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathTan(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for tan!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = tan(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathACos(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for acos!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = acos(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathASin(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for asin!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = asin(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathATan(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for atan!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = atan(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathATan2(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, y = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for atan2!\n");
		return;
	}

	y = ev->GetFloat(1);
	x = ev->GetFloat(2);

	res = atan2(y, x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathCosH(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for cosh!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = cosh(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathSinH(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for sinh!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = sinh(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathTanH(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for tanh!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = tanh(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathExp(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for exp!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = exp(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathFrexp(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;
	int exp = 0;
	ScriptVariable *ref = new ScriptVariable;
	ScriptVariable *array = new ScriptVariable;
	ScriptVariable *SignificandIndex = new ScriptVariable;
	ScriptVariable *ExponentIndex = new ScriptVariable;
	ScriptVariable *SignificandVal = new ScriptVariable;
	ScriptVariable *ExponentVal = new ScriptVariable;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for frexp!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = frexp(x, &exp);

	ref->setRefValue(array);

	SignificandIndex->setStringValue("significand");
	ExponentIndex->setStringValue("exponent");

	SignificandVal->setFloatValue((float)res);
	ExponentVal->setIntValue(exp);

	ref->setArrayAt(*SignificandIndex, *SignificandVal);
	ref->setArrayAt(*ExponentIndex, *ExponentVal);

	ev->AddValue(*array);
}

void ScriptThread::MathLdexp(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;
	int exp = 0;

	numArgs = ev->NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for ldexp!\n");
		return;
	}

	x = ev->GetFloat(1);
	exp = ev->GetInteger(2);

	res = ldexp(x, exp);

	ev->AddFloat((float)res);
}

void ScriptThread::MathLog(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for log!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = log(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathLog10(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for log10!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = log10(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathModf(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;
	double intpart = 0;
	ScriptVariable *array = new ScriptVariable;
	ScriptVariable *ref = new ScriptVariable;
	ScriptVariable *IntpartIndex = new ScriptVariable;
	ScriptVariable *FractionalIndex = new ScriptVariable;
	ScriptVariable *FractionalVal = new ScriptVariable;
	ScriptVariable *IntpartVal = new ScriptVariable;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for modf!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = modf(x, &intpart);

	ref->setRefValue(array);

	IntpartIndex->setStringValue("intpart");
	FractionalIndex->setStringValue("fractional");
	FractionalVal->setFloatValue((float)res);
	IntpartVal->setFloatValue((float)intpart);

	ref->setArrayAt(*IntpartIndex, *IntpartVal);
	ref->setArrayAt(*FractionalIndex, *FractionalVal);

	ev->AddValue(*array);
}

void ScriptThread::MathPow(Event* ev)
{
	int numArgs = 0;
	double base = 0.0f, res = 0.0f;
	int exponent = 0;

	numArgs = ev->NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for pow!\n");
		return;
	}

	base = ev->GetFloat(1);
	exponent = ev->GetInteger(2);
	res = pow(base, exponent);

	ev->AddFloat((float)res);
}

void ScriptThread::MathSqrt(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for sqrt!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = sqrt(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathCeil(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for ceil!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = ceil(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathFloor(Event* ev)
{
	int numArgs = 0;
	double x = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for floor!\n");
		return;
	}

	x = ev->GetFloat(1);
	res = floor(x);

	ev->AddFloat((float)res);
}

void ScriptThread::MathFmod(Event* ev)
{
	int numArgs = 0;
	double numerator = 0.0f, denominator = 0.0f, res = 0.0f;

	numArgs = ev->NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for fmod!\n");
		return;
	}

	numerator = ev->GetFloat(1);
	denominator = ev->GetFloat(2);
	res = fmod(numerator, denominator);

	ev->AddFloat((float)res);
}

/*
int checkMD5(const char *filepath, char *md5Hash)
{
	md5_state_t state;
	md5_byte_t digest[16];
	int di;

	FILE *f = NULL;
	char *buff = NULL;
	size_t filesize = 0;
	size_t bytesread = 0;


	f = fopen(filepath, "rb");

	if (f == NULL)
		return -1;

	fseek(f, 0, SEEK_END);
	filesize = ftell(f);
	rewind(f);

	////glbs.Printf("Size: %i\n", filesize);

	buff = (char *)glbs.Malloc(filesize + 1);

	if (buff == NULL)
	{
		fclose(f);
		Com_Printf("error0\n");
		return -2;
	}

	buff[filesize] = '\0';

	bytesread = fread(buff, 1, filesize, f);

	if (bytesread < filesize)
	{
		glbs.Free(buff);
		fclose(f);
		Com_Printf("error1: %i\n", bytesread);
		return -3;
	}

	fclose(f);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)buff, filesize);
	md5_finish(&state, digest);

	for (di = 0; di < 16; ++di)
		sprintf(md5Hash + di * 2, "%02x", digest[di]);


	glbs.Free(buff);

	return 0;
}

int checkMD5String(const char *string, char *md5Hash)
{
	md5_state_t state;
	md5_byte_t digest[16];
	int di;

	char *buff = NULL;
	size_t stringlen = 0;

	stringlen = strlen(string);

	buff = (char *)glbs.Malloc(stringlen + 1);

	if (buff == NULL)
	{
		return -1;
	}

	buff[stringlen] = '\0';
	memcpy(buff, string, stringlen);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)buff, stringlen);
	md5_finish(&state, digest);

	for (di = 0; di < 16; ++di)
		sprintf(md5Hash + di * 2, "%02x", digest[di]);


	glbs.Free(buff);

	return 0;
}
*/

void ScriptThread::Md5File(Event* ev)
{
	/*
	char hash[64];
	str filename = NULL;
	int ret = 0;

	if (ev->NumArgs() != 1)
	{
		ScriptError("Wrong arguments count for md5file!\n");
		return;
	}

	filename = ev->GetString(1);

	ret = checkMD5(filename, hash);
	if (ret != 0)
	{
		ev->AddInteger(-1);
		ScriptError("Error while generating MD5 checksum for file - md5file!\n");
		return;
	}

	ev->AddString(hash);
	*/
}

void ScriptThread::Md5String(Event* ev)
{
	/*
	char hash[64];
	str text = NULL;
	int ret = 0;

	if (ev->NumArgs() != 1)
	{
		ScriptError("Wrong arguments count for md5string!\n");
		return;
	}

	text = ev->GetString(1);

	ret = checkMD5String(text, hash);
	if (ret != 0)
	{
		ev->AddInteger(-1);
		ScriptError("Error while generating MD5 checksum for strin!\n");
		return;
	}

	ev->AddString(hash);
	*/
}

void ScriptThread::TypeOfVariable(Event* ev)
{
	int numArgs = 0;
	char *type = NULL;
	ScriptVariable * variable;

	numArgs = ev->NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for typeof!\n");
		return;
	}

	variable = (ScriptVariable*)&ev->GetValue(1);
	type = (char*)variable->GetTypeName();

	ev->AddString(type);
}

void ScriptThread::CancelWaiting(Event* ev)
{
	CancelWaitingAll();
}

void ScriptThread::Archive(Archiver &arc)
{
}

void ScriptThread::ArchiveInternal(Archiver& arc)
{
	Listener::Archive(arc);

	//arc.ArchiveObjectPosition(this);
	m_ScriptVM->Archive(arc);
}

void ScriptThread::Abs(Event* ev)
{
	ev->AddFloat(fabsf(ev->GetFloat(1)));
}

void ScriptThread::RemoveArchivedClass(Event* ev)
{
	str classname;
	int except_entity_number = -1;

	classname = ev->GetString(1);

	if (ev->NumArgs() > 1)
	{
		except_entity_number = ev->GetInteger(1);
	}

	Level* level = GetGameManager()->GetLevel();

	for (intptr_t i = level->GetNumSimpleArchivedEntities(); i > 0; i--)
	{
		SimpleArchivedEntity *m_SimpleArchivedEntity = level->GetSimpleArchivedEntity(i);

		if (m_SimpleArchivedEntity == NULL)
		{
			continue;
		}

		// If found, remove the simple archived entity
		if (m_SimpleArchivedEntity->inheritsFrom(classname))
		{
			m_SimpleArchivedEntity->PostEvent(EV_Remove, EV_REMOVE);
		}
	}

	GetEventManager()->ProcessPendingEvents();
}

void ScriptThread::SetTimer(Event* ev)
{
	int interval = -1;
	void* scr_var = NULL;
	int i = 0;
	Event *event;

	if (ev->NumArgs() != 2)
	{
		ScriptError("Wrong arguments count for settimer!\n");
		return;
	}

	interval = ev->GetInteger(1);

	if (interval <= 0)
	{
		ev->AddInteger(1);
		return;
	}

	event = new Event(EV_Listener_ExecuteScript);
	event->AddValue(ev->GetValue(2));

	PostEvent(event, (float)interval / 1000.0f);
}

void ScriptThread::Angles_ToForward(Event* ev)
{
	Vector fwd;

	ev->GetVector(1).AngleVectorsLeft(&fwd);
	ev->AddVector(fwd);
}

void ScriptThread::Angles_ToLeft(Event* ev)
{
	Vector left;

	ev->GetVector(1).AngleVectorsLeft(NULL, &left);
	ev->AddVector(left);
}

void ScriptThread::Angles_ToUp(Event* ev)
{
	Vector up;

	ev->GetVector(1).AngleVectorsLeft(NULL, NULL, &up);
	ev->AddVector(up);
}

void ScriptThread::Assert(Event* ev)
{
	assert(ev->GetFloat(1));
}

void ScriptThread::Cache(Event* ev)
{
}

void ScriptThread::CastBoolean(Event* ev)
{
	ev->AddInteger(ev->GetBoolean(1));
}

void ScriptThread::CastEntity(Event* ev)
{
	ev->AddListener((Listener *)ev->GetEntity(1));
}

void ScriptThread::CastFloat(Event* ev)
{
	ev->AddFloat(ev->GetFloat(1));
}

void ScriptThread::CastInt(Event* ev)
{
	ev->AddInteger(ev->GetInteger(1));
}

void ScriptThread::CastString(Event* ev)
{
	ev->AddString(ev->GetString(1));
}

void ScriptThread::EventDelayThrow(Event* ev)
{
	if (!m_ScriptVM->m_PrevCodePos) {
		return;
	}

	if (m_ScriptVM->EventThrow(ev))
	{
		if (m_ScriptVM->State() == STATE_EXECUTION)
		{
			Wait(0);
		}
		else
		{
			Stop();

			m_ScriptVM->Resume();
		}
	}
	else
	{
		// we make sure this won't get deleted
		SafePtr< ScriptThread > This = this;

		Stop();

		if (!BroadcastEvent(0, *ev))
		{
			m_ScriptVM->GetScriptContainer()->EventDelayThrow(ev);
		}

		if (This)
		{
			delete this;
		}
	}
}

void ScriptThread::EventEnd(Event* ev)
{
	if (ev->NumArgs() > 0)
	{
		ScriptVariable value = ev->GetValue(1);

		m_ScriptVM->End(value);

		ev->AddValue(value);
	}
	else
	{
		m_ScriptVM->End();
	}
}

void ScriptThread::EventTimeout(Event* ev)
{
	//Director.maxTime = ev->GetFloat(1) * 1000.0f + 0.5f;
}

void ScriptThread::EventError(Event* ev)
{
	if (ev->NumArgs() > 1)
	{
		ScriptException::next_abort = 0;
	}
	else
	{
		ScriptException::next_abort = ev->GetInteger(2);
		if (ScriptException::next_abort < 0)
		{
			ScriptException::next_abort = 0;
		}
	}

	ScriptError(ev->GetString(1));
}

void ScriptThread::EventGoto(Event* ev)
{
	m_ScriptVM->EventGoto(ev);

	if (m_ScriptVM->State() == STATE_EXECUTION)
	{
		ScriptExecuteInternal();
	}
	else
	{
		Stop();
		m_ScriptVM->Resume();
	}
}

void ScriptThread::EventRegisterCommand(Event* ev)
{
}

void ScriptThread::EventThrow(Event* ev)
{
	if (!m_ScriptVM->m_PrevCodePos) {
		return;
	}

	if (m_ScriptVM->EventThrow(ev))
	{
		if (m_ScriptVM->State() == STATE_EXECUTION)
		{
			ScriptExecuteInternal();
		}
		else
		{
			Stop();

			m_ScriptVM->Resume();
		}
	}
	else
	{
		// we make sure this won't get deleted
		SafePtr< ScriptThread > This = this;

		Stop();

		if (!BroadcastEvent("", *ev))
		{
			m_ScriptVM->GetScriptContainer()->EventThrow(ev);
		}

		if (This)
		{
			delete this;
		}
	}
}

void ScriptThread::EventWait(Event* ev)
{
	Wait(ev->GetFloat(1));
}

void ScriptThread::EventWaitFrame(Event* ev)
{
	Wait(GetGameManager()->GetLevel()->GetFrameTime());
}

void ScriptThread::GetSelf(Event* ev)
{
	ev->AddListener(m_ScriptVM->GetScriptContainer()->GetSelf());
}

void ScriptThread::Println(Event* ev)
{
}

void ScriptThread::Print(Event* ev)
{
}

void ScriptThread::MPrintln(Event* ev)
{
	SimpleEntity *m_Self = (SimpleEntity *)m_ScriptVM->GetScriptContainer()->GetSelf();

	if (!m_Self || !m_Self->isSubclassOf(SimpleEntity))
	{
		return;
	}

	MPrint(ev);
	m_Self->MPrintf("\n");
}

void ScriptThread::MPrint(Event* ev)
{
	SimpleEntity *m_Self = (SimpleEntity *)m_ScriptVM->GetScriptContainer()->GetSelf();

	if (!m_Self || !m_Self->isSubclassOf(SimpleEntity))
	{
		return;
	}

	for (int i = 1; i <= ev->NumArgs(); i++)
	{
		m_Self->MPrintf(ev->GetString(i));
	}
}

void ScriptThread::RandomFloat(Event* ev)
{
	ev->AddFloat(Random(ev->GetFloat(1)));
}

void ScriptThread::RandomInt(Event* ev)
{
	ev->AddInteger((int)Random((float)ev->GetInteger(1)));
}

void ScriptThread::Spawn(Event* ev)
{
	Listener *listener = SpawnInternal(ev);

	/*
	if (listener && checkInheritance(&Object::ClassInfo, listener->classinfo()))
	{
		ScriptError("You must specify an explicit classname for misc object tik models");
	}
	*/
}

Listener *ScriptThread::SpawnInternal(Event* ev)
{
	/*
	SpawnArgs args;
	str classname;
	Listener *l;

	if (ev->NumArgs() <= 0)
	{
		ScriptError("Usage: spawn entityname [keyname] [value]...");
	}

	classname = ev->GetString(1);

	if (getClassForID(classname) || getClass(classname))
	{
		args.setArg("classname", classname);
	}
	else
	{
		if (!strstr(classname.c_str(), ".tik"))
		{
			classname.append(".tik");
		}

		args.setArg("model", classname);
	}

	for (int i = 2; i < ev->NumArgs(); i += 2)
	{
		args.setArg(ev->GetString(i), ev->GetString(i + 1));
	}

	if (!args.getClassDef())
	{
		ScriptError("'%s' is not a valid entity name", classname.c_str());
	}

	const char *spawntarget = args.getArg("spawntarget");

	if (spawntarget)
	{
		SimpleEntity *target = G_FindTarget(NULL, spawntarget);

		if (!target)
		{
			ScriptError("Can't find targetname %s", spawntarget);
		}

		args.setArg("origin", va("%f %f %f", target->origin[0], target->origin[1], target->origin[2]));
		args.setArg("angle", va("%f", target->angles[1]));
	}

	level.spawnflags = 0;

	const char *s = args.getArg("spawnflags");
	if (s) {
		level.spawnflags = atoi(s);
	}

	level.m_bScriptSpawn = true;
	l = args.Spawn();
	level.m_bScriptSpawn = false;

	if (level.m_bRejectSpawn)
	{
		level.m_bRejectSpawn = false;
		ScriptError("Spawn command rejected for %s", classname.c_str());
	}

	return l;
	*/
	return nullptr;
}

void ScriptThread::SpawnReturn(Event* ev)
{
	Listener *listener = SpawnInternal(ev);

	ev->AddListener(listener);

	/*
	if (listener && checkInheritance(&Object::ClassInfo, listener->classinfo()))
	{
		ScriptError("You must specify an explicit classname for misc object tik models");
	}
	*/
}

void ScriptThread::EventVectorAdd(Event* ev)
{
	ev->AddVector(ev->GetVector(1) + ev->GetVector(2));
}

void ScriptThread::EventVectorCloser(Event* ev)
{

}

void ScriptThread::EventVectorCross(Event* ev)
{
	ev->AddVector(Vector::Cross(ev->GetVector(1), ev->GetVector(2)));
}

void ScriptThread::EventVectorDot(Event* ev)
{
	Vector vector1 = ev->GetVector(1), vector2 = ev->GetVector(2);

	ev->AddVector(Vector(vector1.x * vector2.x, vector1.y * vector2.y, vector1.z * vector2.z));
}

void ScriptThread::EventVectorLength(Event* ev)
{
	ev->AddFloat(ev->GetVector(1).length());
}

void ScriptThread::EventVectorNormalize(Event* ev)
{
	Vector vector = ev->GetVector(1);

	vector.normalize();

	ev->AddVector(vector);
}

void ScriptThread::EventVectorScale(Event* ev)
{
	Vector vector = ev->GetVector(1);

	vector *= ev->GetFloat(2);

	ev->AddVector(vector);
}

void ScriptThread::EventVectorSubtract(Event* ev)
{
	ev->AddVector(ev->GetVector(1) - ev->GetVector(2));
}

void ScriptThread::EventVectorToAngles(Event* ev)
{
	ev->AddVector(ev->GetVector(1).toAngles());
}

void ScriptThread::EventVectorWithin(Event* ev)
{
	Vector delta;
	float dist = ev->GetFloat(2);

	delta = ev->GetVector(1) - ev->GetVector(2);

	// check squared distance
	ev->AddInteger(((delta * delta) < (dist * dist)));
}

void ScriptThread::GetTime(Event* ev)
{
	int timearray[3], gmttime;
	char buff[1024];

	time_t rawtime;
	struct tm * timeinfo, *ptm;

	int timediff;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	timearray[0] = timeinfo->tm_hour;
	timearray[1] = timeinfo->tm_min;
	timearray[2] = timeinfo->tm_sec;

	ptm = gmtime(&rawtime);

	gmttime = ptm->tm_hour;

	timediff = timearray[0] - gmttime;

	sprintf(buff, "%02i:%02i:%02i", (int)timearray[0], (int)timearray[1], (int)timearray[2]);

	ev->AddString(buff);
}

void ScriptThread::Execute(Event& ev)
{
	Execute(&ev);
}

void ScriptThread::Execute(Event* ev)
{
	assert(m_ScriptVM);

	try
	{
		if (ev == NULL)
		{
			ScriptExecuteInternal();
		}
		else
		{
			ScriptVariable returnValue;

			returnValue.newPointer();

			ScriptExecute(ev->data, ev->dataSize, returnValue);

			ev->AddValue(returnValue);
		}
	}
	catch (ScriptException& exc)
	{
		if (exc.bAbort)
		{
			//glbs.Error(ERR_DROP, "%s\n", exc.string.c_str());
		}
		else
		{
			//Com_Printf("^~^~^ Script Error: %s\n", exc.string.c_str());
		}
	}
}

void ScriptThread::DelayExecute(Event& ev)
{
	DelayExecute(&ev);
}

void ScriptThread::DelayExecute(Event* ev)
{
	assert(m_ScriptVM);

	if (ev)
	{
		ScriptVariable returnValue;

		m_ScriptVM->SetFastData(ev->data, ev->dataSize);

		returnValue.newPointer();
		m_ScriptVM->m_ReturnValue = returnValue;
		ev->AddValue(returnValue);
	}

	GetScriptManager()->AddTiming(this, 0);
}

void ScriptThread::AllowContextSwitch(bool allow)
{
	m_ScriptVM->AllowContextSwitch(allow);
}

ScriptContainer *ScriptThread::GetScriptContainer(void)
{
	return m_ScriptVM->m_ScriptContainer;
}

int ScriptThread::GetThreadState(void)
{
	return m_ScriptVM->ThreadState();
}

ScriptThread *ScriptThread::GetWaitingContext(void)
{
	return m_WaitingContext;
}

void ScriptThread::SetWaitingContext(ScriptThread *thread)
{
	m_WaitingContext = thread;
}

void ScriptThread::HandleContextSwitch(ScriptThread *childThread)
{
	if (childThread->GetThreadState() == THREAD_CONTEXT_SWITCH)
	{
		// so, we request a context switch
		m_ScriptVM->RequestContextSwitch();

		SetWaitingContext(childThread);
	}
}

void ScriptThread::ScriptExecute(ScriptVariable *data, int dataSize, ScriptVariable& returnValue)
{
	m_ScriptVM->m_ReturnValue = returnValue;

	ScriptExecuteInternal(data, dataSize);
}

void ScriptThread::ScriptExecuteInternal(ScriptVariable *data, int dataSize)
{
	ScriptManager* Director = GetScriptManager();
	SafePtr< ScriptThread > previousThread = Director->m_PreviousThread;
	SafePtr< ScriptThread > currentThread = Director->m_CurrentThread;

	Director->m_PreviousThread = previousThread;
	Director->m_CurrentThread = this;

	Stop();
	m_ScriptVM->Execute(data, dataSize);

	// restore the previous values
	Director->m_PreviousThread = previousThread;
	Director->m_CurrentThread = currentThread;

	Director->ExecuteRunning();
}

void ScriptThread::StoppedNotify(void)
{
	// This is invalid and we mustn't get here
	if (m_ScriptVM) {
		delete this;
	}
}

void ScriptThread::StartedWaitFor(void)
{
	Stop();

	m_ScriptVM->m_ThreadState = THREAD_SUSPENDED;
	m_ScriptVM->Suspend();
}

void ScriptThread::StoppedWaitFor(const_str name, bool bDeleting)
{
	if (!m_ScriptVM)
	{
		return;
	}

	// The thread is deleted if the listener is deleting
	if (bDeleting)
	{
		delete this;
		return;
	}

	CancelEventsOfType(EV_ScriptThread_CancelWaiting);

	if (m_ScriptVM->m_ThreadState == THREAD_SUSPENDED)
	{
		if (name != 0)
		{
			if (m_ScriptVM->state == STATE_EXECUTION)
			{
				Execute();
			}
			else
			{
				m_ScriptVM->Resume();
			}
		}
		else
		{
			m_ScriptVM->m_ThreadState = THREAD_RUNNING;
			CancelWaitingAll();
			m_ScriptVM->m_ThreadState = THREAD_WAITING;

			GetScriptManager()->AddTiming(this, 0.0f);
		}
	}
}

void ScriptThread::Pause()
{
	Stop();
	m_ScriptVM->Suspend();
}

void ScriptThread::Stop(void)
{
	if (m_ScriptVM->ThreadState() == THREAD_WAITING)
	{
		m_ScriptVM->m_ThreadState = THREAD_RUNNING;
		GetScriptManager()->RemoveTiming(this);
	}
	else if (m_ScriptVM->ThreadState() == THREAD_SUSPENDED)
	{
		m_ScriptVM->m_ThreadState = THREAD_RUNNING;
		CancelWaitingAll();
	}
}

void ScriptThread::Wait(float time)
{
	Stop();

	m_ScriptVM->m_ThreadState = THREAD_WAITING;

	GetScriptManager()->AddTiming(this, time);
	m_ScriptVM->Suspend();
}

CLASS_DECLARATION(Listener, ScriptThread, NULL)
{
	{ &EV_Listener_CreateReturnThread,			&ScriptThread::CreateReturnThread },
	{ &EV_Listener_CreateThread,				&ScriptThread::CreateThread },
	{ &EV_Listener_ExecuteReturnScript,			&ScriptThread::ExecuteReturnScript },
	{ &EV_Listener_ExecuteScript,				&ScriptThread::ExecuteScript },
	{ &EV_ScriptThread_Abs,						&ScriptThread::Abs },
	{ &EV_ScriptThread_AnglesToForward,			&ScriptThread::Angles_ToForward },
	{ &EV_ScriptThread_AnglesToLeft,			&ScriptThread::Angles_ToLeft },
	{ &EV_ScriptThread_AnglesToUp,				&ScriptThread::Angles_ToUp },
	{ &EV_ScriptThread_Assert,					&ScriptThread::Assert },
	{ &EV_Cache,								&ScriptThread::Cache },
	{ &EV_ScriptThread_CastBoolean,				&ScriptThread::CastBoolean },
	{ &EV_ScriptThread_CastEntity,				&ScriptThread::CastEntity },
	{ &EV_ScriptThread_CastFloat,				&ScriptThread::CastFloat },
	{ &EV_ScriptThread_CastInt,					&ScriptThread::CastInt },
	{ &EV_ScriptThread_CastString,				&ScriptThread::CastString },
	{ &EV_ScriptThread_CreateListener,			&ScriptThread::EventCreateListener },
	{ &EV_DelayThrow,							&ScriptThread::EventDelayThrow },
	{ &EV_ScriptThread_End,						&ScriptThread::EventEnd },
	{ &EV_ScriptThread_Timeout,					&ScriptThread::EventTimeout },
	{ &EV_ScriptThread_Error,					&ScriptThread::EventError },
	{ &EV_ScriptThread_GetSelf,					&ScriptThread::GetSelf },
	{ &EV_ScriptThread_Goto,					&ScriptThread::EventGoto },
	{ &EV_ScriptThread_Println,					&ScriptThread::Println },
	{ &EV_ScriptThread_Print,					&ScriptThread::Print },
	{ &EV_ScriptThread_MPrintln,				&ScriptThread::MPrintln },
	{ &EV_ScriptThread_MPrint,					&ScriptThread::MPrint },
	{ &EV_ScriptThread_RandomFloat,				&ScriptThread::RandomFloat },
	{ &EV_ScriptThread_RandomInt,				&ScriptThread::RandomInt },
	{ &EV_ScriptThread_RegisterCommand,			&ScriptThread::EventRegisterCommand },
	{ &EV_ScriptThread_Spawn,					&ScriptThread::Spawn },
	{ &EV_ScriptThread_SpawnReturn,				&ScriptThread::SpawnReturn },
	{ &EV_Throw,								&ScriptThread::EventThrow },
	{ &EV_ScriptThread_VectorAdd,				&ScriptThread::EventVectorAdd },
	{ &EV_ScriptThread_VectorCloser,			&ScriptThread::EventVectorCloser },
	{ &EV_ScriptThread_VectorCross,				&ScriptThread::EventVectorCross },
	{ &EV_ScriptThread_VectorDot,				&ScriptThread::EventVectorDot },
	{ &EV_ScriptThread_VectorLength,			&ScriptThread::EventVectorLength },
	{ &EV_ScriptThread_VectorNormalize,			&ScriptThread::EventVectorNormalize },
	{ &EV_ScriptThread_VectorScale,				&ScriptThread::EventVectorScale },
	{ &EV_ScriptThread_VectorSubtract,			&ScriptThread::EventVectorSubtract },
	{ &EV_ScriptThread_VectorToAngles,			&ScriptThread::EventVectorToAngles },
	{ &EV_ScriptThread_VectorWithin,			&ScriptThread::EventVectorWithin },
	{ &EV_ScriptThread_Wait,					&ScriptThread::EventWait },
	{ &EV_ScriptThread_WaitFrame,				&ScriptThread::EventWaitFrame },
	{ &EV_ScriptThread_IsArray,					&ScriptThread::EventIsArray },
	{ &EV_ScriptThread_IsDefined,				&ScriptThread::EventIsDefined },
	{ &EV_ScriptThread_MathACos,				&ScriptThread::MathACos },
	{ &EV_ScriptThread_MathASin,				&ScriptThread::MathASin },
	{ &EV_ScriptThread_MathATan,				&ScriptThread::MathATan },
	{ &EV_ScriptThread_MathATan2,				&ScriptThread::MathATan2 },
	{ &EV_ScriptThread_MathCeil,				&ScriptThread::MathCeil },
	{ &EV_ScriptThread_MathCos,					&ScriptThread::MathCos },
	{ &EV_ScriptThread_MathCosH,				&ScriptThread::MathCosH },
	{ &EV_ScriptThread_MathExp,					&ScriptThread::MathExp },
	{ &EV_ScriptThread_MathFloor,				&ScriptThread::MathFloor },
	{ &EV_ScriptThread_MathFmod,				&ScriptThread::MathFmod },
	{ &EV_ScriptThread_MathFrexp,				&ScriptThread::MathFrexp },
	{ &EV_ScriptThread_MathLdexp,				&ScriptThread::MathLdexp },
	{ &EV_ScriptThread_MathLog,					&ScriptThread::MathLog },
	{ &EV_ScriptThread_MathLog10,				&ScriptThread::MathLog10 },
	{ &EV_ScriptThread_MathModf,				&ScriptThread::MathModf },
	{ &EV_ScriptThread_MathPow,					&ScriptThread::MathPow },
	{ &EV_ScriptThread_MathSin,					&ScriptThread::MathSin },
	{ &EV_ScriptThread_MathSinH,				&ScriptThread::MathSinH },
	{ &EV_ScriptThread_MathSqrt,				&ScriptThread::MathSqrt },
	{ &EV_ScriptThread_MathTan,					&ScriptThread::MathTan },
	{ &EV_ScriptThread_MathTanH,				&ScriptThread::MathTanH },
	{ &EV_ScriptThread_Md5String,				&ScriptThread::Md5String },
	{ &EV_ScriptThread_TypeOf,					&ScriptThread::TypeOfVariable },
	{ &EV_ScriptThread_CancelWaiting,			&ScriptThread::CancelWaiting },
	{ &EV_ScriptThread_GetTime,					&ScriptThread::GetTime },
	{ &EV_ScriptThread_GetTimeZone,				&ScriptThread::GetTimeZone },
	{ &EV_ScriptThread_PregMatch,				&ScriptThread::PregMatch },
	{ &EV_ScriptThread_FlagClear,				&ScriptThread::FlagClear },
	{ &EV_ScriptThread_FlagInit,				&ScriptThread::FlagInit },
	{ &EV_ScriptThread_FlagSet,					&ScriptThread::FlagSet },
	{ &EV_ScriptThread_FlagWait,				&ScriptThread::FlagWait },
	{ &EV_ScriptThread_Lock,					&ScriptThread::Lock },
	{ &EV_ScriptThread_UnLock,					&ScriptThread::UnLock },

	{ &EV_ScriptThread_CharToInt,				&ScriptThread::CharToInt },
	{ &EV_ScriptThread_FileClose,				&ScriptThread::FileClose },
	{ &EV_ScriptThread_FileCopy,				&ScriptThread::FileCopy },
	{ &EV_ScriptThread_FileEof,					&ScriptThread::FileEof },
	{ &EV_ScriptThread_FileError,				&ScriptThread::FileError },
	{ &EV_ScriptThread_FileExists,				&ScriptThread::FileExists },
	{ &EV_ScriptThread_FileFlush,				&ScriptThread::FileFlush },
	{ &EV_ScriptThread_FileGetc,				&ScriptThread::FileGetc },
	{ &EV_ScriptThread_FileGets,				&ScriptThread::FileGets },
	{ &EV_ScriptThread_FileList,				&ScriptThread::FileList },
	{ &EV_ScriptThread_FileNewDirectory,		&ScriptThread::FileNewDirectory },
	{ &EV_ScriptThread_FileOpen,				&ScriptThread::FileOpen },
	{ &EV_ScriptThread_FilePutc,				&ScriptThread::FilePutc },
	{ &EV_ScriptThread_FilePuts,				&ScriptThread::FilePuts },
	{ &EV_ScriptThread_FileRead,				&ScriptThread::FileRead },
	{ &EV_ScriptThread_FileReadAll,				&ScriptThread::FileReadAll },
	{ &EV_ScriptThread_FileRemove,				&ScriptThread::FileRemove },
	{ &EV_ScriptThread_FileRemoveDirectory,		&ScriptThread::FileRemoveDirectory },
	{ &EV_ScriptThread_FileRename,				&ScriptThread::FileRename },
	{ &EV_ScriptThread_FileRewind,				&ScriptThread::FileRewind },
	{ &EV_ScriptThread_FileSaveAll,				&ScriptThread::FileSaveAll },
	{ &EV_ScriptThread_FileSeek,				&ScriptThread::FileSeek },
	{ &EV_ScriptThread_FileTell,				&ScriptThread::FileTell },
	{ &EV_ScriptThread_FileWrite,				&ScriptThread::FileWrite },
	{ &EV_ScriptThread_GetArrayKeys,			&ScriptThread::GetArrayKeys },
	{ &EV_ScriptThread_GetArrayValues,			&ScriptThread::GetArrayValues },
	{ &EV_ScriptThread_GetDate,					&ScriptThread::GetDate },
	{ &EV_ScriptThread_RemoveArchivedClass,		&ScriptThread::RemoveArchivedClass },
	{ &EV_ScriptThread_SetTimer,				&ScriptThread::SetTimer },
	{ NULL, NULL }
};

CLASS_DECLARATION(Listener, ScriptMutex, NULL)
{
	{ NULL, NULL }
};

ScriptMutex::ScriptMutex()
{
	m_iLockCount = 0;
	LL_Reset(&m_list, next, prev);
}

ScriptMutex::~ScriptMutex()
{
	mutex_thread_list_t *list, *next;

	list = m_list.next;
	while (!LL_Empty(&m_list, next, prev))
	{
		next = list->next;
		LL_Remove(list, next, prev);
		delete list;
		list = next;
	}
}

void ScriptMutex::setOwner(ScriptThread *pThread)
{
	m_pLockThread = pThread;
}

void ScriptMutex::Lock(mutex_thread_list_t *pList)
{
	ScriptThread *pThread = pList->m_pThread;

	if (!m_pLockThread)
	{
		// Acquire ownership
		setOwner(pThread);
	}
	else
	{
		// Wait for the owner to unlock
		Register(0, pThread);
	}

	m_iLockCount++;
}

void ScriptMutex::Lock(void)
{
	mutex_thread_list_t *list;

	ScriptManager* Director = GetScriptManager();

	list = new mutex_thread_list_t;
	list->m_pThread = Director->CurrentThread();
	LL_Add(&m_list, list, next, prev);

	Lock(list);
}

void ScriptMutex::Unlock(void)
{
	mutex_thread_list_t *list, *next;

	ScriptManager* Director = GetScriptManager();

	m_iLockCount--;

	list = m_list.next;
	while (list != &m_list)
	{
		if (list->m_pThread == Director->CurrentThread())
		{
			next = list->next;
			LL_Remove(list, next, prev);
			delete list;
		}
		else
		{
			next = list->next;
		}

		list = next;
	}

	if (Director->CurrentThread() == m_pLockThread)
	{
		m_pLockThread = NULL;

		if (!LL_Empty(&m_list, next, prev))
		{
			list = m_list.next;

			setOwner(list->m_pThread);
			Unregister(0, list->m_pThread);
		}
	}
}

void ScriptMutex::StoppedNotify(void)
{
	ScriptManager* Director = GetScriptManager();

	if (Director->CurrentThread() == m_pLockThread)
	{
		// Safely unlock in case the thread is exiting
		Unlock();
	}
}

Flag *FlagList::FindFlag(const char * name)
{
	for (int i = 0; i < m_Flags.NumObjects(); i++)
	{
		Flag * index = m_Flags[i];

		// found the flag
		if (strcmp(index->flagName, name) == 0) {
			return index;
		}
	}

	return NULL;
}

void FlagList::AddFlag(Flag *flag)
{
	m_Flags.AddObject(flag);
}

void FlagList::RemoveFlag(Flag *flag)
{
	m_Flags.RemoveObject(flag);
}

Flag::Flag()
{
	GetScriptManager()->flags.AddFlag(this);
}

Flag::~Flag()
{
	GetScriptManager()->flags.RemoveFlag(this);

	m_WaitList.FreeObjectList();
}

void Flag::Reset()
{
	bSignaled = false;
}

void Flag::Set()
{
	// Don't signal again
	if (bSignaled)
	{
		return;
	}

	bSignaled = true;

	for (int i = 0; i < m_WaitList.NumObjects(); i++)
	{
		ScriptVM *Thread = m_WaitList[i];

		if (Thread->state != STATE_DESTROYED && Thread->m_Thread != NULL)
		{
			Thread->m_Thread->StoppedWaitFor(STRING_EMPTY, false);
		}
	}

	// Clear the list
	m_WaitList.FreeObjectList();
}

void Flag::Wait(ScriptThread *Thread)
{
	// Don't wait if it's signaled
	if (bSignaled)
	{
		return;
	}

	Thread->StartedWaitFor();

	m_WaitList.AddObject(Thread->m_ScriptVM);
}


