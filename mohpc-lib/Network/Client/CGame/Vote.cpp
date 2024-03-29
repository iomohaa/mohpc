#include <MOHPC/Network/Client/CGame/Vote.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Utility/CommandManager.h>
#include <MOHPC/Common/Log.h>

#include <functional>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

static constexpr char MOHPC_LOG_NAMESPACE[] = "cg_vote";

MOHPC_OBJECT_DEFINITION(VoteManager)

VoteManager::VoteManager()
	: voteTime(0)
	, numVotesYes(0)
	, numVotesNo(0)
	, numUndecidedVotes(0)
{
}

VoteManager::~VoteManager()
{
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
	choiceList.emplace_back(choiceName, voteString);
}

void VoteOptionList::optimize()
{
	choiceList.shrink_to_fit();
}

size_t VoteOptionList::getNumChoices() const
{
	return choiceList.size();
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
		return noChoicesOptionsList.size();
	case optionType_e::list:
		return listOptionsList.size();
	case optionType_e::text:
		return textOptionsList.size();
	case optionType_e::integerValue:
		return intOptionsList.size();
	case optionType_e::floatValue:
		return floatOptionsList.size();
	case optionType_e::client:
		return clientOptionsList.size();
	case optionType_e::clientnotself:
		return clientNotSelfOptionsList.size();
	default:
		return 0;
	}
}

const VoteOption* VoteOptions::getOption(optionType_e type, uintptr_t index) const
{
	switch (type)
	{
	case optionType_e::nochoices:
		return &noChoicesOptionsList.at(index);
	case optionType_e::list:
		return &listOptionsList.at(index);
	case optionType_e::text:
		return &textOptionsList.at(index);
	case optionType_e::integerValue:
		return &intOptionsList.at(index);
	case optionType_e::floatValue:
		return &floatOptionsList.at(index);
	case optionType_e::client:
		return &clientOptionsList.at(index);
	case optionType_e::clientnotself:
		return &clientNotSelfOptionsList.at(index);
	default:
		return nullptr;
	}
}

void VoteOptions::addNoChoiceOption(const str& choiceName, const str& voteString)
{
	noChoicesOptionsList.emplace_back(choiceName, voteString);
}

VoteOptionList& VoteOptions::createListVoteOption(const str& choiceName, const str& voteString)
{
	return listOptionsList.emplace_back(choiceName, voteString);
}

void VoteOptions::addTextOption(const str& choiceName, const str& voteString)
{
	textOptionsList.emplace_back(choiceName, voteString);
}

void VoteOptions::addIntegerOption(const str& choiceName, const str& voteString)
{
	intOptionsList.emplace_back(choiceName, voteString);
}

void VoteOptions::addFloatOption(const str& choiceName, const str& voteString)
{
	floatOptionsList.emplace_back(choiceName, voteString);
}

void VoteOptions::addClientOption(const str& choiceName, const str& voteString, bool allowSelf)
{
	if (allowSelf)
	{
		clientOptionsList.emplace_back(choiceName, voteString);
	}
	else
	{
		clientNotSelfOptionsList.emplace_back(choiceName, voteString);
	}
}

void VoteOptions::optimize()
{
	noChoicesOptionsList.shrink_to_fit();
	listOptionsList.shrink_to_fit();
	textOptionsList.shrink_to_fit();
	intOptionsList.shrink_to_fit();
	floatOptionsList.shrink_to_fit();
	clientOptionsList.shrink_to_fit();
	clientNotSelfOptionsList.shrink_to_fit();
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
		parseVoteOptions(voteOptionsStr.c_str(), voteOptionsStr.length(), voteOptions);
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
		if (!strHelpers::icmp(token, "{") || !strHelpers::icmp(token, "}"))
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
				if (!strHelpers::icmp(token, "{"))
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
	if (!strHelpers::icmp(optionType, "nochoices")) {
		voteOptions.addNoChoiceOption(optionName, commandString);
	}
	else if (!strHelpers::icmp(optionType, "list")) {
		VoteOptionList& list = voteOptions.createListVoteOption(optionName, commandString);
		parseChoiceList(list, parser);
		return true;
	}
	else if (!strHelpers::icmp(optionType, "text")) {
		voteOptions.addTextOption(optionName, commandString);
	}
	else if (!strHelpers::icmp(optionType, "integer")) {
		voteOptions.addIntegerOption(optionName, commandString);
	}
	else if (!strHelpers::icmp(optionType, "float")) {
		voteOptions.addFloatOption(optionName, commandString);
	}
	else if (!strHelpers::icmp(optionType, "client")) {
		voteOptions.addClientOption(optionName, commandString, true);
	}
	else if (!strHelpers::icmp(optionType, "clientnotself")) {
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
		if (!strHelpers::icmp(token, "{"))
		{
			while (parser.TokenAvailable(true))
			{
				// get the choice name
				token = parser.GetToken(true);
				if (!strHelpers::icmp(token, "}"))
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
