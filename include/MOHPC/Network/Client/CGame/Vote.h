/**
 * Vote system, present since SH 2.0.
 *
 * It is intended to be used with widgets.
 */

#pragma once

#include "../../NetGlobal.h"
#include "../../../Common/str.h"
#include "../../../Utility/HandlerList.h"

#include <morfuse/Container/Container.h>

#include <list>

namespace MOHPC
{
class TokenParser;
class CommandManager;

namespace Network
{
	namespace CGame
	{
		class VoteListChoice;
		class VoteOptions;
		class VoteManager;

		namespace VoteHandlers
		{
			/**
			 * Called when the vote options has been received and parsed.
			 *
			 * @param options Vote options.
			 */
			struct ReceivedVoteOptions : public HandlerNotifyBase<void(const VoteOptions& options)> {};

			/**
			 * Called when the vote options has been received and parsed.
			 *
			 * @param options Vote options.
			 */
			struct VoteModified : public HandlerNotifyBase<void(const VoteManager& voteManager)> {};
		}

		//=============================
		// Vote enums
		//=============================

		enum class optionType_e : unsigned char
		{
			/** No input. */
			nochoices,
			/** List of choices. */
			list,
			/** Text input type command. */
			text,
			/** Accepts an integer as an input. */
			integerValue,
			/** Accepts a float number as input. */
			floatValue,
			/** A list of clients, one to choose. */
			client,
			/** A list of clients (excluding self), one to choose. */
			clientnotself,
			max
		};

		//=============================
		// Vote classes
		//=============================

		struct voteInfo_t
		{
		public:
			voteInfo_t();

			/** Return the last time vote has started. */
			MOHPC_NET_EXPORTS uint64_t getVoteTime() const;

			/** Return the number of players that voted yes. */
			MOHPC_NET_EXPORTS uint32_t getNumVotesYes() const;

			/** Return the number of players that voted no. */
			MOHPC_NET_EXPORTS uint32_t getNumVotesNo() const;

			/** Return the number of players that didn't vote. */
			MOHPC_NET_EXPORTS uint32_t getNumVotesUndecided() const;

			/** Return the vote name/text. */
			MOHPC_NET_EXPORTS const char* getVoteString() const;

		public:
			uint64_t voteTime;
			uint32_t numVotesYes;
			uint32_t numVotesNo;
			uint32_t numUndecidedVotes;
			str voteString;
			bool modified : 1;
		};

		/**
		 * Vote option that specify a command when calling a vote.
		 */
		class VoteOption
		{
		public:
			VoteOption(const str& inName, const str& inCommand);

			/** Return the option name. */
			MOHPC_NET_EXPORTS const char* getName() const;

			/** Return the command to use when calling a vote. */
			MOHPC_NET_EXPORTS const char* getCommand() const;

		private:
			str optionName;
			str commandString;
		};

		/**
		 * Provides a list of choice for this option.
		 */
		class VoteOptionList : public VoteOption
		{
		public:
			VoteOptionList(const str& inName, const str& inCommand);

			/** Return the number of choices. */
			MOHPC_NET_EXPORTS size_t getNumChoices() const;

			/**
			 * Return the choice at the specified index.
			 *
			 * @param index The index to get the choice from.
			 */
			MOHPC_NET_EXPORTS const VoteListChoice* getChoice(uintptr_t index) const;

			void createVoteOption(const str& choiceName, const str& voteString);
			void optimize();

		private:
			mfuse::con::Container<VoteListChoice> choiceList;
		};

		/**
		 * It represents a single choice, in a list of choices.
		 */
		class VoteListChoice
		{
		public:
			VoteListChoice(const str& inChoiceName, const str& inVoteString);

			/** Return the name of the choice. */
			MOHPC_NET_EXPORTS const char* getName() const;

			/** Return the command for the choice. */
			MOHPC_NET_EXPORTS const char* getVoteString() const;

		private:
			str choiceName;
			str voteString;
		};

		/**
		 * This holds all options.
		 */
		class VoteOptions
		{
		public:
			/**
			 * Return the number of options for the specified option type.
			 *
			 * @param type The option type.
			 */
			MOHPC_NET_EXPORTS size_t getNumOptions(optionType_e type) const;

			/**
			 * Return the number of options for the specified option type.
			 *
			 * @param type The option type.
			 * @param index The index to get the option from.
			 * @return A vote option.
			 */
			MOHPC_NET_EXPORTS const VoteOption* getOption(optionType_e type, uintptr_t index) const;

			void addNoChoiceOption(const str& choiceName, const str& voteString);
			VoteOptionList& createListVoteOption(const str& choiceName, const str& voteString);
			void addTextOption(const str& choiceName, const str& voteString);
			void addIntegerOption(const str& choiceName, const str& voteString);
			void addFloatOption(const str& choiceName, const str& voteString);
			void addClientOption(const str& choiceName, const str& voteString, bool allowSelf);
			void optimize();

