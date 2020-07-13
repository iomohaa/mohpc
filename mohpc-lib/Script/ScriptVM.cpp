#include <Shared.h>
#include <MOHPC/Script/ScriptVM.h>
#include <MOHPC/Script/World.h>
#include <MOHPC/Script/Level.h>
#include <MOHPC/Script/Game.h>
#include <MOHPC/Script/Parm.h>
#include <MOHPC/Script/Listener.h>
#include <MOHPC/Script/World.h>
#include <MOHPC/Script/ScriptContainer.h>
#include <MOHPC/Script/ScriptThread.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/ScriptVariable.h>
#include <MOHPC/Script/GameScript.h>
#include <MOHPC/Script/short3.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EventManager.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>
#include <MOHPC/Script/ScriptOpcodes.h>
#include <stdarg.h>

using namespace MOHPC;

ScriptVM::ScriptVM(ScriptContainer *scriptClass, unsigned char *pCodePos, ScriptThread *thread)
{
	next = NULL;

	m_Thread = thread;
	m_ScriptContainer = scriptClass;

	m_Stack = NULL;

	m_PrevCodePos = NULL;
	m_CodePos = pCodePos;

	state = STATE_RUNNING;
	m_ThreadState = THREAD_RUNNING;

	m_pOldData = NULL;
	m_OldDataSize = 0;

	m_bMarkStack = false;
	m_StackPos = NULL;

	m_bAllowContextSwitch = true;

	localStackSize = m_ScriptContainer->GetScript()->GetRequiredStackSize();

	if (localStackSize <= 0)
	{
		localStackSize = 1;
	}

	AssetManagerPtr AM = GetAssetManager();

	localStack = new ScriptVariable[localStackSize];

	pTop = localStack;
	fastEvent = new Event;

	m_ScriptContainer->AddThread(this);
}

/*
====================
~ScriptVM
====================
*/
ScriptVM::~ScriptVM()
{
	fastEvent->data = m_pOldData;
	fastEvent->dataSize = m_OldDataSize;

	// clean-up the call stack
	while (callStack.NumObjects())
	{
		LeaveFunction();
	}

	AssetManagerPtr AM = GetAssetManager();
	delete[] localStack;
	delete fastEvent;
}

/*
====================
Archive
====================
*/
void ScriptVM::Archive(Archiver& arc)
{
	/*
	int stack = 0;

	if (arc.Saving())
	{
		if (m_Stack)
			stack = m_Stack->m_Count;

		arc.ArchiveInteger(&stack);
	}
	else
	{
		arc.ArchiveInteger(&stack);

		if (stack)
		{
			m_Stack = new ScriptStack;
			m_Stack->m_Array = new ScriptVariable[stack];
			m_Stack->m_Count = stack;
		}
	}

	for (int i = 1; i <= stack; i++)
	{
		m_Stack->m_Array[i].ArchiveInternal(arc);
	}

	m_ReturnValue.ArchiveInternal(arc);
	m_ScriptContainer->ArchiveCodePos(arc, &m_PrevCodePos);
	m_ScriptContainer->ArchiveCodePos(arc, &m_CodePos);
	arc.ArchiveByte(&state);
	arc.ArchiveByte(&m_ThreadState);
	*/
}

/*
====================
error

Triggers an error
====================
*/
void ScriptVM::error(const char *format, ...)
{
	char buffer[4000];
	va_list va;

	va_start(va, format);
	vsprintf(buffer, format, va);
	va_end(va);

	//glbs.Printf("----------------------------------------------------------\n%s\n", buffer);
	m_ReturnValue.setStringValue("$.INTERRUPTED");
}

/*
====================
executeCommand
====================
*/
void ScriptVM::executeCommand(Listener *listener, uint16_t iParamCount, int eventnum, bool bMethod, bool bReturn)
{
	ScriptVariable *var;

	Event ev(eventnum);
	ev.InitAssetManager(this);

	if (bReturn)
	{
		var = pTop;
	}
	else
	{
		var = pTop + 1;
	}

	AssetManagerPtr AM = GetAssetManager();

	ev.dataSize = iParamCount;
	if (iParamCount)
	{
		ev.data = new ScriptVariable[ev.dataSize];
	}

	ev.fromScript = true;

	if (!bMethod)
	{
		for (uint16_t i = 0; i < iParamCount; i++)
		{
			ev.data[i] = var[i];
		}
	}
	else
	{
		for (uint16_t i = 0; i < iParamCount; i++)
		{
			ev.data[i] = var[i + 1];
		}
	}

	listener->ProcessScriptEvent(ev);

	if (ev.NumArgs() > iParamCount)
	{
		*pTop = ev.GetValue(ev.NumArgs());
	}
	else
	{
		pTop->Clear();
	}
}

