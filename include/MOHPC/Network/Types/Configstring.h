#pragma once

#include "../NetGlobal.h"
#include "../Configstring.h"
#include "../Types.h"
#include "../ProtocolSingleton.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{
	/**
	 * The gameState struct keep tracks of config-strings.
	 */
	struct ConfigStringManager
	{
	public:
		/** Initialize an empty gameState, with no config-strings. */
		MOHPC_NET_EXPORTS ConfigStringManager();

		/**
		 * Initialize the class with a maximum number of config-strings and game state chars.
		 *
		 * @param numConfigStrings Number of config-strings to allocate.
		 * @param maxChars Maximum number of characters to allow for the entire config-strings buffer.
		 */
		MOHPC_NET_EXPORTS ConfigStringManager(const size_t numConfigStrings, const size_t maxChars);
		MOHPC_NET_EXPORTS ~ConfigStringManager();

		/**
		 * Return the config-string at the specified number.
		 *
		 * @param	num		Config string ID. Must be < MAX_CONFIGSTRINGS
		 *
		 * @return	The config-string. NULL if num is greater than MAX_CONFIGSTRINGS
		 */
		MOHPC_NET_EXPORTS const char* getConfigString(csNum_t num) const;

		/** Checked/faster version that doesn't check the number bounds. */
		const char* getConfigStringChecked(csNum_t num) const;

		/**
		 * Set the specified config-string value.
		 *
		 * @param	num				Config string ID. Must be an ID bellow the maximum number of config-strings.
		 * @param	configString	The value to put in.
		 * @param	sz				Length of the string.
		 */
		MOHPC_NET_EXPORTS void setConfigString(csNum_t num, const char* configString, size_t sz);
		MOHPC_NET_EXPORTS void setConfigString(csNum_t num, const char* configString);

		/** Return the maximum number of characters that can be written. */
		MOHPC_NET_EXPORTS size_t getMaxChars() const;

		/** Return the maximum number of config-strings that can be held. */
		MOHPC_NET_EXPORTS size_t getMaxConfigStrings() const;

		/** Return the number of characters. */
		MOHPC_NET_EXPORTS size_t getDataCount() const;

		/** Empty the config-string list. */
		MOHPC_NET_EXPORTS void reset();

	public:
		class iterator
		{
		public:
			MOHPC_NET_EXPORTS iterator(const ConfigStringManager& csManValue);
			MOHPC_NET_EXPORTS ~iterator();

			MOHPC_NET_EXPORTS iterator operator++(int);
			MOHPC_NET_EXPORTS iterator& operator++();
			MOHPC_NET_EXPORTS operator bool() const;

			MOHPC_NET_EXPORTS csNum_t getNumber() const;
			MOHPC_NET_EXPORTS const char* getConfigString() const;

		private:
			void iter();

		private:
			const ConfigStringManager& csMan;
			csNum_t currentNum;
		};

	public:
		size_t* stringOffsets;
		char* stringData;
		size_t maxChars;
		size_t maxConfigStrings;
		size_t highestConfigstring;
		size_t dataCount;
	};

	/**
	 * A config-string translator, used to convert known config-string values into protocol-specific values.
	 */
	class IConfigStringTranslator : public IProtocolSingleton<IConfigStringTranslator>
	{
	public:
		/** Translate a config-string into a version-specific config-string. */
		virtual csNum_t translateFromNet(uint32_t num) const = 0;
		virtual uint32_t translateToNet(csNum_t num) const = 0;
	};

	namespace ConfigstringErrors
	{
		class Base : public NetworkException {};

		/**
		 * Bad config-string number.
		 */
		class MaxConfigStringException : public Base
		{
		public:
			MaxConfigStringException(const char* inName, csNum_t inConfigStringNum);

			/** Return the name of the code that tried to access the config string. */
			MOHPC_NET_EXPORTS const char* getName() const;
			/** Return the config string number. */
			MOHPC_NET_EXPORTS csNum_t getConfigstringNum() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			const char* name;
			csNum_t configStringNum;
		};

		/**
		 * MAX_GAMESTATE_CHARS was reached while parsing a config-string.
		 */
		class MaxGameStateCharsException : public Base
		{
		public:
			MaxGameStateCharsException(size_t inStringLen);

			/** Return the length of the string that was passed. */
			MOHPC_NET_EXPORTS size_t GetStringLength() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			size_t stringLen;
		};
	}
}
}