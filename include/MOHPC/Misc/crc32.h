#pragma once

#include "../Global.h"
#include <stdint.h>
#include <stdlib.h>

MOHPC_EXPORTS uint32_t crc32_hash(const void *buf, size_t size, int base);
