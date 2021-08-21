#include <MOHPC/Assets/Formats/TIKI.h>

using namespace MOHPC;

/**
 * Animation flags.
 */
namespace MOHPC::TAF
{
	const tikiFlag_t RANDOM = (1 << 0);
	const tikiFlag_t NOREPEAT = (1<< 1);
	const tikiFlag_t DONTREPEAT = (RANDOM | NOREPEAT);
	const tikiFlag_t AUTOSTEPS_DOG = (1 << 2);
	const tikiFlag_t DEFAULT_ANGLES = (1 << 3);
	const tikiFlag_t NOTIMECHECK = (1 << 4);
	const tikiFlag_t AUTOSTEPS_WALK = (NOTIMECHECK | AUTOSTEPS_DOG);
	const tikiFlag_t AUTOSTEPS_RUN = (NOTIMECHECK | AUTOSTEPS_DOG | DEFAULT_ANGLES);
	const tikiFlag_t DELTADRIVEN = (1 << 5);
}

namespace MOHPC::TF
{
	const unsigned long LAST = -5;
	const unsigned long END = -4;
	const unsigned long ENTRY = -3;
	const unsigned long EXIT = -2;
	const unsigned long EVERY = -1;
	const unsigned long FIRST = 0;
}

namespace MOHPC::TS
{
	const tikiFlag_t SKIN1 = (1 << 0);
	const tikiFlag_t SKIN2 = (1 << 1);
	const tikiFlag_t SKIN3 = (SKIN1 | SKIN2);
	const tikiFlag_t NODRAW = (1 << 2);
	const tikiFlag_t CROSSFADE = (1 << 6);
	const tikiFlag_t NODAMAGE = (1 << 7);
	const tikiFlag_t NOMIPMAPS = (1 << 8);
	const tikiFlag_t NOPICMIP = (1 << 9);
}

const unsigned long MOHPC::TIKI_FRAME_MAXFRAMERATE = 60;
const char MOHPC::TIKI_HEADER[] = "TIKI";
