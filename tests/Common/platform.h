#pragma once

#include <cstdint>
#include <cstddef>

void initConsole();
void sleepTime(size_t ms);
bool isKeyboardHit();
int getConsoleChar();

void setTimerResolution(uint64_t resolution);
void restoreTimerSolution(uint64_t resolution);
