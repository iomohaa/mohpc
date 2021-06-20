#pragma once

namespace MOHPC
{
	class ReferenceCounter
	{
	private:
		size_t sharedCount;
		size_t weakCount;

	public:
		ReferenceCounter() noexcept
			: sharedCount(0)
			, weakCount(0)
		{
		}

		void AddSharedRef() noexcept
		{
			++sharedCount;
		}

		bool ReleaseSharedRef() noexcept
		{
			--sharedCount;
			if (!weakCount && !sharedCount) delete this;
			return !sharedCount;
		}

		void AddWeakRef() noexcept
		{
			++weakCount;
		}

		bool ReleaseWeakRef() noexcept
		{
			--weakCount;
			if (!weakCount && !sharedCount) delete this;
			return !weakCount;
		}

		size_t Count() const noexcept
		{
			return weakCount + sharedCount;
		}
	};
}
