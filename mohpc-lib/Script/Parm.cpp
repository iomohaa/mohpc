#include <Shared.h>
#include <MOHPC/Script/Parm.h>
#include <MOHPC/Script/Event.h>
#include <MOHPC/Script/ScriptThread.h>
#include <MOHPC/Managers/ScriptManager.h>

using namespace MOHPC;

Event EV_Parm_GetOther
(
	"other",
	EV_DEFAULT,
	NULL,
	NULL,
	"other",
	EV_GETTER
);

Event EV_Parm_GetOwner
(
	"owner",
	EV_DEFAULT,
	NULL,
	NULL,
	"owner",
	EV_GETTER
);

Event EV_Parm_GetPreviousThread
(
	"previousthread",
	EV_DEFAULT,
	NULL,
	NULL,
	"previousthread",
	EV_GETTER
);

void Parm::Archive(Archiver& arc)
{
	Listener::Archive(arc);

	//arc.ArchiveSafePointer(&other);
	//arc.ArchiveSafePointer(&owner);
}

void Parm::GetOther(Event *ev)
{
	ev->AddListener(other);
}

void Parm::GetOwner(Event *ev)
{
	ev->AddListener(owner);
}

void Parm::GetPreviousThread(Event *ev)
{
	ev->AddListener(GetScriptManager()->PreviousThread());
}

CLASS_DECLARATION(Listener, Parm, NULL)
{
	{ &EV_Parm_GetOther,				&Parm::GetOther },
	{ &EV_Parm_GetOwner,				&Parm::GetOwner },
	{ &EV_Parm_GetPreviousThread,		&Parm::GetPreviousThread },
	{ NULL, NULL }
};

