#pragma once

#include "Listener.h"

namespace MOHPC
{
	class Parm : public Listener
	{
	public:
		// General trigger variables
		SafePtr<Listener> other;
		SafePtr<Listener> owner;

		// Failure variables
		bool movedone;
		bool movefail;
		bool motionfail;
		bool upperfail;
		bool sayfail;

	public:
		CLASS_PROTOTYPE(Parm);

		virtual void Archive(Archiver& arc);

		void GetOther(Event *ev);
		void GetOwner(Event *ev);
		void GetPreviousThread(Event *ev);
	};
}
