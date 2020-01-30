#include <Shared.h>
#include <MOHPC/Script/ScriptThreadLabel.h>
#include <MOHPC/Script/ScriptThread.h>
#include <MOHPC/Script/ScriptContainer.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/ScriptVariable.h>
#include <MOHPC/Script/Level.h>
#include <MOHPC/Script/ConstStr.h>
#include <MOHPC/Script/GameScript.h>
#include <MOHPC/Script/StateScript.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>

using namespace MOHPC;

ScriptThreadLabel::ScriptThreadLabel()
{
	m_Script = NULL;
	m_Label = STRING_EMPTY;
}

ScriptThread *ScriptThreadLabel::Create(Listener *listener)
{
	if (!m_Script)
	{
		return NULL;
	}

	ScriptContainer *scriptClass = new ScriptContainer(m_Script, listener);
	ScriptThread *thread = GetScriptManager()->CreateScriptThread(scriptClass, m_Label);

	return thread;
}

void ScriptThreadLabel::Execute(Listener *listener)
{
	if (!m_Script) {
		return;
	}

	ScriptThread *thread = Create(listener);

	if (thread)
	{
		thread->Execute();
	}
}

void ScriptThreadLabel::Execute(Listener *listener, Event &ev)
{
	Execute(listener, &ev);
}

void ScriptThreadLabel::Execute(Listener *listener, Event *ev)
{
	if (!m_Script) {
		return;
	}

	ScriptThread *thread = Create(listener);

	if (thread)
	{
		thread->Execute(ev);
	}
}

void ScriptThreadLabel::Set(const char *label)
{
	str script;
	char buffer[1023];
	char *p = buffer;
	bool foundLabel = false;

	if (!label || !*label)
	{
		m_Script = NULL;
		m_Label = STRING_EMPTY;
		return;
	}

	strcpy(buffer, label);

	while (*p != '\0')
	{
		if (p[0] == ':' && p[1] == ':')
		{
			*p = '\0';

			script = buffer;
			m_Label = GetScriptManager()->AddString(&p[2]);
			foundLabel = true;

			break;
		}

		p++;
	}

	if (!foundLabel)
	{
		script = GetGameManager()->GetLevel()->GetMapName();
		m_Label = GetScriptManager()->AddString(buffer);
	}

	m_Script = GetScriptManager()->GetGameScript(script);

	if (!m_Script->m_State->FindLabel(m_Label))
	{
		str l = GetScriptManager()->GetString(m_Label);

		m_Script = NULL;
		m_Label = STRING_EMPTY;

		ScriptError("^~^~^ Could not find label '%s' in '%s'", l.c_str(), script.c_str());
	}
}

void ScriptThreadLabel::SetScript(const ScriptVariable& label)
{
	if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING)
	{
		m_Script = GetScriptManager()->GetGameScript(label.stringValue());
		m_Label = STRING_EMPTY;
	}
	else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1)
	{
		ScriptVariable *script = label[1];
		ScriptVariable *labelname = label[2];

		m_Script = GetScriptManager()->GetGameScript(script->stringValue());
		m_Label = labelname->constStringValue();

		if (!m_Script->m_State->FindLabel(m_Label))
		{
			m_Script = NULL;
			m_Label = STRING_EMPTY;

			ScriptError("^~^~^ Could not find label '%s' in '%s'", labelname->stringValue().c_str(), script->stringValue().c_str());
		}
	}
	else
	{
		ScriptError("ScriptThreadLabel::SetScript: bad label type '%s'", label.GetTypeName());
	}
}

void ScriptThreadLabel::SetScript(const char *label)
{
	Set(label);
}

void ScriptThreadLabel::SetThread(const ScriptVariable& label)
{
	ScriptThread *thread = NULL;

	if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING)
	{
		m_Script = GetScriptManager()->CurrentScriptContainer()->GetScript();
		m_Label = label.constStringValue();
	}
	else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1)
	{
		ScriptVariable *script = label[1];
		ScriptVariable *labelname = label[2];

		m_Script = GetScriptManager()->GetGameScript(script->stringValue());
		m_Label = labelname->constStringValue();

		if (!m_Script->m_State->FindLabel(m_Label))
		{
			m_Script = NULL;
			m_Label = STRING_EMPTY;

			ScriptError("^~^~^ Could not find label '%s' in '%s'", labelname->stringValue().c_str(), script->stringValue().c_str());
		}
	}
	else
	{
		ScriptError("ScriptThreadLabel::SetThread bad label type '%s'", label.GetTypeName());
	}
}

bool ScriptThreadLabel::TrySet(const char *label)
{
	try
	{
		Set(label);
	}
	catch (const char *string)
	{
		printf("%s\n", string);
		return false;
	}

	return true;
}

bool ScriptThreadLabel::TrySet(const_str label)
{
	return TrySet(GetScriptManager()->GetString(label));
}

bool ScriptThreadLabel::TrySetScript(const char *label)
{
	try
	{
		SetScript(label);
	}
	catch (const char *string)
	{
		printf("%s\n", string);
		return false;
	}

	return true;
}

bool ScriptThreadLabel::TrySetScript(const_str label)
{
	return TrySetScript(GetScriptManager()->GetString(label));
}

bool ScriptThreadLabel::IsSet(void)
{
	return m_Script != NULL ? true : false;
}

void ScriptThreadLabel::Archive(Archiver& arc)
{
	m_Script->Archive(arc);
	//arc.ArchiveConstString(m_Label);
}

