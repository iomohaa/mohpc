#pragma once

namespace MOHPC
{
	class Huff;

	namespace MessageCodecs
	{
		namespace Compression
		{
			extern ConstHuff<513> huff;
			//extern Huff huff;
		}

		namespace Decompression
		{
			extern ConstHuff<513> huff;
			//extern Huff huff;
		}
	}
}
