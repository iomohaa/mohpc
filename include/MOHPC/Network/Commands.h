#pragma once

namespace MOHPC
{
	class TokenParser;
	class MSG;

	namespace Network
	{
		class ICommand
		{
			friend class CommandList;

		private:
			ICommand* next;
			ICommand* prev;
			size_t id;

		public:
			ICommand(class CommandList& eventList);
			virtual ~ICommand();

			ICommand* getNext() const;

			template<typename T>
			T* getNextAs() const
			{
				return static_cast<T*>(getNext());
			}

			size_t getId() const;
		};

		class IConnectionlessCommand : public ICommand
		{
		private:
			const char* eventName;

		public:
			IConnectionlessCommand(CommandList& eventList, const char* inEventName);

			const char* getEventName() { return eventName; }

			virtual bool doRun(MSG& msg, TokenParser& script, class Event& ev) = 0;
		};

		/**
		 * List of commands
		 */
		class CommandList
		{
		private:
			ICommand* head;
			ICommand* last;

		public:
			CommandList();

			void add(ICommand* cmd);
			ICommand* getHead() const;
			size_t getNumCommands() const;

			template<typename T>
			T* getHeadAs() const
			{
				return static_cast<T*>(getHead());
			}
		};
	}
}
