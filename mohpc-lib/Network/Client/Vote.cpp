#include <MOHPC/Network/Client/Vote.h>
#include <MOHPC/Utilities/TokenParser.h>

using namespace MOHPC;
using namespace Network;

voteInfo_t::voteInfo_t()
	: voteTime(0)
	, numVotesYes(0)
	, numVotesNo(0)
	, numUndecidedVotes(0)
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

VoteOptionList::VoteOptionList(const str& inName, const str& inCommand, TokenParser& parser)
	: VoteOption(inName, inCommand)
{
	parseChoiceList(parser);
}

void VoteOptionList::createVoteOption(const str& choiceName, const str& voteString)
{
	new (choiceList) VoteListChoice(choiceName, voteString);
}

void VoteOptionList::parseChoiceList(TokenParser& parser)
{
	if (choiceList.NumObjects())
	{
		// already parsed
		return;
	}

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
					throw NoListChoiceVoteStringException(parser, getName());
				}

				const str choiceName = token;
				// get the vote string
				const char* voteString = parser.GetToken(false);

				// create a new vote option
				createVoteOption(choiceName, voteString);
			}
		}
		else
		{
			// empty list
			throw NoListChoiceVoteStringException(parser, getName());
		}
	}
	else
	{
		// nothing after
		throw NoListChoiceVoteStringException(parser, getName());
	}

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

void VoteOptions::parseVoteOptions(const char* options, size_t len)
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
			const bool wasParsed = parseOptionType(optionName, commandString, optionType, parser);
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

	noChoicesOptionsList.Shrink();
	listOptionsList.Shrink();
	textOptionsList.Shrink();
	intOptionsList.Shrink();
	floatOptionsList.Shrink();
	clientOptionsList.Shrink();
	clientNotSelfOptionsList.Shrink();
}

bool VoteOptions::parseOptionType(const str& optionName, const str& commandString, const char* optionType, TokenParser& parser)
{
	if (!str::icmp(optionType, "nochoices")) {
		new (noChoicesOptionsList) VoteOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "list")) {
		new (listOptionsList) VoteOptionList(optionName, commandString, parser);
		return true;
	}
	else if (!str::icmp(optionType, "text")) {
		new (textOptionsList) VoteOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "integer")) {
		new (intOptionsList) VoteOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "float")) {
		new (floatOptionsList) VoteOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "client")) {
		new (clientOptionsList) VoteOption(optionName, commandString);
	}
	else if (!str::icmp(optionType, "clientnotself")) {
		new (clientNotSelfOptionsList) VoteOption(optionName, commandString);
	}
	else {
		// unknown option type
		throw IllegalOptionTypeException(parser, optionName, optionType);
	}

	return false;

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
