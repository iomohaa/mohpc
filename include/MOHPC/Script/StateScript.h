#pragma once

#include "ScriptClass.h"
#include "con_set.h"
#include "Container.h"

namespace MOHPC
{
	class GameScript;

	typedef struct {
		/** Pointer to the actual code position. */
		unsigned char* codepos;

		/** The label name the codepos is in. */
		const_str key;

		/** True if the label is in a private part. */
		bool isprivate;
	} script_label_t;

	class StateScript : public BaseScriptClass
	{
		friend class GameScript;

	private:
		/** The list of labels the StateScript contains. */
		con_set<const_str, script_label_t> label_list;
		Container<script_label_t *> reverse_label_list;

	public:
		/** The parent game script. */
		GameScript* m_Parent;

	public:
		StateScript();

		virtual void Archive(Archiver& arc);

		bool AddLabel(const str& label, unsigned char* pos, bool private_section = false);
		bool AddLabel(const_str label, unsigned char* pos, bool private_section = false);
		unsigned char* FindLabel(str label);
		unsigned char* FindLabel(const_str label);
		const_str NearestLabel(unsigned char* pos);
	};
}
