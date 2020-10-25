#pragma once

#include "Global.h"
#include "Class.h"
#include "Common/str.h"

namespace MOHPC
{
	typedef SafePtr<class Asset> AssetPtr;

	class Asset : public Class
	{
	friend AssetManager;

	private:
		str Filename;
		class Hasher* Hash;

	public:
		virtual ~Asset();

		/** Returns the filename of the asset. */
		MOHPC_EXPORTS const str& GetFilename() const;

		/** Returns the 20-bit hash for this asset. */
		MOHPC_EXPORTS void HashGetHash(uint8_t* Destination) const;

	protected:
		MOHPC_EXPORTS Asset();

	private:
		// Called by the asset manager
		void Init(const char *Filename);
		void HashFinalize();

	protected:
		void HashUpdate(const uint8_t* Data, std::streamsize Length);
		void HashCopy(const Asset* A);

		virtual bool Load();
	};
}
