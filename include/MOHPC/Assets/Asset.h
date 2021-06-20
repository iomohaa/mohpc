#pragma once

#include "AssetBase.h"
#include "../Common/Object.h"
#include "../Common/str.h"

namespace MOHPC
{
	class Asset : public AssetObject
	{
	friend AssetManager;

	private:
		str Filename;
		class Hasher* Hash;

	public:
		virtual ~Asset();

		/** Returns the filename of the asset. */
		MOHPC_ASSETS_EXPORTS const str& GetFilename() const;

		/** Returns the 20-bit hash for this asset. */
		MOHPC_ASSETS_EXPORTS void HashGetHash(uint8_t* Destination) const;

	protected:
		MOHPC_ASSETS_EXPORTS Asset();

	private:
		// Called by the asset manager
		void Init(const char *Filename);
		void HashFinalize();

	protected:
		void HashUpdate(const uint8_t* Data, uint64_t Length);
		void HashCopy(const Asset* A);

		virtual void Load() = 0;
	};
}
