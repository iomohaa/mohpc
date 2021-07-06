#include <MOHPC/Network/Client/CGame/Vote.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Utility/CommandManager.h>
#include <MOHPC/Common/Log.h>

#include <functional>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

static constexpr char MOHPC_LOG_NAMESPACE[] = "cg_vote";

voteInfo_t::voteInfo_t()
	: voteTime(0)
	, numVotesYes(0)
	, numVotesNo(0)
	, numUndecidedVotes(0)
	, modified(false)
{

}

uint64_t voteInfo_t::getVoteTime() const
{
	return voteTime;
}

uint32_t voteInfo_t::getNumVotesYes() const
{
	return numVotesYes;
}

uint32_t voteInfo_t::getNumVotesNo() const
{
	return numVotesNo;
}

uint32_t voteInfo_t::getNumVotesUndecided() const
{
	return numUndecidedVotes;
}

const char* voteInfo_t::getVoteString() const
{
	return voteString.c_str();
}

VoteListChoice::VoteListChoice(const str& inChoiceName, const str& inVoteString)
	: choiceName(inChoiceName)
	, voteString(inVoteString)
{
}

const char* VoteListChoice::getName() const
{
	return choiceName.c_str();
}

const char* VoteListChoice::getVoteString() const
{
	return voteString.c_str();
}

VoteOption::VoteOption(const str& inName, const str& inCommand)
	: optionName(inName)
	, commandString(inCommand)
{
}

const char* VoteOption::getName() const
{
	return optionName.c_str();
}

const char* VoteOption::getCommand() const
{
	return commandString.c_str();
}

VoteOptionList::VoteOptionList(const str& inName, const str& inCommand)
	: VoteOption(inName, inCommand)
{
}

void VoteOptionList::createVoteOption(const str& choiceName, const str& voteString)
{
	new (choiceList) VoteListChoice(choiceName, voteString);
}

void VoteOptionList::optimize()
{
	choiceList.Shrink();
}

size_t VoteOptionList::getNumChoices() const
{
	return choiceList.NumObjects();
}

const VoteListChoice* VoteOptionList::getChoice(uintptr_t index) const
{
	return &choiceList[index];
}

size_t VoteOptions::getNumOptions(optionType_e type) const
{
	switch (type)
	{
	case optionType_e::nochoices:
		return noChoicesOptionsList.NumObjects();
	case optionType_e::list:
		return listOptionsList.NumObjects();
	case optionType_e::text:
		return textOptionsList.NumObjects();
	case optionType_e::integerValue:
		return intOptionsList.NumObjects();
	case optionType_e::floatValue:
		return floatOptionsList.NumObjects();
	case optionType_e::client:
		return clientOptionsList.NumObjects();
	case optionType_e::clientnotself:
		return clientNotSelfOptionsList.NumObjects();
	default:
		return 0;
	}
}

const VoteOption* VoteOptions::getOption(optionType_e type, uintptr_t index) const
{
	switch (type)
	{
	case optionType_e::nochoices:
		return &noChoicesOptionsList.ObjectAt(index + 1);
	case optionType_e::list:
		return &listOptionsList.ObjectAt(index + 1);
	case optionType_e::text:
		return &textOptionsList.ObjectAt(index + 1);
	case optionType_e::integerValue:
		return &intOptionsList.ObjectAt(index + 1);
	case optionType_e::floatValue:
		return &floatOptionsList.ObjectAt(index + 1);
	case optionType_e::client:
		return &clientOptionsList.ObjectAt(index + 1);
	case optionType_e::clientnotself:
		return &clientNotSelfOptionsList.ObjectAt(index + 1);
	default:
		return nullptr;
	}
}

void VoteOptions::addNoChoiceOption(const str& choiceName, const str& voteString)
{
	new (noChoicesOptionsList) VoteOption(choiceName, voteString);
}

VoteOptionList& VoteOptions::createListVoteOption(const str& choiceName, const str& voteString)
{
	VoteOptionList* const list = new (listOptionsList) VoteOptionList(choiceName, voteString);
	return *list;
}

void VoteOptions::addTextOption(const str& choiceName, const str& voteString)
{
	new (textOptionsList) VoteOption(choiceName, voteString);
}

void VoteOptions::addIntegerOption(const str& choiceName, const str& voteString)
{
	new (intOptionsList) VoteOption(choiceName, voteString);
}

void VoteOptions::addFloatOption(const str& choiceName, const str& voteString)
{
	new (floatOptionsList) VoteOption(choiceName, voteString);
}

void VoteOptions::addClientOption(const str& choiceName, const str& voteString, bool allowSelf)
{
	if (allowSelf)
	{
		new (clientOptionsList) VoteOption(choiceName, voteString);
	}
	else
	{
		new (clientNotSelfOptionsList) VoteOption(choiceName, voteString);
	}
}

