#include <MOHPC/Network/Client/DownloadManager.h>
#include <MOHPC/Network/Types/Reliable.h>
#include <MOHPC/Utility/Misc/Escape.h>

#include <MOHPC/Common/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;

static constexpr char MOHPC_LOG_NAMESPACE[] = "client_download";

static constexpr size_t MAX_DOWNLOAD_SIZE = 49152;
static constexpr char startDownloadCommand[] = "download";
static constexpr char nextDownloadCommand[] = "nextdl";

DownloadManager::DownloadManager()
	: downloadSize(0)
	, downloadBlock(0)
	, downloadRequested(false)
{}

void DownloadManager::processDownload(MSG& msg, const Parsing::IString& stringParser)
{
	if (!downloadRequested)
	{
		// not requested
		sequence->addCommand("stopdl");
		throw ClientDownloadError::UnexpectedDownloadException();
	}

	uint8_t data[MAX_DOWNLOAD_SIZE];

	const uint16_t block = msg.ReadUShort();
	if (!block)
	{
		// block zero = file size
		uint32_t fileSize = msg.ReadInteger();

		if (!fileSize || fileSize == -1)
		{
			// not a valid file size
			cancelDownload();
			// throw the error
			throw ClientDownloadError::DownloadException(stringParser.readString(msg));
		}

		downloadBlock = 0;

		MOHPC_LOG(Debug, "downloading file of size %d", fileSize);
	}

	const uint16_t size = msg.ReadUShort();

	if (size > sizeof(data))
	{
		// invalid size
		throw ClientDownloadError::DownloadSizeException(size);
	}

	if (downloadBlock != block)
	{
		// unexpected block
		throw ClientDownloadError::BadDownloadBlockException(block, downloadBlock);
	}

	if (size > 0)
	{
		msg.ReadData(data, size);

		if (!receive(data, size))
		{
			cancelDownload();
			return;
		}

		MOHPC_LOG(Debug, "downloaded block %d size %d", block, size);

		// tell the server to continue downloading
		sequence->addCommand((nextDownloadCommand + std::to_string(downloadBlock)).c_str());

		downloadSize += size;
	}
	else
	{
		// a size of 0 means EOF
		clearDownload();
		downloadsComplete(sequence);
	}

	downloadBlock++;
}

bool DownloadManager::receive(const uint8_t* data, const size_t size)
{
	// let the callee do whatever with the data and possibly cancel
	bool interrupt = false;
	handlers().receiveHandler.broadcast(data, size, interrupt);

	return !interrupt;
}

void DownloadManager::startDownload(const char* downloadNameValue)
{
	bool interrupt = false;
	// notify about the new download
	handlers().startDownloadHandler.broadcast(downloadNameValue, interrupt);

	if (interrupt)
	{
		// one of the handler canceled the download
		return;
	}

	// send the server command
	const str escapedName = EscapeString(downloadNameValue);
	sequence->addCommand((std::string(startDownloadCommand) + '"' + escapedName + '"').c_str());
	downloadRequested = true;
	downloadName = downloadNameValue;
}

void DownloadManager::cancelDownload()
{
	clearDownload();
	sequence->addCommand("stopdl");
}

void DownloadManager::clearDownload()
{
	downloadName.clear();
	downloadRequested = false;
}

void DownloadManager::downloadsComplete(IReliableSequence* sequence)
{
	// inform that download has stopped
	sequence->addCommand("donedl");
}

DownloadManager::HandlerList& DownloadManager::handlers()
{
	return handlerList;
}

const DownloadManager::HandlerList& DownloadManager::handlers() const
{
	return handlerList;
}

void DownloadManager::setReliableSequence(IReliableSequence* sequencePtr)
{
	sequence = sequencePtr;
}

ClientDownloadError::DownloadException::DownloadException(StringMessage&& inError)
	: error(std::move(inError))
{}

ClientDownloadError::DownloadSizeException::DownloadSizeException(uint16_t inSize)
	: size(inSize)
{}

const char* ClientDownloadError::DownloadException::getError() const
{
	return error;
}
str ClientDownloadError::DownloadException::what() const
{
	return str(getError());
}

uint16_t ClientDownloadError::DownloadSizeException::getSize() const
{
	return size;
}

ClientDownloadError::BadDownloadBlockException::BadDownloadBlockException(uint16_t inBlock, uint16_t inExpectedBlock)
	: block(inBlock)
	, expectedBlock(inExpectedBlock)
{
}

uint16_t ClientDownloadError::BadDownloadBlockException::getBlock() const noexcept
{
	return block;
}

uint16_t ClientDownloadError::BadDownloadBlockException::getExpectedBlock() const noexcept
{
	return expectedBlock;
}