		private:
			// using contiguous memory for every type.
			mfuse::con::Container<VoteOption> noChoicesOptionsList;
			mfuse::con::Container<VoteOptionList> listOptionsList;
			mfuse::con::Container<VoteOption> textOptionsList;
			mfuse::con::Container<VoteOption> intOptionsList;
			mfuse::con::Container<VoteOption> floatOptionsList;
			mfuse::con::Container<VoteOption> clientOptionsList;
			mfuse::con::Container<VoteOption> clientNotSelfOptionsList;
		};

		class VoteOptionsParser
		{
		public:
			void begin(const char* options);
			void append(const char* options);
			void end(const char* options, VoteOptions& voteOptions);

		private:
			void parseVoteOptions(VoteOptions& voteOptions);
			void parseVoteOptions(const char* options, size_t len, VoteOptions& voteOptions);
			bool parseOptionType(const str& optionName, const str& commandString, const char* optionType, VoteOptions& voteOptions, TokenParser& parser);
			void parseChoiceList(VoteOptionList& list, TokenParser& parser);

		private:
			str voteOptionsStr;
		};

		class VoteManager
		{
		public:
			class HandlerList
			{
			public:
				FunctionList<VoteHandlers::ReceivedVoteOptions> receivedVoteOptionsHandler;
				FunctionList<VoteHandlers::VoteModified> voteModifiedHandler;
			};

		public:
			VoteManager();

			MOHPC_NET_EXPORTS HandlerList& handlers();
			MOHPC_NET_EXPORTS const HandlerList& handlers() const;

			size_t getNumCommandsToRegister() const;
			void registerCommands(CommandManager& commandManager);

			void commandStartReadFromServer(TokenParser& args);
			void commandContinueReadFromServer(TokenParser& args);
			void commandFinishReadFromServer(TokenParser& args);
			bool isModified() const;
			void notifyDirty();

			void setVoteTime(uint64_t time);
			MOHPC_NET_EXPORTS uint64_t getVoteTime() const;

			void setVoteString(const char* string);
			MOHPC_NET_EXPORTS const char* getVoteString() const;

			void setNumVotesYes(uint32_t count);
			void setNumVotesNo(uint32_t count);
			void setNumVotesUndecided(uint32_t count);
			MOHPC_NET_EXPORTS void getVotesCount(uint32_t& numYes, uint32_t& numNo, uint32_t& numUndecided) const;

		private:
			void markDirty();

		public:
			VoteOptions voteOptions;
			VoteOptionsParser voteOptionsParser;
			HandlerList handlerList;
			uint64_t voteTime;
			uint32_t numVotesYes;
			uint32_t numVotesNo;
			uint32_t numUndecidedVotes;
			str voteString;
			bool modified : 1;
		};

		//=============================
		// Vote exceptions
		//=============================

		/** General vote exception. */
		class VoteException : public std::exception
		{
		public:
			VoteException(const TokenParser& parser);
			virtual ~VoteException() = default;

			/** Return the line number that caused the error. */
			MOHPC_NET_EXPORTS size_t getLineNumber() const;

		public:
			size_t lineNumber;
		};

		/** Option-specific vote exception. */
		class VoteOptionException : public VoteException
		{
		public:
			VoteOptionException(const TokenParser& parser, const str& inOptionName);

			/** Return the option name responsible for the error. */
			MOHPC_NET_EXPORTS const char* getOptionName() const;

		private:
			str optionName;
		};

		/** Found choices list without option header. */
		class BadOptionHeaderException : public VoteException
		{
			using VoteException::VoteException;

		public:
			const char* what() const noexcept override;
		};

		/** Empty option name. */
		class EmptyOptionNameException : public VoteException
		{
			using VoteException::VoteException;

		public:
			const char* what() const noexcept override;
		};

		/** Choices list specified for non-list option. */
		class ChoiceListOnNonListOptionException : public VoteOptionException
		{
			using VoteOptionException::VoteOptionException;

		public:
			const char* what() const noexcept override;
		};

		/** Option without a command specified. */
		class EmptyOptionCommandException : public VoteOptionException
		{
			using VoteOptionException::VoteOptionException;

		public:
			const char* what() const noexcept override;
		};

		/** Illegal end of choises list without list being started. */
		class IllegalEndOfChoicesException : public VoteOptionException
		{
			using VoteOptionException::VoteOptionException;

		public:
			const char* what() const noexcept override;
		};

		/** Illegal option type specified. */
		class IllegalOptionTypeException : public VoteOptionException
		{
		public:
			IllegalOptionTypeException(const TokenParser& parser, const str& inOptionName, const char* optionType);

			/** Return the option type that is invalid. */
			MOHPC_NET_EXPORTS const char* getOptionType() const;
			const char* what() const noexcept override;

		private:
			str optionType;
		};

		/** Missing '{'. No choices list specified for list option. */
		class NoListChoiceVoteStringException : public VoteOptionException
		{
			using VoteOptionException::VoteOptionException;

		public:
			const char* what() const noexcept override;
		};
	}
}
}