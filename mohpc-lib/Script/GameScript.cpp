#include <Shared.h>
#include <MOHPC/Script/GameScript.h>
#include <MOHPC/Script/Compiler.h>
#include <MOHPC/Script/ScriptVariable.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/ScriptManager.h>

using namespace MOHPC;

GameScript::GameScript()
{
	m_Filename = 0;
	successCompile = false;

	m_ProgBuffer = NULL;
	m_ProgLength = 0;
	m_bPrecompiled = false;

	requiredStackSize = 0;

	m_State = new StateScript();
	m_State->m_Parent = this;
}

GameScript::GameScript(const char *filename)
{
	m_Filename = GetScriptManager()->AddString(filename);
	successCompile = false;

	m_ProgBuffer = NULL;
	m_ProgLength = 0;
	m_ProgToSource = NULL;
	m_bPrecompiled = false;

	m_SourceBuffer = NULL;
	m_SourceLength = 0;

	requiredStackSize = 0;

	m_State = new StateScript();
	m_State->m_Parent = this;
}

GameScript::~GameScript()
{
	Close();

	if (m_State)
	{
		delete m_State;
	}
}

struct pfixup_t {
	bool			isString;
	unsigned int	*ptr;
};

static Container< const_str > archivedEvents;
static Container< const_str > archivedStrings;
static Container< pfixup_t * > archivedPointerFixup;

void ArchiveOpcode(Archiver& arc, unsigned char *code)
{
	/*
	unsigned int index;

	arc.ArchiveByte(code);

	switch (*code)
	{
	case OP_STORE_NIL:
	case OP_STORE_NULL:
	case OP_DONE:
		break;

	case OP_EXEC_CMD_COUNT1:
	case OP_EXEC_CMD_METHOD_COUNT1:
	case OP_EXEC_METHOD_COUNT1:
		arc.ArchiveByte(code + 1);
		goto __exec;

	case OP_EXEC_CMD0:
	case OP_EXEC_CMD1:
	case OP_EXEC_CMD2:
	case OP_EXEC_CMD3:
	case OP_EXEC_CMD4:
	case OP_EXEC_CMD5:
	case OP_EXEC_CMD_METHOD0:
	case OP_EXEC_CMD_METHOD1:
	case OP_EXEC_CMD_METHOD2:
	case OP_EXEC_CMD_METHOD3:
	case OP_EXEC_CMD_METHOD4:
	case OP_EXEC_CMD_METHOD5:
	case OP_EXEC_METHOD0:
	case OP_EXEC_METHOD1:
	case OP_EXEC_METHOD2:
	case OP_EXEC_METHOD3:
	case OP_EXEC_METHOD4:
	case OP_EXEC_METHOD5:
		code--;
	__exec:
		if (!arc.Loading())
		{
			index = archivedEvents.AddUniqueObject(*reinterpret_cast<unsigned int *>(code + 2));
		}

		arc.ArchiveUnsigned(&index);

		if (arc.Loading())
		{
			pfixup_t *p = new pfixup_t;

			p->isString = false;
			p->ptr = reinterpret_cast<unsigned int *>(code + 2);

			*reinterpret_cast<unsigned int *>(code + 2) = index;
			archivedPointerFixup.AddObject(p);
		}
		break;

	case OP_LOAD_FIELD_VAR:
	case OP_LOAD_GAME_VAR:
	case OP_LOAD_GROUP_VAR:
	case OP_LOAD_LEVEL_VAR:
	case OP_LOAD_LOCAL_VAR:
	case OP_LOAD_OWNER_VAR:
	case OP_LOAD_PARM_VAR:
	case OP_LOAD_SELF_VAR:
	case OP_LOAD_STORE_GAME_VAR:
	case OP_LOAD_STORE_GROUP_VAR:
	case OP_LOAD_STORE_LEVEL_VAR:
	case OP_LOAD_STORE_LOCAL_VAR:
	case OP_LOAD_STORE_OWNER_VAR:
	case OP_LOAD_STORE_PARM_VAR:
	case OP_LOAD_STORE_SELF_VAR:
	case OP_STORE_FIELD:
	case OP_STORE_FIELD_REF:
	case OP_STORE_GAME_VAR:
	case OP_STORE_GROUP_VAR:
	case OP_STORE_LEVEL_VAR:
	case OP_STORE_LOCAL_VAR:
	case OP_STORE_OWNER_VAR:
	case OP_STORE_PARM_VAR:
	case OP_STORE_SELF_VAR:
	case OP_STORE_STRING:
		if (!arc.Loading())
		{
			index = archivedStrings.AddUniqueObject(*reinterpret_cast<unsigned int *>(code + 1));
		}

		arc.ArchiveUnsigned(&index);

		if (arc.Loading())
		{
			pfixup_t *p = new pfixup_t;

			p->isString = true;
			p->ptr = reinterpret_cast<unsigned int *>(code + 1);

			*reinterpret_cast<unsigned int *>(code + 1) = index;
			archivedPointerFixup.AddObject(p);
		}
		break;

	default:
		if (OpcodeLength(*code) > 1)
			arc.ArchiveRaw(code + 1, OpcodeLength(*code) - 1);
	}
	*/
}