void VoteOptions::optimize()
{
	noChoicesOptionsList.Shrink();
	listOptionsList.Shrink();
	textOptionsList.Shrink();
	intOptionsList.Shrink();
	floatOptionsList.Shrink();
	clientOptionsList.Shrink();
	clientNotSelfOptionsList.Shrink();
}

VoteException::VoteException(const TokenParser& parser)
	: lineNumber(parser.GetLineNumber())
{
}

size_t VoteException::getLineNumber() const
{
	return lineNumber;
}

VoteOptionException::VoteOptionException(const TokenParser& parser, const str& inOptionName)
	: VoteException(parser)
	, optionName(inOptionName)
{
}

const char* VoteOptionException::getOptionName() const
{
	return optionName.c_str();
}

IllegalOptionTypeException::IllegalOptionTypeException(const TokenParser& parser, const str& inOptionName, const char* inOptionType)
	: VoteOptionException(parser, inOptionName)
	, optionType(inOptionType)
{
}

const char* IllegalOptionTypeException::getOptionType() const
{
	return optionType.c_str();
}

const char* IllegalOptionTypeException::what() const noexcept 
{
	return "Illegal option type specified";
}

const char* BadOptionHeaderException::what() const noexcept 
{
	return "Found choices list without option header";
}

const char* EmptyOptionNameException::what() const noexcept 
{
	return "Empty option name";
}

const char* ChoiceListOnNonListOptionException::what() const noexcept 
{
	return "Choices list specified for non-list option";
}

const char* EmptyOptionCommandException::what() const noexcept 
{
	return "Option without a command specified";
}

const char* IllegalEndOfChoicesException::what() const noexcept 
{
	return "Illegal end of choises list without list being started";
}

const char* NoListChoiceVoteStringException::what() const noexcept 
{
	return "Missing '{'. No choices list specified for list option";
}

void VoteOptionsParser::begin(const char* options)
{
	voteOptionsStr = options;
}

void VoteOptionsParser::append(const char* options)
{
	voteOptionsStr += options;
}

void VoteOptionsParser::end(const char* options, VoteOptions& voteOptions)
{
	append(options);

	const size_t len = voteOptionsStr.length();
	// fix options string
	for (size_t i = 0; i < len; ++i)
	{
		if (voteOptionsStr[i] == 1) {
			voteOptionsStr[i] = '\"';
		}
	}

	parseVoteOptions(voteOptions);
	voteOptionsStr.clear();
}

void VoteOptionsParser::parseVoteOptions(VoteOptions& voteOptions)
{
	// FIXME: For now, ignore exceptions, instead, log them.
	// in the original game, the client is disconnected
	// but it's better to not do so and just log it instead, it's not dangerous

	try
	{
		parseVoteOptions(voteOptionsStr, voteOptionsStr.length(), voteOptions);
	}
	catch (IllegalOptionTypeException& e)
	{
		MOHPC_LOG(
			Error,
			"Vote option exception -- %s. Line %ul, option \"%s\": option type \"%s\"",
			e.what(),
			e.getOptionName(),
			e.getLineNumber(),
			e.getOptionType()
		);
	}
	catch (VoteOptionException& e)
	{
		MOHPC_LOG(Error, "Vote option exception -- %s. Line %ul, option \"%s\"", e.what(), e.getOptionName(), e.getLineNumber());
	}
	catch (VoteException& e)
	{
		MOHPC_LOG(Error, "Vote exception -- %s. Line %ul", e.what(), e.getLineNumber());
	}
}

void VoteOptionsParser::parseVoteOptions(const char* options, size_t len, VoteOptions& voteOptions)
{
	TokenParser parser;
	parser.Parse(options, len);

	while (parser.TokenAvailable(true))
	{
		// get the option name
		const char* token = parser.GetToken(true);
		if (!str::icmp(token, "{") || !str::icmp(token, "}"))
		{
			// this is illegal and unexpected
			throw BadOptionHeaderException(parser);
		}

		if (!*token)
		{
			// empty option name
			throw EmptyOptionNameException(parser);
		}

		const str optionName = token;

		// get the option type
		const str commandString = parser.GetToken(false);

		if (parser.TokenAvailable(false))
		{
			const char* optionType = parser.GetToken(false);
			const bool wasParsed = parseOptionType(optionName, commandString, optionType, voteOptions, parser);
			if (!wasParsed && parser.TokenAvailable(true))
			{
				token = parser.GetToken(true);
				if (!str::icmp(token, "{"))
				{
					// bad option list
					throw ChoiceListOnNonListOptionException(parser, optionName.c_str());
				}

				parser.UnGetToken();
			}
		}
		else {
			throw EmptyOptionCommandException(parser, optionName.c_str());
		}
	}

	voteOptions.optimize();
}