/*
====================
executeGetter
====================
*/
bool ScriptVM::executeGetter(Listener *listener, const_str name)
{
	EventManagerPtr eventManager = GetEventManager();
	uintptr_t eventnum = eventManager->FindGetterEventNum(name);

	if (eventnum && listener->classinfo()->GetDef(eventnum))
	{
		Event ev(eventnum);
		ev.InitAssetManager(this);
		ev.fromScript = true;

		if (listener->ProcessScriptEvent(ev))
		{

			if (ev.NumArgs() > 0)
			{
				*pTop = ev.GetValue(ev.NumArgs());
			}
			else
			{
				pTop->Clear();
			}

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		eventnum = eventManager->FindSetterEventNum(name);
		assert(!eventnum || !listener->classinfo()->GetDef(eventnum));
		if (eventnum && listener->classinfo()->GetDef(eventnum))
		{
			ScriptError("Cannot get a write-only variable");
		}
	}

	return false;
}

/*
====================
executeSetter
====================
*/
bool ScriptVM::executeSetter(Listener *listener, const_str name)
{
	EventManagerPtr eventManager = GetEventManager();
	uintptr_t eventnum = eventManager->FindSetterEventNum(name);

	if (eventnum && listener->classinfo()->GetDef(eventnum))
	{
		Event ev(eventnum);
		ev.InitAssetManager(this);
		ev.fromScript = true;

		ev.AddValue(*pTop);

		if (listener->ProcessScriptEvent(ev))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		eventnum = eventManager->FindGetterEventNum(name);
		if (eventnum && listener->classinfo()->GetDef(eventnum))
		{
			ScriptError("Cannot set a read-only variable");
		}
	}

	return false;
}

/*
====================
jump
====================
*/
void ScriptVM::jump(int offset)
{
	m_CodePos += offset;
}

/*
====================
jumpBool
====================
*/
void ScriptVM::jumpBool(int offset, bool booleanValue)
{
	if (booleanValue)
	{
		jump(offset);
	}
	else
	{
		m_CodePos += sizeof(unsigned int);
	}
}

/*
====================
loadTop
====================
*/
void ScriptVM::loadTop(Listener *listener, bool noTop)
{
	const_str variable = *reinterpret_cast<int *>(m_CodePos);
	m_CodePos += sizeof(unsigned int);

	if (variable != -1)
	{
		ScriptManagerPtr Director = GetScriptManager();

		if (!executeSetter(listener, variable))
		{
			listener->Vars()->SetVariable(variable, *pTop);
		}
	}

	if (!noTop)
	{
		pTop--;
	}
}

/*
====================
storeTop
====================
*/
void ScriptVM::storeTop(Listener *listener, bool noTop)
{
	const_str variable = *reinterpret_cast<int *>(m_CodePos);
	m_CodePos += sizeof(unsigned int);

	if (!noTop)
	{
		pTop++;
	}

	if (variable != -1 && !executeGetter(listener, variable))
	{
		*pTop = *listener->Vars()->GetOrCreateVariable(variable);
	}
}

unsigned char *ScriptVM::ProgBuffer()
{
	return m_CodePos;
}

void ScriptVM::EnterFunction(Event *ev)
{
	ScriptCallStack *stack;
	str label = ev->GetString(1);

	SetFastData(ev->data + 1, ev->dataSize - 1);

	unsigned char *codePos = m_ScriptContainer->FindLabel(label);

	if (!codePos)
	{
		ScriptError("ScriptVM::EnterFunction: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str());
	}

	stack = new ScriptCallStack;

	stack->codePos = m_CodePos;

	stack->pTop = pTop;
	stack->returnValue = m_ReturnValue;
	stack->localStack = localStack;
	stack->m_Self = m_ScriptContainer->GetSelf();

	callStack.AddObject(stack);

	m_CodePos = codePos;

	localStack = new ScriptVariable[localStackSize];

	pTop = localStack;
	m_ReturnValue.Clear();
}

void ScriptVM::LeaveFunction()
{
	size_t num = callStack.NumObjects();

	AssetManagerPtr AM = GetAssetManager();

	if (num)
	{
		ScriptCallStack *stack = callStack.ObjectAt(num);

		pTop = stack->pTop;
		*pTop = m_ReturnValue;

		m_CodePos = stack->codePos;
		m_ReturnValue = stack->returnValue;
		m_ScriptContainer->m_Self = stack->m_Self;

		delete localStack;

		localStack = stack->localStack;

		delete stack;

		callStack.RemoveObjectAt(num);
	}
	else
	{
		delete m_Thread;
	}
}

void ScriptVM::End(const ScriptVariable& returnValue)
{
	m_ReturnValue.setPointer(returnValue);

	LeaveFunction();
}

void ScriptVM::End()
{
	m_ReturnValue.ClearPointer();

	LeaveFunction();
}

void ScriptVM::Execute(ScriptVariable *data, int dataSize, str label)
{
	unsigned char *opcode;
	bool doneProcessing = false;
	bool deleteThread = false;
	bool eventCalled = false;

	ScriptVariable *a;
	ScriptVariable *b;
	ScriptVariable *c;

	int index, iParamCount;

	Listener *listener;

	Event ev;
	Event *ev2;
	ScriptVariable *var = NULL;

	ev.InitAssetManager(this);

	static str str_null = "";
	str& value = str_null;

	ConSimple *targetList;

	if (label != "")
	{
		// Throw if label is not found
		if (!(m_CodePos = m_ScriptContainer->FindLabel(label)))
		{
			ScriptError("ScriptVM::Execute: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str());
		}
	}

	ScriptManagerPtr Director = GetScriptManager();

	if (Director->GetStackCount() >= MAX_STACK_DEPTH)
	{
		state = STATE_EXECUTION;

		ScriptException::next_abort = -1;
		ScriptException exc("stack overflow");

		throw exc;
	}

	Director->AddStack();

	if (dataSize)
	{
		SetFastData(data, dataSize);
	}

	state = STATE_RUNNING;

	GameManagerPtr gameManager = GetGameManager();
	EventManagerPtr eventManager = GetEventManager();
	AssetManagerPtr AM = GetAssetManager();

	Level* level = gameManager->GetLevel();
	float cmdTime = level->GetTimeSeconds();
	int32_t cmdCount = 0;

	while (!doneProcessing && state == STATE_RUNNING)
	{
		m_PrevCodePos = m_CodePos;

		//Director->cmdCount++;

		try
		{
			if (!Director->IsContextSwitchAllowed())
			{
				//if (Director->cmdCount > 9999 && glbs.Milliseconds() - Director->cmdTime > Director->maxTime)
				if(level->GetTimeSeconds() - cmdTime > 5000)
				{
					if (Director->HasLoopProtection())
					{
						cmdTime = level->GetTimeSeconds();

						deleteThread = true;
						state = STATE_EXECUTION;

						if (Director->HasLoopDrop())
						{
							ScriptException::next_abort = -1;
						}

						ScriptError("Command overflow. Possible infinite loop in thread.\n");
					}

					//VM_DPrintf("Update of script position - This is not an error.\n");
					//VM_DPrintf("=================================================\n");
					m_ScriptContainer->GetScript()->PrintSourcePos(m_CodePos, true);
					//VM_DPrintf("=================================================\n");

					//Director->cmdCount = 0;
				}
			}
			else
			{
				//if (level->GetTimeSeconds() - cmdTime > Director->maxTime)
				{
					// Request a context switch
					RequestContextSwitch();
				}
			}

			if (!m_bMarkStack)
			{
				assert(pTop >= localStack && pTop < localStack + localStackSize);
				if (pTop < localStack)
				{
					deleteThread = true;
					state = STATE_EXECUTION;

					error("VM stack error. Negative stack value %d.\n", pTop - localStack);
					break;
				}
				else if (pTop >= localStack + localStackSize)
				{
					deleteThread = true;
					state = STATE_EXECUTION;

					error("VM stack error. Exceeded the maximum stack size %d.\n", localStackSize);
					break;
				}
			}

			index = 0;
			eventCalled = false;

			opcode = m_CodePos++;
			switch (*opcode)
			{
			case OP_BIN_BITWISE_AND:
				a = pTop--;
				b = pTop;

				*b &= *a;
				break;

			case OP_BIN_BITWISE_OR:
				a = pTop--;
				b = pTop;

				*b |= *a;
				break;

			case OP_BIN_BITWISE_EXCL_OR:
				a = pTop--;
				b = pTop;

				*b ^= *a;
				break;

			case OP_BIN_EQUALITY:
				a = pTop--;
				b = pTop;

				b->setIntValue(*b == *a);
				break;

			case OP_BIN_INEQUALITY:
				a = pTop--;
				b = pTop;

				b->setIntValue(*b != *a);
				break;

			case OP_BIN_GREATER_THAN:
				a = pTop--;
				b = pTop;

				b->greaterthan(*a);
				break;

			case OP_BIN_GREATER_THAN_OR_EQUAL:
				a = pTop--;
				b = pTop;

				b->greaterthanorequal(*a);
				break;

			case OP_BIN_LESS_THAN:
				a = pTop--;
				b = pTop;

				b->lessthan(*a);
				break;

			case OP_BIN_LESS_THAN_OR_EQUAL:
				a = pTop--;
				b = pTop;

				b->lessthanorequal(*a);
				break;

			case OP_BIN_PLUS:
				a = pTop--;
				b = pTop;

				*b += *a;
				break;

			case OP_BIN_MINUS:
				a = pTop--;
				b = pTop;

				*b -= *a;
				break;

			case OP_BIN_MULTIPLY:
				a = pTop--;
				b = pTop;

				*b *= *a;
				break;

			case OP_BIN_DIVIDE:
				a = pTop--;
				b = pTop;

				*b /= *a;
				break;

			case OP_BIN_PERCENTAGE:
				a = pTop--;
				b = pTop;

				*b %= *a;
				break;

			case OP_BIN_SHIFT_LEFT:
				a = pTop--;
				b = pTop;

				*b <<= *a;
				break;

			case OP_BIN_SHIFT_RIGHT:
				a = pTop--;
				b = pTop;

				*b >>= *a;
				break;

			case OP_BOOL_JUMP_FALSE4:
				jumpBool(*reinterpret_cast<unsigned int *>(m_CodePos) + sizeof(unsigned int), !pTop->m_data.intValue);

				pTop--;

				break;

			case OP_BOOL_JUMP_TRUE4:
				jumpBool(*reinterpret_cast<unsigned int *>(m_CodePos) + sizeof(unsigned int), pTop->m_data.intValue ? true : false);

				pTop--;

				break;

			case OP_VAR_JUMP_FALSE4:
				jumpBool(*reinterpret_cast<unsigned int *>(m_CodePos) + sizeof(unsigned int), !pTop->booleanValue());

				pTop--;

				break;

			case OP_VAR_JUMP_TRUE4:
				jumpBool(*reinterpret_cast<unsigned int *>(m_CodePos) + sizeof(unsigned int), pTop->booleanValue());

				pTop--;

				break;

			case OP_BOOL_LOGICAL_AND:
				if (pTop->m_data.intValue)
				{
					pTop--;
					m_CodePos += sizeof(unsigned int);
				}
				else
				{
					m_CodePos += *reinterpret_cast<unsigned int *>(m_CodePos) + sizeof(unsigned int);
				}

				break;

			case OP_BOOL_LOGICAL_OR:
				if (!pTop->m_data.intValue)
				{
					pTop--;
					m_CodePos += sizeof(unsigned int);
				}
				else
				{
					m_CodePos += *reinterpret_cast<unsigned int *>(m_CodePos) + sizeof(unsigned int);
				}

				break;

			case OP_VAR_LOGICAL_AND:
				if (pTop->booleanValue())
				{
					pTop--;
					m_CodePos += sizeof(unsigned int);
				}
				else
				{
					pTop->SetFalse();
					m_CodePos += *reinterpret_cast<unsigned int *>(m_CodePos) + sizeof(unsigned int);
				}
				break;

			case OP_VAR_LOGICAL_OR:
				if (!pTop->booleanValue())
				{
					pTop--;
					m_CodePos += sizeof(unsigned int);
				}
				else
				{
					pTop->SetTrue();
					m_CodePos += *reinterpret_cast<unsigned int *>(m_CodePos) + sizeof(unsigned int);
				}
				break;

			case OP_BOOL_STORE_FALSE:
				pTop++;
				pTop->SetFalse();
				break;

			case OP_BOOL_STORE_TRUE:
				pTop++;
				pTop->SetTrue();
				break;

			case OP_BOOL_UN_NOT:
				pTop->m_data.intValue = (pTop->m_data.intValue == 0);
				break;

			case OP_CALC_VECTOR:
				c = pTop--;
				b = pTop--;
				a = pTop;

				pTop->setVectorValue(Vector(a->floatValue(), b->floatValue(), c->floatValue()));
				break;

			case OP_EXEC_CMD0:
				iParamCount = 0;
				goto __execCmd;

			case OP_EXEC_CMD1:
				iParamCount = 1;
				goto __execCmd;

			case OP_EXEC_CMD2:
				iParamCount = 2;
				goto __execCmd;

			case OP_EXEC_CMD3:
				iParamCount = 3;
				goto __execCmd;

			case OP_EXEC_CMD4:
				iParamCount = 4;
				goto __execCmd;

			case OP_EXEC_CMD5:
				iParamCount = 5;
				goto __execCmd;

			case OP_EXEC_CMD_COUNT1:
				iParamCount = *m_CodePos++;

			__execCmd:
				index = *reinterpret_cast<unsigned int *>(m_CodePos);

				m_CodePos += sizeof(unsigned int);

				pTop -= iParamCount;

				try
				{
					executeCommand(m_Thread, iParamCount, index);
				}
				catch (ScriptException& exc)
				{
					throw exc;
				}

				break;

			case OP_EXEC_CMD_METHOD0:
				iParamCount = 0;
				goto __execCmdMethod;

			case OP_EXEC_CMD_METHOD1:
				iParamCount = 1;
				goto __execCmdMethod;

			case OP_EXEC_CMD_METHOD2:
				iParamCount = 2;
				goto __execCmdMethod;

			case OP_EXEC_CMD_METHOD3:
				iParamCount = 3;
				goto __execCmdMethod;

			case OP_EXEC_CMD_METHOD4:
				iParamCount = 4;
				goto __execCmdMethod;

			case OP_EXEC_CMD_METHOD5:
				iParamCount = 5;
				goto __execCmdMethod;

			__execCmdMethod:
				m_CodePos--;
				goto __execCmdMethodInternal;

			case OP_EXEC_CMD_METHOD_COUNT1:
				iParamCount = *m_CodePos;

			__execCmdMethodInternal:
				try
				{
					index = *reinterpret_cast<unsigned int *>(m_CodePos + sizeof(uint8_t));

					pTop -= iParamCount;
					a = pTop--;

					if (a->arraysize() < 0)
					{
						ScriptError("command '%s' applied to NIL", eventManager->GetEventName(index));
					}

					ScriptVariable array = *a;
					Listener *listener;

					array.CastConstArrayValue();

					for (intptr_t i = array.arraysize(); i > 0; i--)
					{
						if (!(listener = array[i]->listenerValue()))
						{
							ScriptError("command '%s' applied to NULL listener", eventManager->GetEventName(index));
						}

						executeCommand(listener, iParamCount, index, true);
					}
				}
				catch (ScriptException& exc)
				{
					m_CodePos += sizeof(uint8_t) + sizeof(unsigned int);

					throw exc;
				}

				m_CodePos += sizeof(uint8_t) + sizeof(unsigned int);

				break;

			case OP_EXEC_METHOD0:
				iParamCount = 0;
				goto __execMethod;

			case OP_EXEC_METHOD1:
				iParamCount = 1;
				goto __execMethod;

			case OP_EXEC_METHOD2:
				iParamCount = 2;
				goto __execMethod;

			case OP_EXEC_METHOD3:
				iParamCount = 3;
				goto __execMethod;

			case OP_EXEC_METHOD4:
				iParamCount = 4;
				goto __execMethod;

			case OP_EXEC_METHOD5:
				iParamCount = 5;

			__execMethod:
				m_CodePos--;
				goto __execMethodInternal;

			case OP_EXEC_METHOD_COUNT1:
				iParamCount = *m_CodePos;

			__execMethodInternal:
				try
				{
					index = *reinterpret_cast<unsigned int *>(m_CodePos + sizeof(uint8_t));

					pTop -= iParamCount;
					a = pTop--;
					pTop++; // push the return value

					Listener *listener = a->listenerValue();

					if (!listener)
					{
						ScriptError("command '%s' applied to NULL listener", eventManager->GetEventName(index));
					}

					executeCommand(listener, iParamCount, index, true, true);
				}
				catch (ScriptException& exc)
				{
					m_CodePos += sizeof(uint8_t) + sizeof(unsigned int);

					throw exc;
				}

				m_CodePos += sizeof(uint8_t) + sizeof(unsigned int);

				break;

			case OP_FUNC:
				ev.Clear();

				if (!*m_CodePos++)
				{
					const str& label = Director->GetString(*reinterpret_cast<unsigned int *>(m_CodePos));

					m_CodePos += sizeof(unsigned int);

					try
					{
						listener = pTop->listenerValue();

						if (!listener)
						{
							ScriptError("function '%s' applied to NULL listener", label.c_str());
						}
					}
					catch (ScriptException& exc)
					{
						pTop -= *m_CodePos++;

						throw exc;
					}

					pTop--;

					ev.AddString(label);

					int params = *m_CodePos++;

					var = pTop;
					pTop -= params;

					for (int i = 0; i < params; var++, i++)
					{
						ev.AddValue(*var);
					}

					pTop++;
					EnterFunction(&ev);

					m_ScriptContainer->m_Self = listener;
				}
				else
				{
					str filename, label;

					filename = Director->GetString(*reinterpret_cast<unsigned int *>(m_CodePos));
					m_CodePos += sizeof(unsigned int);
					label = Director->GetString(*reinterpret_cast<unsigned int *>(m_CodePos));
					m_CodePos += sizeof(unsigned int);

					try
					{
						listener = pTop->listenerValue();

						if (!listener)
						{
							ScriptError("function '%s' in '%s' applied to NULL listener", label.c_str(), filename.c_str());
						}
					}
					catch (ScriptException& exc)
					{
						pTop -= *m_CodePos++;

						throw exc;
					}

					pTop--;

					ScriptVariable constarray;
					ScriptVariable *pVar = new ScriptVariable[2];

					pVar[0].setStringValue(filename);
					pVar[1].setStringValue(label);

					constarray.setConstArrayValue(pVar, 2);

					delete[] pVar;

					ev2 = new Event(EV_Listener_WaitCreateReturnThread);
					ev2->AddValue(constarray);

					int params = *m_CodePos++;

					var = pTop;
					pTop -= params;

					for (int i = 0; i < params; var++, i++) {
						ev2->AddValue(*var);
					}

					pTop++;
					*pTop = listener->ProcessEventReturn(ev2);
				}
				break;

			case OP_JUMP4:
				m_CodePos += *reinterpret_cast<int *>(m_CodePos) + sizeof(unsigned int);
				break;

			case OP_JUMP_BACK4:
				m_CodePos -= *reinterpret_cast<int *>(m_CodePos);
				break;

			case OP_LOAD_ARRAY_VAR:
				a = pTop--;
				b = pTop--;
				c = pTop--;

				b->setArrayAt(*a, *c);
				break;

			case OP_LOAD_FIELD_VAR:
				a = pTop--;

				try
				{
					listener = a->listenerValue();

					if (listener == NULL)
					{
						value = Director->GetString(*reinterpret_cast<int *>(m_CodePos));
						ScriptError("Field '%s' applied to NULL listener", value.c_str());
					}
					else
					{
						eventCalled = true;
						loadTop(listener);
					}
				}
				catch (ScriptException& exc)
				{
					pTop--;

					if (!eventCalled) {
						m_CodePos += sizeof(unsigned int);
					}

					throw exc;
				}

				break;

			case OP_LOAD_CONST_ARRAY1:
				index = *reinterpret_cast<short *>(m_CodePos);
				m_CodePos += sizeof(short);

				pTop -= index - 1;
				pTop->setConstArrayValue(pTop, index);
				break;

			case OP_LOAD_GAME_VAR:
				loadTop(gameManager->GetGame());
				break;

			case OP_LOAD_GROUP_VAR:
				loadTop(m_ScriptContainer);
				break;

			case OP_LOAD_LEVEL_VAR:
				loadTop(level);
				break;

			case OP_LOAD_LOCAL_VAR:
				loadTop(m_Thread);
				break;

			case OP_LOAD_OWNER_VAR:
				if (!m_ScriptContainer->m_Self)
				{
					pTop--;
					m_CodePos += sizeof(unsigned int);
					ScriptError("self is NULL");
				}

				if (!m_ScriptContainer->m_Self->GetScriptOwner())
				{
					pTop--;
					m_CodePos += sizeof(unsigned int);
					ScriptError("self.owner is NULL");
				}

				loadTop(m_ScriptContainer->m_Self->GetScriptOwner());
				break;

			case OP_LOAD_PARM_VAR:
				loadTop(Director->GetParm());
				break;

			case OP_LOAD_SELF_VAR:
				if (!m_ScriptContainer->m_Self)
				{
					pTop--;
					m_CodePos += sizeof(unsigned int);
					ScriptError("self is NULL");
				}

				loadTop(m_ScriptContainer->m_Self);
				break;

			case OP_LOAD_STORE_GAME_VAR:
				loadTop(gameManager->GetGame(), true);
				break;

			case OP_LOAD_STORE_GROUP_VAR:
				loadTop(m_ScriptContainer, true);
				break;

			case OP_LOAD_STORE_LEVEL_VAR:
				loadTop(level, true);
				break;

			case OP_LOAD_STORE_LOCAL_VAR:
				loadTop(m_Thread, true);
				break;

			case OP_LOAD_STORE_OWNER_VAR:
				if (!m_ScriptContainer->m_Self)
				{
					m_CodePos += sizeof(unsigned int);
					ScriptError("self is NULL");
				}

				if (!m_ScriptContainer->m_Self->GetScriptOwner())
				{
					m_CodePos += sizeof(unsigned int);
					ScriptError("self.owner is NULL");
				}

				loadTop(m_ScriptContainer->m_Self->GetScriptOwner(), true);
				break;

			case OP_LOAD_STORE_PARM_VAR:
				loadTop(Director->GetParm(), true);
				break;

			case OP_LOAD_STORE_SELF_VAR:
				if (!m_ScriptContainer->m_Self)
				{
					ScriptError("self is NULL");
				}

				loadTop(m_ScriptContainer->m_Self, true);
				break;

			case OP_MARK_STACK_POS:
				m_StackPos = pTop;
				m_bMarkStack = true;
				break;

			case OP_STORE_PARAM:
				if (fastEvent->dataSize)
				{
					pTop = fastEvent->data++;
					fastEvent->dataSize--;
				}
				else
				{
					pTop = m_StackPos + 1;
					pTop->Clear();
				}
				break;

			case OP_RESTORE_STACK_POS:
				pTop = m_StackPos;
				m_bMarkStack = false;
				break;

			case OP_STORE_ARRAY:
				pTop--;
				pTop->evalArrayAt(*(pTop + 1));
				break;

			case OP_STORE_ARRAY_REF:
				pTop--;
				pTop->setArrayRefValue(*(pTop + 1));
				break;

			case OP_STORE_FIELD_REF:
			case OP_STORE_FIELD:
				try
				{
					value = Director->GetString(*reinterpret_cast<int *>(m_CodePos));

					listener = pTop->listenerValue();

					if (listener == NULL)
					{
						ScriptError("Field '%s' applied to NULL listener", value.c_str());
					}
					else
					{
						eventCalled = true;
						storeTop(listener, true);
					}

					if (*opcode == OP_STORE_FIELD_REF)
					{
						pTop->setRefValue(listener->vars->GetOrCreateVariable(value));
					}
				}
				catch (ScriptException& exc)
				{
					if (*opcode == OP_STORE_FIELD_REF)
					{
						pTop->setRefValue(pTop);
					}

					if (!eventCalled) {
						m_CodePos += sizeof(unsigned int);
					}

					throw exc;
				}
				break;

			case OP_STORE_FLOAT:
				pTop++;
				pTop->setFloatValue(*reinterpret_cast<float *>(m_CodePos));

				m_CodePos += sizeof(float);

				break;

			case OP_STORE_INT0:
				pTop++;
				pTop->setIntValue(0);

				break;

			case OP_STORE_INT1:
				pTop++;
				pTop->setIntValue(*m_CodePos++);

				break;

			case OP_STORE_INT2:
				pTop++;
				pTop->setIntValue(*reinterpret_cast<short *>(m_CodePos));

				m_CodePos += sizeof(short);

				break;

			case OP_STORE_INT3:
				pTop++;
				pTop->setIntValue(*reinterpret_cast<short3 *>(m_CodePos));

				m_CodePos += sizeof(short3);
				break;

			case OP_STORE_INT4:
				pTop++;
				pTop->setIntValue(*reinterpret_cast<int *>(m_CodePos));

				m_CodePos += sizeof(int);

				break;

			case OP_STORE_GAME_VAR:
				storeTop(gameManager->GetGame());
				break;

			case OP_STORE_GROUP_VAR:
				storeTop(m_ScriptContainer);
				break;

			case OP_STORE_LEVEL_VAR:
				storeTop(level);
				break;

			case OP_STORE_LOCAL_VAR:
				storeTop(m_Thread);
				break;

			case OP_STORE_OWNER_VAR:
				if (!m_ScriptContainer->m_Self)
				{
					pTop++;
					m_CodePos += sizeof(unsigned int);
					ScriptError("self is NULL");
				}

				if (!m_ScriptContainer->m_Self->GetScriptOwner())
				{
					pTop++;
					m_CodePos += sizeof(unsigned int);
					ScriptError("self.owner is NULL");
				}

				storeTop(m_ScriptContainer->m_Self->GetScriptOwner());
				break;

			case OP_STORE_PARM_VAR:
				storeTop(Director->GetParm());
				break;

			case OP_STORE_SELF_VAR:
				if (!m_ScriptContainer->m_Self)
				{
					pTop++;
					m_CodePos += sizeof(unsigned int);
					ScriptError("self is NULL");
				}

				storeTop(m_ScriptContainer->m_Self);
				break;

			case OP_STORE_GAME:
				pTop++;
				pTop->setListenerValue(gameManager->GetGame());
				break;

			case OP_STORE_GROUP:
				pTop++;
				pTop->setListenerValue(m_ScriptContainer);
				break;

			case OP_STORE_LEVEL:
				pTop++;
				pTop->setListenerValue(level);
				break;

			case OP_STORE_LOCAL:
				pTop++;
				pTop->setListenerValue(m_Thread);
				break;

			case OP_STORE_OWNER:
				pTop++;

				if (!m_ScriptContainer->m_Self)
				{
					pTop++;
					ScriptError("self is NULL");
				}

				pTop->setListenerValue(m_ScriptContainer->m_Self->GetScriptOwner());
				break;

			case OP_STORE_PARM:
				pTop++;
				pTop->setListenerValue(Director->GetParm());
				break;

			case OP_STORE_SELF:
				pTop++;
				pTop->setListenerValue(m_ScriptContainer->m_Self);
				break;

			case OP_STORE_NIL:
				pTop++;
				pTop->Clear();
				break;

			case OP_STORE_NULL:
				pTop++;
				pTop->setListenerValue(NULL);
				break;

			case OP_STORE_STRING:
				pTop++;
				pTop->setConstStringValue(*reinterpret_cast<unsigned int *>(m_CodePos));

				m_CodePos += sizeof(unsigned int);

				break;

			case OP_STORE_VECTOR:
				pTop++;
				pTop->setVectorValue(*reinterpret_cast<Vector *>(m_CodePos));

				m_CodePos += sizeof(Vector);

				break;

			case OP_SWITCH:
				if (!Switch(*reinterpret_cast<StateScript **>(m_CodePos), *pTop))
				{
					m_CodePos += sizeof(unsigned int);
				}

				pTop--;
				break;

			case OP_UN_CAST_BOOLEAN:
				pTop->CastBoolean();
				break;

			case OP_UN_COMPLEMENT:
				pTop->complement();
				break;

			case OP_UN_MINUS:
				pTop->minus();
				break;

			case OP_UN_DEC:
				(*pTop)--;
				break;

			case OP_UN_INC:
				(*pTop)++;
				break;

			case OP_UN_SIZE:
				pTop->setIntValue((int)pTop->size());
				break;

			case OP_UN_TARGETNAME:
				targetList = gameManager->GetWorld()->GetExistingTargetList(pTop->constStringValue());

				if (!targetList)
				{
					pTop->setListenerValue(NULL);

					if (*m_CodePos >= OP_BIN_EQUALITY && *m_CodePos <= OP_BIN_GREATER_THAN_OR_EQUAL || *m_CodePos >= OP_BOOL_UN_NOT && *m_CodePos <= OP_UN_CAST_BOOLEAN) {
						ScriptError("Targetname '%s' does not exist.", pTop->stringValue().c_str());
					}

					break;
				}

				if (targetList->NumObjects() == 1)
				{
					pTop->setListenerValue(targetList->ObjectAt(1));
				}
				else if (targetList->NumObjects() > 1)
				{
					pTop->setContainerValue((Container< SafePtr< Listener > > *)targetList);
				}
				else
				{
					value = pTop->stringValue();

					pTop->setListenerValue(NULL);

					if (*m_CodePos >= OP_BIN_EQUALITY && *m_CodePos <= OP_BIN_GREATER_THAN_OR_EQUAL || *m_CodePos >= OP_BOOL_UN_NOT && *m_CodePos <= OP_UN_CAST_BOOLEAN) {
						ScriptError("Targetname '%s' does not exist.", value.c_str());
					}

					break;
				}

				break;

			case OP_VAR_UN_NOT:
				pTop->setIntValue(pTop->booleanValue());
				break;

			case OP_DONE:
				End();
				break;

			case OP_NOP:
				break;

			default:
				if (*opcode < OP_MAX)
				{
					//glbs.DPrintf("unknown opcode %d ('%s')\n", *opcode, OpcodeName(*opcode));
				}
				else
				{
					//glbs.DPrintf("unknown opcode %d\n", *opcode);
				}
				break;
			}
		}
		catch (ScriptException& exc)
		{
			HandleScriptException(exc);
		}
	}

	Director->RemoveStack();

	if (deleteThread || state == STATE_WAITING)
	{
		delete m_Thread;
	}
	else if (state == STATE_SUSPENDED)
	{
		state = STATE_EXECUTION;
	}

	if (state == STATE_DESTROYED)
	{
		delete this;
	}
}

void ScriptVM::HandleScriptException(ScriptException& exc)
{
	if (m_ScriptContainer)
	{
		m_ScriptContainer->GetScript()->PrintSourcePos(m_PrevCodePos, true);
	}
	else
	{
		//glbs.DPrintf("unknown source pos");
	}

	if (exc.bAbort)
	{
		ScriptException e(exc.string);

		e.bAbort = exc.bAbort;
		e.bIsForAnim = exc.bIsForAnim;

		state = STATE_EXECUTION;
		throw e;
	}

	printf("^~^~^ Script Error : %s\n\n", exc.string.c_str());
}

void ScriptVM::SetFastData(ScriptVariable *data, int dataSize)
{
	if (fastEvent->data)
	{
		fastEvent->data = m_pOldData;
		fastEvent->dataSize = m_OldDataSize;

		fastEvent->Clear();

		m_pOldData = NULL;
		m_OldDataSize = 0;
	}

	if (dataSize)
	{
		fastEvent->data = new ScriptVariable[dataSize];
		fastEvent->dataSize = dataSize;

		for (int i = 0; i < dataSize; i++)
		{
			fastEvent->data[i] = data[i];
		}

		m_pOldData = fastEvent->data;
		m_OldDataSize = fastEvent->dataSize;
	}
}

void ScriptVM::NotifyDelete()
{
	switch (state)
	{
	case STATE_DESTROYED:
		ScriptError("Attempting to delete a dead thread.");
		break;

	case STATE_RUNNING:
	case STATE_SUSPENDED:
	case STATE_WAITING:
		state = STATE_DESTROYED;

		if (m_ScriptContainer) {
			m_ScriptContainer->RemoveThread(this);
		}

		break;

	case STATE_EXECUTION:
		state = STATE_DESTROYED;

		if (m_ScriptContainer) {
			m_ScriptContainer->RemoveThread(this);
		}

		delete this;
		break;
	}
}

void ScriptVM::Resume(bool bForce)
{
	if (state == STATE_SUSPENDED || (bForce && state != STATE_DESTROYED))
	{
		state = STATE_RUNNING;
	}
}

void ScriptVM::Suspend()
{
	if (state == STATE_DESTROYED) {
		ScriptError("Cannot suspend a dead thread.");
	}
	else if (!state) {
		state = STATE_SUSPENDED;
	}
}

bool ScriptVM::Switch(StateScript *stateScript, ScriptVariable& var)
{
	unsigned char *pos;

	fastEvent->dataSize = 0;

	pos = stateScript->FindLabel(var.stringValue());

	if (!pos)
	{
		pos = stateScript->FindLabel("");

		if (!pos) {
			return false;
		}
	}

	m_CodePos = pos;

	return true;
}

const str& ScriptVM::Filename()
{
	return m_ScriptContainer->Filename();
}

const str& ScriptVM::Label()
{
	const_str label = m_ScriptContainer->NearestLabel(m_CodePos);
	ScriptManagerPtr Director = GetScriptManager();
	return Director->GetString(label);
}

ScriptContainer *ScriptVM::GetScriptContainer()
{
	return m_ScriptContainer;
}

bool ScriptVM::IsSuspended()
{
	return state == STATE_SUSPENDED;
}

int ScriptVM::State()
{
	return state;
}

int ScriptVM::ThreadState()
{
	return m_ThreadState;
}

void ScriptVM::EventGoto(Event *ev)
{
	str label = ev->GetString(1);

	SetFastData(ev->data + 1, ev->dataSize - 1);

	unsigned char *codePos = m_ScriptContainer->FindLabel(label);

	if (!codePos)
	{
		ScriptError("ScriptVM::EventGoto: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str());
	}

	m_CodePos = codePos;
}

bool ScriptVM::EventThrow(Event *ev)
{
	str label = ev->GetString(1);

	SetFastData(ev->data, ev->dataSize);

	fastEvent->eventnum = ev->eventnum;

	while (1)
	{
		StateScript *stateScript = m_ScriptContainer->GetCatchStateScript(m_PrevCodePos, m_PrevCodePos);

		if (!stateScript) {
			break;
		}

		m_CodePos = stateScript->FindLabel(label);

		if (m_CodePos)
		{
			fastEvent->data++;
			fastEvent->dataSize--;

			return true;
		}
	}

	return false;
}

void ScriptVM::AllowContextSwitch(bool allow)
{
	m_bAllowContextSwitch = allow;
}

void ScriptVM::RequestContextSwitch()
{
	ScriptManagerPtr Director = GetScriptManager();

	if (!m_bAllowContextSwitch || !Director->IsContextSwitchAllowed())
	{
		return;
	}

	//glbs.DPrintf( "Performing context switch\n" );

	Director->AddContextSwitch(m_Thread);

	m_ThreadState = THREAD_CONTEXT_SWITCH;
	Suspend();
}
