#pragma once

#include "../Global.h"
#include "../Managers/Manager.h"
#include "../Script/Event.h"
#include "../Common/con_set.h"
#include "../Common/con_arrayset.h"

namespace MOHPC
{
	class Archiver;

	class EventManager : public Manager
	{
		friend class Event;
		friend class Listener;
		friend class ScriptManager;

		CLASS_BODY(EventManager);

	public:
		con_map<Event *, EventDef> eventDefList;
		con_arrayset<command_t, command_t> commandList;

		con_map<const_str, uintptr_t> normalCommandList;
		con_map<const_str, uintptr_t> returnCommandList;
		con_map<const_str, uintptr_t> getterCommandList;
		con_map<const_str, uintptr_t> setterCommandList;

	private:
		static EventDef *lastEventDef;

		EventQueueNode EventQueue;
		bool bEventSystemStarted;
		static size_t totalEvents;

	public:
		MOHPC_EXPORTS EventManager();
		virtual ~EventManager() override;

		MOHPC_EXPORTS void ClearEventList();
		MOHPC_EXPORTS bool EventSystemStarted();
		void InitEvents();
		void LoadEvents();
		static void UnloadEvents();
		MOHPC_EXPORTS static size_t NumEventCommands();
		MOHPC_EXPORTS void ProcessPendingEvents();
		MOHPC_EXPORTS void ShutdownEvents(void);
		MOHPC_EXPORTS void ArchiveEvents(Archiver &arc);
		MOHPC_EXPORTS void UnarchiveEvents(Archiver &arc);

		MOHPC_EXPORTS uintptr_t GetEvent(str name, uint8_t type = EV_NORMAL);
		MOHPC_EXPORTS uintptr_t GetEventWithFlags(str name, int flags, uint8_t type = EV_NORMAL);

		MOHPC_EXPORTS command_t *GetEventInfo(int eventnum);
		MOHPC_EXPORTS unsigned int GetEventFlags(uintptr_t eventnum);
		MOHPC_EXPORTS const char *GetEventName(uintptr_t eventnum);

		MOHPC_EXPORTS uintptr_t FindEventNum(const str& s);
		MOHPC_EXPORTS uintptr_t FindNormalEventNum(const_str s);
		MOHPC_EXPORTS uintptr_t FindNormalEventNum(const str& s);
		MOHPC_EXPORTS uintptr_t FindReturnEventNum(const_str s);
		MOHPC_EXPORTS uintptr_t FindReturnEventNum(const str& s);
		MOHPC_EXPORTS uintptr_t FindSetterEventNum(const_str s);
		MOHPC_EXPORTS uintptr_t FindSetterEventNum(const str& s);
		MOHPC_EXPORTS uintptr_t FindGetterEventNum(const_str s);
		MOHPC_EXPORTS uintptr_t FindGetterEventNum(const str& s);

		MOHPC_EXPORTS void SortEventList(Container<uintptr_t> *sortedList);
		MOHPC_EXPORTS void ClassEvents(const char *classname, bool print_to_disk);
		MOHPC_EXPORTS void DumpClass(FILE * class_file, const char * className);
		MOHPC_EXPORTS void DumpAllClasses();

		MOHPC_EXPORTS Event *NewEvent();
		MOHPC_EXPORTS Event *NewEvent(const Event* ev);
		MOHPC_EXPORTS Event *NewEvent(const Event& ev);
		MOHPC_EXPORTS Event *NewEvent(uintptr_t eventnum);
		MOHPC_EXPORTS Event *NewEvent(const str& command);
		MOHPC_EXPORTS Event *NewEvent(const str& command, uint8_t type);

	protected:
		virtual void Init() override;

	private:
#ifdef _WIN32
		static int compareEvents(void *context, const void *arg1, const void *arg2);
#else
		static int compareEvents(const void *arg1, const void *arg2, void *context);
#endif
	};
	using EventManagerPtr = SharedPtr<EventManager>;
};