bool VoteOptionsParser::parseOptionType(const str& optionName, const str& commandString, const char* optionType, VoteOptions& voteOptions, TokenParser& parser)
{
	if (!str::icmp(optionType, "nochoices")) {
		voteOptions.addNoChoiceOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "list")) {
		VoteOptionList& list = voteOptions.createListVoteOption(optionName, commandString);
		parseChoiceList(list, parser);
		return true;
	}
	else if (!str::icmp(optionType, "text")) {
		voteOptions.addTextOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "integer")) {
		voteOptions.addIntegerOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "float")) {
		voteOptions.addFloatOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "client")) {
		voteOptions.addClientOption(optionName, commandString, true);
	}
	else if (!str::icmp(optionType, "clientnotself")) {
		voteOptions.addClientOption(optionName, commandString, false);
	}
	else {
		// unknown option type
		throw IllegalOptionTypeException(parser, optionName, optionType);
	}

	return false;

}

void VoteOptionsParser::parseChoiceList(VoteOptionList& list, TokenParser& parser)
{
	if (parser.TokenAvailable(true))
	{
		// parse the beginning
		const char* token = parser.GetToken(true);
		if (!str::icmp(token, "{"))
		{
			while (parser.TokenAvailable(true))
			{
				// get the choice name
				token = parser.GetToken(true);
				if (!str::icmp(token, "}"))
				{
					// end of list
					break;
				}

				if (!parser.TokenAvailable(false))
				{
					// no choice name
					throw NoListChoiceVoteStringException(parser, list.getName());
				}

				const str choiceName = token;
				// get the vote string
				const char* voteString = parser.GetToken(false);

				// create a new vote option
				list.createVoteOption(choiceName, voteString);
			}
		}
		else
		{
			// empty list
			throw NoListChoiceVoteStringException(parser, list.getName());
		}
	}
	else
	{
		// nothing after
		throw NoListChoiceVoteStringException(parser, list.getName());
	}

	list.optimize();
}

VoteManager::VoteManager()
	: startReadFromServerHandler(*this)
	, continueReadFromServerHandler(*this)
	, finishReadFromServerHandler(*this)
	, voteTime(0)
	, numVotesYes(0)
	, numVotesNo(0)
	, numUndecidedVotes(0)
	, modified(false)
{
}

VoteManager::HandlerList& VoteManager::handlers()
{
	return handlerList;
}

const VoteManager::HandlerList& VoteManager::handlers() const
{
	return handlerList;
}

size_t VoteManager::getNumCommandsToRegister() const
{
	return 3;
}

void VoteManager::registerCommands(CommandManager& commandManager)
{
	commandManager.add("vo0", &startReadFromServerHandler);
	commandManager.add("vo1", &continueReadFromServerHandler);
	commandManager.add("vo2", &finishReadFromServerHandler);
}

void VoteManager::commandStartReadFromServer(TokenParser& args)
{
	voteOptionsParser.begin(args.GetString(true, false));
}

void VoteManager::commandContinueReadFromServer(TokenParser& args)
{
	voteOptionsParser.append(args.GetString(true, false));
}

void VoteManager::commandFinishReadFromServer(TokenParser& args)
{
	voteOptionsParser.end(args.GetString(true, false), voteOptions);

	handlers().receivedVoteOptionsHandler.broadcast(voteOptions);
}

bool VoteManager::isModified() const
{
	return modified;
}

void VoteManager::notifyDirty()
{
	modified = false;
	handlers().voteModifiedHandler.broadcast(*this);
}

void VoteManager::setVoteTime(uint64_t time)
{
	voteTime = time;
}

uint64_t VoteManager::getVoteTime() const
{
	return voteTime;
}

void VoteManager::setVoteString(const char* string)
{
	voteString = string;
}

const char* VoteManager::getVoteString() const
{
	return voteString.c_str();
}

void VoteManager::setNumVotesYes(uint32_t count)
{
	numVotesYes = count;
}

void VoteManager::setNumVotesNo(uint32_t count)
{
	numVotesNo = count;
}

void VoteManager::setNumVotesUndecided(uint32_t count)
{
	numUndecidedVotes = count;
}

void VoteManager::getVotesCount(uint32_t& numYes, uint32_t& numNo, uint32_t& numUndecided) const
{
	numYes = numVotesYes;
	numNo = numVotesNo;
	numUndecided = numUndecidedVotes;
}

void VoteManager::markDirty()
{
	modified = true;
}
