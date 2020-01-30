#pragma once

#include "AbstractScript.h"
#include "StateScript.h"

namespace MOHPC
{
	class Listener;
	class ScriptThread;
	class GameScript;

	class CatchBlock
	{
	public:
		// program variable
		StateScript m_StateScript;

		// code position variables
		unsigned char *m_TryStartCodePos;
		unsigned char *m_TryEndCodePos;
	};

	class GameScript : public AbstractScript
	{
	protected:
		// try/throw variable
		Container<CatchBlock *> m_CatchBlocks;

	public:
		// program variables
		StateScript* m_State;
		unsigned char* m_ProgBuffer;
		size_t m_ProgLength;

		// compile variables
		bool successCompile;
		bool m_bPrecompiled;

		// stack variables
		unsigned int requiredStackSize;

	public:

		GameScript();
		GameScript(const char *filename);
		~GameScript();

		virtual void Archive(Archiver& arc);
		static void Archive(Archiver& arc, GameScript *&scr);
		void ArchiveCodePos(Archiver& arc, unsigned char **codePos);

		void Close();
		void Load(const void *sourceBuffer, size_t sourceLength);


		bool GetCodePos(unsigned char *codePos, str& filename, uintptr_t& pos);
		bool SetCodePos(unsigned char *&codePos, str& filename, uintptr_t pos);

		unsigned int GetRequiredStackSize(void);

		bool labelExists(const char *name);

		StateScript* CreateCatchStateScript(unsigned char *try_begin_code_pos, unsigned char *try_end_code_pos);
		StateScript* CreateSwitchStateScript(void);

		StateScript* GetCatchStateScript(unsigned char *in, unsigned char *&out);
	};
};
