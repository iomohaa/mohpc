#include <MOHPC/Network/Parsing/Environment.h>
#include <MOHPC/Network/Types/Environment.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

class Environment8 : public Parsing::IEnvironment
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	void parseEnvironment(const char* string, environment_t& env) const override
	{
		int tmp = 0;
		sscanf(
			string,
			"%d %f %f %f %f",
			&tmp,
			&env.farplaneDistance,
			&env.farplaneColor[0],
			&env.farplaneColor[1],
			&env.farplaneColor[2]
		);
		// Parse bool
		env.farplaneCull = tmp;
	}

	void parseSky(const char* string, environment_t& env) const override
	{
		int tmp = 0;
		sscanf(string, "%f %d", &env.skyAlpha, &tmp);
		// Parse bool
		env.skyPortal = tmp;
	}

	void writeEnvironment(const environment_t& env, str& out) const override
	{
		out = std::to_string(env.farplaneCull);
		out += " " + std::to_string(env.farplaneDistance);
		out += " " + std::to_string(env.farplaneColor[0]);
		out += " " + std::to_string(env.farplaneColor[1]);
		out += " " + std::to_string(env.farplaneColor[2]);
	}

	void writeSky(const environment_t& env, str& out) const override
	{
		out = std::to_string(env.skyAlpha);
		out += " " + std::to_string(env.skyPortal);
	}
};

class Environment17 : public Environment8
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	void parseEnvironment(const char* string, environment_t& env) const override
	{
		int tmp = 0, tmp2 = 0;
		sscanf(
			string,
			"%d %f %f %f %f %f %f %f %d %f %f %f %f",
			&tmp,
			&env.farplaneDistance,
			&env.farplaneBias,
			&env.skyboxFarplane,
			&env.skyboxSpeed,
			&env.farplaneColor[0],
			&env.farplaneColor[1],
			&env.farplaneColor[2],
			&tmp2,
			&env.farclipOverride,
			&env.farplaneColorOverride[0],
			&env.farplaneColorOverride[1],
			&env.farplaneColorOverride[2]
		);
		// Parse bool
		env.farplaneCull = tmp;
		env.renderTerrain = tmp2;
	}

	void writeEnvironment(const environment_t& env, str& out) const override
	{
		out = std::to_string(env.farplaneCull);
		out += " " + std::to_string(env.farplaneDistance);
		out += " " + std::to_string(env.farplaneBias);
		out += " " + std::to_string(env.skyboxFarplane);
		out += " " + std::to_string(env.skyboxSpeed);
		out += " " + std::to_string(env.farplaneColor[0]);
		out += " " + std::to_string(env.farplaneColor[1]);
		out += " " + std::to_string(env.farplaneColor[2]);
		out += " " + std::to_string(env.renderTerrain);
		out += " " + std::to_string(env.farclipOverride);
		out += " " + std::to_string(env.farplaneColorOverride[0]);
		out += " " + std::to_string(env.farplaneColorOverride[1]);
		out += " " + std::to_string(env.farplaneColorOverride[2]);
	}
};

Environment8 env8;
Environment17 env17;