template<>
void Entry < unsigned char *, sourceinfo_t >::Archive(Archiver& arc)
{
	/*
	unsigned int offset;

	if (arc.Loading())
	{
		arc.ArchiveUnsigned(&offset);
		key = current_progBuffer + offset;
	}
	else
	{
		offset = key - current_progBuffer;
		arc.ArchiveUnsigned(&offset);
	}

	arc.ArchiveUnsigned(&value.sourcePos);
	arc.ArchiveInteger(&value.column);
	arc.ArchiveInteger(&value.line);
	*/
}

void GameScript::Archive(Archiver& arc)
{
	/*
	int count = 0, i;
	unsigned char *p, *code_pos, *code_end;
	const_str s;
	command_t *c, cmd;

	arc.ArchiveSize((long *)&m_ProgLength);

	if (arc.Saving())
	{
		p = m_ProgBuffer;
		current_progBuffer = m_ProgBuffer;

		// archive opcodes
		while (*p != OP_DONE)
		{
			ArchiveOpcode(arc, p);

			p += OpcodeLength(*p);
		}

		ArchiveOpcode(arc, p);

		// archive string dictionary list
		i = archivedStrings.NumObjects();
		arc.ArchiveInteger(&i);

		for (; i > 0; i--)
		{
			GetScriptManager()->ArchiveString(arc, archivedStrings.ObjectAt(i));
		}

		// archive event list
		i = archivedEvents.NumObjects();
		arc.ArchiveInteger(&i);

		for (; i > 0; i--)
		{
			c = Event::GetEventInfo(archivedEvents.ObjectAt(i));

			arc.ArchiveString(&c->command);
			arc.ArchiveInteger(&c->flags);
			arc.ArchiveByte(&c->type);
		}
	}
	else
	{
		m_ProgBuffer = (unsigned char *)glbs.Malloc(m_ProgLength);
		code_pos = m_ProgBuffer;
		code_end = m_ProgBuffer + m_ProgLength;

		current_progBuffer = m_ProgBuffer;

		do
		{
			ArchiveOpcode(arc, code_pos);

			code_pos += OpcodeLength(*code_pos);
		} while (*code_pos != OP_DONE && arc.NoErrors());

		if (!arc.NoErrors())
		{
			return;
		}

		// retrieve the string dictionary list
		arc.ArchiveInteger(&i);
		archivedStrings.Resize(i + 1);

		for (; i > 0; i--)
		{
			GetScriptManager()->ArchiveString(arc, s);
			archivedStrings.AddObjectAt(i, s);
		}

		// retrieve the event list
		arc.ArchiveInteger(&i);
		archivedEvents.Resize(i + 1);

		for (; i > 0; i--)
		{
			arc.ArchiveString(&cmd.command);
			arc.ArchiveInteger(&cmd.flags);
			arc.ArchiveByte(&cmd.type);

			archivedEvents.AddObjectAt(i, Event::GetEventWithFlags(cmd.command, cmd.flags, cmd.type));
		}

		// fix program string/event pointers
		for (i = archivedPointerFixup.NumObjects(); i > 0; i--)
		{
			pfixup_t *fixup = archivedPointerFixup.ObjectAt(i);

			if (fixup->isString)
			{
				*fixup->ptr = archivedStrings.ObjectAt(*fixup->ptr);
			}
			else
			{
				*fixup->ptr = archivedEvents.ObjectAt(*fixup->ptr);
			}

			delete fixup;
		}

		successCompile = true;
	}

	// cleanup
	archivedStrings.FreeObjectList();
	archivedEvents.FreeObjectList();
	archivedPointerFixup.FreeObjectList();

	if (!arc.Loading())
	{
		if (m_ProgToSource)
		{
			count = m_ProgToSource->size();
			arc.ArchiveInteger(&count);

			m_ProgToSource->Archive(arc);
		}
		else
		{
			arc.ArchiveInteger(&count);
		}
	}
	else
	{
		arc.ArchiveInteger(&count);

		if (count)
		{
			m_ProgToSource = new con_set < unsigned char *, sourceinfo_t >;
			m_ProgToSource->Archive(arc);
		}
	}

	arc.ArchiveUnsigned(&requiredStackSize);
	arc.ArchiveBool(&m_bPrecompiled);

	if (!m_bPrecompiled && arc.Loading())
	{
		fileHandle_t filehandle = NULL;

		m_SourceLength = glbs.FS_ReadFile(Filename().c_str(), (void **)&m_SourceBuffer, true);

		if (m_SourceLength > 0)
		{
			m_SourceBuffer = (char *)glbs.Malloc(m_SourceLength);

			glbs.FS_Read(m_SourceBuffer, m_SourceLength, filehandle);
			glbs.FS_FCloseFile(filehandle);
		}
	}

	m_State->Archive(arc);

	current_progBuffer = NULL;
	*/
}

