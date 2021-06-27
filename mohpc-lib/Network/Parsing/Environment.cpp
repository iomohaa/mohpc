#include <MOHPC/Network/Parsing/Environment.h>
#include <MOHPC/Network/Types/Environment.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

class Environment8 : public Parsing::IEnvironment
{
public:
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
		out = str::printf(
			"%d %f %f %f %f",
			env.farplaneCull,
			env.farplaneDistance,
			env.farplaneColor[0],
			env.farplaneColor[1],
			env.farplaneColor[2]
		);
	}

	void writeSky(const environment_t& env, str& out) const override
	{
		out = str::printf("%f %d", env.skyAlpha, env.skyPortal);
	}
};

class Environment17 : public Environment8
{
public:
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
		out = str::printf(
			"%d %f %f %f %f %f %f %f %d %f %f %f %f",
			env.farplaneCull,
			env.farplaneDistance,
			env.farplaneBias,
			env.skyboxFarplane,
			env.skyboxSpeed,
			env.farplaneColor[0],
			env.farplaneColor[1],
			env.farplaneColor[2],
			env.renderTerrain,
			env.farclipOverride,
			env.farplaneColorOverride[0],
			env.farplaneColorOverride[1],
			env.farplaneColorOverride[2]
		);
	}
};
