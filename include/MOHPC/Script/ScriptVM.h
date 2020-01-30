#pragma once

#include "ScriptClass.h"
#include "Event.h"
#include "ScriptVariable.h"

namespace MOHPC
{
#define	MAX_STACK_DEPTH		20		// 9 in mohaa
	//#define	LOCALSTACK_SIZE		255		// pre-allocated localstack size for each VM
#define MAX_SCRIPTCYCLES	9999	// max cmds

#define STATE_RUNNING					0		// Running
#define STATE_SUSPENDED					1		// Suspended
#define STATE_WAITING					2		// Waiting for something
#define STATE_EXECUTION					3		// Resume to execution
#define STATE_DESTROYED					4		// Pending deletion

#define THREAD_RUNNING					0		// Running
#define THREAD_WAITING					1		// Waiting
#define THREAD_SUSPENDED				2		// Suspended
#define THREAD_CONTEXT_SWITCH			3		// Resume from context switch

	class ScriptContainer;
	class ScriptException;
	class StateScript;
	class ScriptThread;

	class ScriptCallStack
	{
	public:
		// opcode variable
		unsigned char* codePos;	// opcode will be restored once a DONE was hit

		// stack variables
		ScriptVariable* localStack;
		ScriptVariable* pTop;

		// return variable
		ScriptVariable returnValue;

		// OLD self value
		SafePtr<Listener> m_Self;
	};

	class ScriptStack
	{
	public:
		ScriptVariable * m_Array;
		int m_Count;
	};

	class ScriptVM : public BaseScriptClass
	{
		friend class ScriptThread;

	public:
		// important thread variables
		ScriptVM * next;				// next VM in the current ScriptContainer

		ScriptThread* m_Thread;			// script thread
		ScriptContainer* m_ScriptContainer;		// current group of threads

	public:
		// return variables
		ScriptStack* m_Stack;		// Currently unused
		ScriptVariable m_ReturnValue;	// VM return value

		// opcode variables
		unsigned char* m_PrevCodePos;		// previous opcode, for use with script exceptions
		unsigned char* m_CodePos;			// check compiler.h for the list of all opcodes

	public:
		// states
		unsigned char state;			// current VM state
		unsigned char m_ThreadState;	// current thread state

		// stack variables
		Container< ScriptCallStack * > callStack;			// thread's call stack
		ScriptVariable *localStack;		// thread's local stack
		int localStackSize;		// dynamically allocated at initialization
		ScriptVariable *pTop;				// top stack from the local stack
		ScriptVariable *m_StackPos;		// marked stack position

		// parameters variables
		ScriptVariable *m_pOldData;		// old fastEvent data, to cleanup
		int m_OldDataSize;
		bool m_bMarkStack;		// changed by OP_MARK_STACK_POS and OP_RESTORE_STACK_POS
		Event* fastEvent;			// parameter list, set when the VM is executed

		// miscellaneous
		bool m_bAllowContextSwitch;			// allow parallel VM executions [experimental feature]

	private:
		void error(const char *format, ...);

		void executeCommand(Listener *listener, int iParamCount, int eventnum, bool bMethod = false, bool bReturn = false);
		bool executeGetter(Listener *listener, const_str name);
		bool executeSetter(Listener *listener, const_str name);

		void jump(int offset);
		void jumpBool(int offset, bool value);

		void loadTop(Listener *listener, bool noTop = false);
		void storeTop(Listener *listener, bool noTop = false);

		void SetFastData(ScriptVariable *data, int dataSize);

		bool Switch(StateScript *stateScript, ScriptVariable &var);

		unsigned char *ProgBuffer();
		void HandleScriptException(ScriptException& exc);

	public:
		ScriptVM(ScriptContainer *scriptContainer, unsigned char *pCodePos, ScriptThread *thread);
		~ScriptVM();

		void Archive(Archiver& arc);

		void EnterFunction(Event *ev);
		void LeaveFunction();

		void End(const ScriptVariable& returnValue);
		void End();

		void Execute(ScriptVariable *data = NULL, int dataSize = 0, str label = "");
		void NotifyDelete();
		void Resume(bool bForce = false);
		void Suspend();

		const str& Filename();
		const str& Label();
		ScriptContainer* GetScriptContainer();

		bool IsSuspended();
		int State();
		int ThreadState();

		void EventGoto(Event *ev);
		bool EventThrow(Event *ev);

		void AllowContextSwitch(bool allow = true);
		void RequestContextSwitch();
	};
};
