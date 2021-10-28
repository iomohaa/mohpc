#pragma once

#include "../NetGlobal.h"
#include "../../Utility/Function.h"
#include "../../Utility/HandlerList.h"
#include "../../Utility/Misc/MSG/MSG.h"
#include "../../Common/str.h"

#include "../Parsing/String.h"
#include "../Exception.h"

#include <cstdint>

namespace MOHPC
{
class MSG;

namespace Network
{
	class IReliableSequence;

	namespace DownloadHandlers
	{
		/**
		 * Called to start a download.
		 *
		 * @param name The name of the download (can use the name as a file name).
		 * @param interrupt Reference to a boolean value that indicates whether or not should the download be interrupted.
		 */
		struct StartDownload : public HandlerNotifyBase<void(const char* name, bool& interrupt)> {};

		/**
		 * Called when a chunk of data was read.
		 *
		 * @param data Chunk of data.
		 * @param size Size of the data chunk.
		 * @param interrupt Reference to a boolean value that indicates whether or not should the download be interrupted.
		 */
		struct Receive : public HandlerNotifyBase<void(const uint8_t* data, size_t size, bool& interrupt)> {};
	}

	/**
	 * Manage downloading from remote server.
	 */
	class DownloadManager
	{
	public:
		struct HandlerList
		{
			FunctionList<DownloadHandlers::StartDownload> startDownloadHandler;
			FunctionList<DownloadHandlers::Receive> receiveHandler;
		};

	public:
		DownloadManager();

		void setReliableSequence(IReliableSequence* sequencePtr);

		MOHPC_NET_EXPORTS HandlerList& handlers();
		MOHPC_NET_EXPORTS const HandlerList& handlers() const;

		/**
		 * Called to process a download message
		 */
		void processDownload(MSG& msg, const Parsing::IString& stringParser);

		/**
		 * Start downloading from server.
		 *
		 * @param downloadName Name of the download.
		 */
		void startDownload(const char* downloadNameValue);

		/**
		 * Cancel downloading.
		 */
		void cancelDownload();

	private:
		void clearDownload();
		void downloadsComplete(IReliableSequence* sequence);
		bool receive(const uint8_t* data, const size_t size);

	private:
		HandlerList handlerList;
		IReliableSequence* sequence;
		size_t downloadSize;
		str downloadName;
		uint32_t downloadBlock;
		bool downloadRequested;
	};

	namespace ClientDownloadError
	{
		class Base : public NetworkException
		{};

		/**
		 * Server error while downloading.
		 */
		class DownloadException : public Base
		{
		public:
			DownloadException(StringMessage&& inError);

			/** Return the error the server sent. */
			MOHPC_NET_EXPORTS const char* getError() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			StringMessage error;
		};

		/**
		 * Bad download size (received more than the manager can handle).
		 */
		class DownloadSizeException : public Base
		{
		public:
			DownloadSizeException(uint16_t inSize);

			MOHPC_NET_EXPORTS uint16_t getSize() const;

		private:
			uint16_t size;
		};

		/**
		 * Unexpected download block. The current block is incremented after each chunk
		 * and the client expects the server to have the same incoming block.
		 */
		class BadDownloadBlockException : public Base
		{
		public:
			BadDownloadBlockException(uint16_t block, uint16_t expectedBlock);

			MOHPC_NET_EXPORTS uint16_t getBlock() const noexcept;
			MOHPC_NET_EXPORTS uint16_t getExpectedBlock() const noexcept;

		private:
			uint16_t block;
			uint16_t expectedBlock;

		};

		/**
		 * The server sent a download that wasn't requested by the client.
		 */
		class MOHPC_NET_EXPORTS UnexpectedDownloadException : public NetworkException
		{
		};
	}
}

MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::DownloadHandlers::StartDownload);
MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::DownloadHandlers::Receive);
}