void GameScript::Archive(Archiver& arc, GameScript *& scr)
{
	/*
	str filename;

	if (!arc.Saving())
	{
		arc.ArchiveString(&filename);

		if (filename != "")
		{
			scr = GetScriptManager()->GetScript(filename);
		}
		else
		{
			scr = NULL;
		}
	}
	else
	{
		filename = scr->Filename();

		arc.ArchiveString(&filename);
	}
	*/
}

void GameScript::ArchiveCodePos(Archiver& arc, unsigned char **codePos)
{
	/*
	int pos = 0;
	str filename;

	if (!arc.Saving())
	{
		pos = *codePos - m_ProgBuffer;
		if (pos >= 0 && pos < m_ProgLength)
		{
			filename = Filename();
		}
	}

	arc.ArchiveInteger(&pos);
	arc.ArchiveString(&filename);

	if (arc.Loading())
	{
		if (Filename() == filename)
		{
			*codePos = m_ProgBuffer + pos;
		}
	}
	*/
}

void GameScript::Close(void)
{
	for (intptr_t i = m_CatchBlocks.NumObjects(); i > 0; i--)
	{
		delete m_CatchBlocks.ObjectAt(i);
	}

	m_CatchBlocks.FreeObjectList();

	if (m_ProgToSource)
	{
		delete m_ProgToSource;
		m_ProgToSource = NULL;
	}

	if (m_ProgBuffer)
	{
		free(m_ProgBuffer);
		m_ProgBuffer = NULL;
	}

	if (m_SourceBuffer)
	{
		free(m_SourceBuffer);
		m_SourceBuffer = NULL;
	}

	m_ProgLength = 0;
	m_SourceLength = 0;
	m_bPrecompiled = false;
}

