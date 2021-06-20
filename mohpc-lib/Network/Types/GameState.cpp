#include <MOHPC/Network/Types/GameState.h>
#include <MOHPC/Network/InfoTypes.h>

using namespace MOHPC;
using namespace MOHPC::Network;

BaseLines::BaseLines()
	: entityBaselines(nullptr)
	, maxBaselines(0)
{
}

BaseLines::BaseLines(baselineNum_t maxBaselinesValue)
	: maxBaselines(maxBaselinesValue)
{
	if (maxBaselines)
	{
		// assign an array of baselines
		entityBaselines = new entityState_t[maxBaselines];
	}
	else
	{
		// no entities
		entityBaselines = nullptr;
	}
}

BaseLines::~BaseLines()
{
	if (entityBaselines)
	{
		delete[] entityBaselines;
	}
}

const entityState_t& BaseLines::getBaseline(baselineNum_t index) const
{
	if (index >= getMaxBaselines()) {
		throw GameStateErrors::BaselineOutOfRangeException(index);
	}

	return entityBaselines[index];
}

entityState_t& BaseLines::getBaseline(baselineNum_t index)
{
	if (index >= getMaxBaselines()) {
		throw GameStateErrors::BaselineOutOfRangeException(index);
	}

	return entityBaselines[index];
}

baselineNum_t BaseLines::getMaxBaselines() const
{
	return maxBaselines;
}

void BaseLines::reset()
{
	for (size_t i = 0; i < maxBaselines; ++i)
	{
		entityBaselines[i] = entityState_t();
	}
}

gameState_t::gameState_t()
{
}

gameState_t::gameState_t(const size_t numConfigStrings, const size_t maxChars, const baselineNum_t maxBaselines)
	: csMan(numConfigStrings, maxChars)
	, entityBaselines(maxBaselines)
{
}

gameState_t::~gameState_t()
{
}

ConfigStringManager& gameState_t::getConfigstringManager()
{
	return csMan;
}

const ConfigStringManager& gameState_t::getConfigstringManager() const
{
	return csMan;
}

BaseLines& gameState_t::getEntityBaselines()
{
	return entityBaselines;
}

const BaseLines& gameState_t::getEntityBaselines() const
{
	return entityBaselines;
}

void gameState_t::reset()
{
	csMan.reset();
	entityBaselines.reset();
}

GameStateErrors::BaselineOutOfRangeException::BaselineOutOfRangeException(baselineNum_t inBaselineNum)
	: baselineNum(inBaselineNum)
{}

baselineNum_t GameStateErrors::BaselineOutOfRangeException::getBaselineNum() const
{
	return baselineNum;
}

str GameStateErrors::BaselineOutOfRangeException::what() const
{
	return str((int)getBaselineNum());
}
