#include <Shared.h>
#include <MOHPC/Script/ScriptContainer.h>
#include <MOHPC/Script/ScriptThread.h>
#include <MOHPC/Script/GameScript.h>
#include <MOHPC/Script/ScriptVariable.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/ScriptVM.h>
#include <MOHPC/Managers/ScriptManager.h>

using namespace MOHPC;

CLASS_DECLARATION(Listener, ScriptContainer, NULL)
{
	{ NULL, NULL }
};

ScriptContainer::ScriptContainer(GameScript *gameScript, Listener *self)
{
	m_Self = self;
	m_Script = gameScript;
	m_Threads = NULL;

	ScriptManager* Director = GetScriptManager();
	LL_SafeAddFirst(Director->ContainerHead, this, Next, Prev);
}

ScriptContainer::ScriptContainer()
{
	m_Self = NULL;
	m_Script = NULL;
	m_Threads = NULL;

	ScriptManager* Director = GetScriptManager();
	LL_SafeAddFirst(Director->ContainerHead, this, Next, Prev);
}

ScriptContainer::~ScriptContainer()
{
	if (m_Script == NULL)
	{
		//ScriptError("Attempting to delete dead class.");
		return;
	}

	ScriptManager* Director = GetScriptManager();
	LL_SafeRemoveRoot(Director->ContainerHead, this, Next, Prev);

	KillThreads();

	if (!m_Script->m_Filename)
	{
		// This is a temporary gamescript
		delete m_Script;
	}
}

void ScriptContainer::Archive(Archiver& arc)
{
}

void ScriptContainer::ArchiveInternal(Archiver& arc)
{
	Listener::Archive(arc);

	/*
	arc.ArchiveObjectPosition(this);
	arc.ArchiveSafePointer(&m_Self);
	GameScript::Archive(arc, m_Script);
	*/
}

void ScriptContainer::ArchiveScript(Archiver& arc, ScriptContainer **obj)
{
	/*
	ScriptContainer *scr;
	ScriptVM *m_current;
	ScriptThread *m_thread;
	int num;
	int i;

	if (arc.Saving())
	{
		scr = *obj;
		scr->ArchiveInternal(arc);

		num = 0;
		for (m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next)
			num++;

		arc.ArchiveInteger(&num);

		for (m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next)
			m_current->m_Thread->ArchiveInternal(arc);
	}
	else
	{
		scr = new ScriptContainer();
		scr->ArchiveInternal(arc);

		arc.ArchiveInteger(&num);

		for (i = 0; i < num; i++)
		{
			m_thread = new ScriptThread(scr, NULL);
			m_thread->ArchiveInternal(arc);
		}

		*obj = scr;
	}
	*/
}

void ScriptContainer::ArchiveCodePos(Archiver& arc, unsigned char **codePos)
{
	m_Script->ArchiveCodePos(arc, codePos);
}

ScriptThread *ScriptContainer::CreateThreadInternal(const ScriptVariable& label)
{
	GameScript *scr;
	ScriptThread *thread = NULL;

	ScriptManager* Director = GetScriptManager();

	if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING)
	{
		ScriptContainer *scriptClass = Director->CurrentScriptContainer();
		scr = scriptClass->GetScript();

		if (label.GetType() == VARIABLE_CONSTSTRING)
			thread = Director->CreateScriptThread(scr, m_Self, label.constStringValue());
		else
			thread = Director->CreateScriptThread(scr, m_Self, label.stringValue());
	}
	else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1)
	{
		ScriptVariable *script = label[1];
		ScriptVariable *labelname = label[2];

		if (script->GetType() == VARIABLE_CONSTSTRING)
			scr = Director->GetGameScript(script->constStringValue());
		else
			scr = Director->GetGameScript(script->stringValue());

		if (labelname->GetType() == VARIABLE_CONSTSTRING)
			thread = Director->CreateScriptThread(scr, m_Self, labelname->constStringValue());
		else
			thread = Director->CreateScriptThread(scr, m_Self, labelname->stringValue());
	}
	else
	{
		ScriptError("ScriptContainer::CreateThreadInternal: bad argument format");
	}

	return thread;
}

ScriptThread *ScriptContainer::CreateScriptInternal(const ScriptVariable& label)
{
	GameScript *scr;
	ScriptThread *thread = NULL;

	ScriptManager* Director = GetScriptManager();

	if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING)
	{
		if (label.GetType() == VARIABLE_CONSTSTRING)
			thread = Director->CreateScriptThread(Director->GetGameScript(label.stringValue()), m_Self, "");
		else
			thread = Director->CreateScriptThread(Director->GetGameScript(label.constStringValue()), m_Self, "");
	}
	else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1)
	{
		ScriptVariable *script = label[1];
		ScriptVariable *labelname = label[2];

		if (script->GetType() == VARIABLE_CONSTSTRING)
			scr = Director->GetGameScript(script->constStringValue());
		else
			scr = Director->GetGameScript(script->stringValue());

		if (labelname->GetType() == VARIABLE_CONSTSTRING)
			thread = Director->CreateScriptThread(scr, m_Self, labelname->constStringValue());
		else
			thread = Director->CreateScriptThread(scr, m_Self, labelname->stringValue());
	}
	else
	{
		ScriptError("ScriptContainer::CreateScriptInternal: bad label type '%s'", label.GetTypeName());
	}

	return thread;
}

void ScriptContainer::AddThread(ScriptVM *m_ScriptVM)
{
	m_ScriptVM->next = m_Threads;
	m_Threads = m_ScriptVM;
}

void ScriptContainer::KillThreads()
{
	if (!m_Threads)
	{
		return;
	}

	ScriptVM *m_current;
	ScriptVM *m_next;

	m_current = m_Threads;

	do
	{
		m_current->m_ScriptContainer = NULL;

		m_next = m_current->next;
		delete m_current->m_Thread;

	} while (m_current = m_next);

	m_Threads = NULL;
}

void ScriptContainer::RemoveThread(ScriptVM *m_ScriptVM)
{
	if (m_Threads == m_ScriptVM)
	{
		m_Threads = m_ScriptVM->next;

		if (m_Threads == NULL) {
			delete this;
		}
	}
	else
	{
		ScriptVM *m_current = m_Threads;
		ScriptVM *i;

		for (i = m_Threads->next; i != m_ScriptVM; i = i->next) {
			m_current = i;
		}

		m_current->next = i->next;
	}
}

const str& ScriptContainer::Filename()
{
	return m_Script->Filename();
}

unsigned char *ScriptContainer::FindLabel(const str& label)
{
	return m_Script->m_State->FindLabel(label);
}

unsigned char *ScriptContainer::FindLabel(const_str label)
{
	return m_Script->m_State->FindLabel(label);
}

const_str ScriptContainer::NearestLabel(unsigned char *pos)
{
	return m_Script->m_State->NearestLabel(pos);
}

StateScript *ScriptContainer::GetCatchStateScript(unsigned char *in, unsigned char *&out)
{
	return m_Script->GetCatchStateScript(in, out);
}

GameScript *ScriptContainer::GetScript()
{
	return m_Script;
}

Listener *ScriptContainer::GetSelf()
{
	return static_cast<Listener *>(m_Self.Pointer());
}