void GameScript::Load(const void *sourceBuffer, size_t sourceLength)
{
	size_t nodeLength;
	char *m_PreprocessedBuffer;

	m_SourceBuffer = (char *)malloc(sourceLength + 1);
	m_SourceLength = sourceLength;

	m_SourceBuffer[sourceLength] = 0;

	memcpy(m_SourceBuffer, sourceBuffer, sourceLength);

	ScriptCompiler* Compiler = new ScriptCompiler();
	Compiler->Reset();

	m_PreprocessedBuffer = Compiler->Preprocess(m_SourceBuffer);
	nodeLength = Compiler->Parse(this, m_PreprocessedBuffer);
	Compiler->Preclean(m_PreprocessedBuffer);

	if (!nodeLength)
	{
		printf("^~^~^ Script file compile error:  Couldn't parse '%s'\n", Filename().c_str());
		return Close();
	}

	m_ProgBuffer = (unsigned char *)malloc(nodeLength);
	m_ProgLength = Compiler->Compile(this, m_ProgBuffer);

	if (!m_ProgLength)
	{
		printf("^~^~^ Script file compile error:  Couldn't compile '%s'\n", Filename().c_str());
		return Close();
	}

	requiredStackSize = Compiler->m_iInternalMaxVarStackOffset + 9 * Compiler->m_iMaxExternalVarStackOffset + 1;

	successCompile = true;
}

bool GameScript::GetCodePos(unsigned char *codePos, str& filename, uintptr_t& pos)
{
	pos = codePos - m_ProgBuffer;

	if (pos >= 0 && pos < m_ProgLength)
	{
		filename = Filename();
		return true;
	}
	else
	{
		return false;
	}
}

bool GameScript::SetCodePos(unsigned char *&codePos, str& filename, uintptr_t pos)
{
	if (Filename() == filename)
	{
		codePos = m_ProgBuffer + pos;
		return true;
	}
	else
	{
		return false;
	}
}

unsigned int GameScript::GetRequiredStackSize(void)
{
	return requiredStackSize;
}

bool GameScript::labelExists(const char *name)
{
	str labelname;

	// if we got passed a NULL than that means just run the script so of course it exists
	if (!name)
	{
		return true;
	}

	if (m_State->FindLabel(name))
	{
		return true;
	}

	return false;
}

StateScript *GameScript::CreateCatchStateScript(unsigned char *try_begin_code_pos, unsigned char *try_end_code_pos)
{
	CatchBlock *catchBlock = new CatchBlock;

	catchBlock->m_TryStartCodePos = try_begin_code_pos;
	catchBlock->m_TryEndCodePos = try_end_code_pos;

	m_CatchBlocks.AddObject(catchBlock);

	return &catchBlock->m_StateScript;
}

StateScript *GameScript::CreateSwitchStateScript(void)
{
	return new StateScript();
}

StateScript *GameScript::GetCatchStateScript(unsigned char *in, unsigned char *&out)
{
	CatchBlock *catchBlock;
	CatchBlock *bestCatchBlock = NULL;

	for (intptr_t i = m_CatchBlocks.NumObjects(); i > 0; i--)
	{
		catchBlock = m_CatchBlocks.ObjectAt(i);

		if (in >= catchBlock->m_TryStartCodePos && in < catchBlock->m_TryEndCodePos)
		{
			if (!bestCatchBlock || catchBlock->m_TryEndCodePos < bestCatchBlock->m_TryEndCodePos)
			{
				bestCatchBlock = catchBlock;
			}
		}
	}

	if (bestCatchBlock)
	{
		out = bestCatchBlock->m_TryEndCodePos;

		return &bestCatchBlock->m_StateScript;
	}
	else
	{
		return NULL;
	}
}
