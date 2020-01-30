#include <Shared.h>
#include <MOHPC/Script/StateScript.h>
#include <MOHPC/Script/ScriptContainer.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/GameScript.h>
#include <MOHPC/Managers/ScriptManager.h>
#include <MOHPC/Script/Archiver.h>

using namespace MOHPC;

StateScript::StateScript()
{
	m_Parent = NULL;
}

template<>
void Entry< const_str, script_label_t >::Archive(Archiver& arc)
{
	/*
	unsigned int offset;

	arc.ArchiveConstString(key);

	if (arc.Saving())
	{
		offset = value.codepos - current_progBuffer;
		arc.ArchiveUnsigned(&offset);
	}
	else
	{
		arc.ArchiveUnsigned(&offset);
		value.codepos = current_progBuffer + offset;
		value.key = key;
	}

	arc.ArchiveBool(&value.isprivate);
	*/
}

void StateScript::Archive(Archiver& arc)
{
	label_list.Archive(arc);
}

bool StateScript::AddLabel(const_str label, unsigned char *pos, bool private_section)
{
	if (label_list.findKeyValue(label))
	{
		return true;
	}

	script_label_t &s = label_list.addKeyValue(label);

	s.codepos = pos;
	s.key = label;
	s.isprivate = private_section;

	if (!label_list.findKeyValue(0))
	{
		label_list.addKeyValue(0) = s;
	}

	reverse_label_list.AddObject(&s);

	return false;
}

bool StateScript::AddLabel(const str& label, unsigned char *pos, bool private_section)
{
	return AddLabel(GetScriptManager()->AddString(label), pos, private_section);
}

unsigned char *StateScript::FindLabel(const_str label)
{
	script_label_t *s;
	ScriptContainer *scriptClass;
	GameScript *script;

	s = label_list.findKeyValue(label);

	if (s)
	{
		// check if the label is a private function
		if (s->isprivate)
		{
			scriptClass = GetScriptManager()->CurrentScriptContainer();

			if (scriptClass)
			{
				script = scriptClass->GetScript();

				// now check if the label's statescript matches this statescript
				if (script->m_State != this)
				{
					ScriptError("Cannot call a private function.");
					return NULL;
				}
			}
		}

		return s->codepos;
	}
	else
	{
		return NULL;
	}
}

unsigned char *StateScript::FindLabel(str label)
{
	return FindLabel(GetScriptManager()->AddString(label));
}

const_str StateScript::NearestLabel(unsigned char *pos)
{
	intptr_t offset = pos - m_Parent->m_ProgBuffer;
	intptr_t bestOfs = 0;
	const_str label = 0;

	for (uintptr_t i = 1; i <= reverse_label_list.NumObjects(); i++)
	{
		script_label_t *l = reverse_label_list.ObjectAt(i);

		if ((l->codepos - m_Parent->m_ProgBuffer) >= bestOfs)
		{
			bestOfs = l->codepos - m_Parent->m_ProgBuffer;

			if (bestOfs > offset)
			{
				break;
			}

			label = l->key;
		}
	}

	return label;
}
