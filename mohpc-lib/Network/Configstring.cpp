#include <MOHPC/Network/Configstring.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CS;

csNum_t CS::MAX_CONFIGSTRINGS = 2736;
csNum_t CS::MAX_MODELS = 1024;
csNum_t CS::MAX_IMAGES = 64;
csNum_t CS::MAX_SOUNDS = 512;
csNum_t CS::MAX_OBJECTIVES = 20;
csNum_t CS::MAX_LIGHTSTYLES = 32;
csNum_t CS::MAX_WEAPONS = 64;
csNum_t CS::MAX_CLIENTS = 64;

csNum_t CS::SERVERINFO = 0;
csNum_t CS::SYSTEMINFO = 1;
csNum_t CS::RESERVED_CONFIGSTRINGS = 2;

csNum_t CS::MESSAGE = 2;
csNum_t CS::SAVENAME = 3;
csNum_t CS::MOTD = 4;
csNum_t CS::WARMUP = 5;

csNum_t CS::MUSIC = 6;
csNum_t CS::FOGINFO = 7;
csNum_t CS::SKYINFO = 8;

csNum_t CS::GAME_VERSION = 9;
csNum_t CS::LEVEL_START_TIME = 10;

csNum_t CS::CURRENT_OBJECTIVE = 11;

csNum_t CS::RAIN_DENSITY = 12;
csNum_t CS::RAIN_SPEED = 13;
csNum_t CS::RAIN_SPEEDVARY = 14;
csNum_t CS::RAIN_SLANT = 15;
csNum_t CS::RAIN_LENGTH = 16;
csNum_t CS::RAIN_MINDIST = 17;
csNum_t CS::RAIN_WIDTH = 18;
csNum_t CS::RAIN_SHADER = 19;
csNum_t CS::RAIN_NUMSHADERS = 20;

csNum_t CS::VOTE_TIME = 21;
csNum_t CS::VOTE_STRING = 22;
csNum_t CS::VOTES_YES = 23;
csNum_t CS::VOTES_NO = 24;
csNum_t CS::VOTES_UNDECIDED = 25;

csNum_t CS::MATCHEND = 26;

csNum_t CS::MODELS = 32;
csNum_t CS::OBJECTIVES = (MODELS + MAX_MODELS);
csNum_t CS::SOUNDS = (OBJECTIVES + MAX_OBJECTIVES);
csNum_t CS::IMAGES = (SOUNDS + MAX_SOUNDS);
csNum_t CS::LIGHTSTYLES = (IMAGES + MAX_IMAGES);

csNum_t CS::PLAYERS = (LIGHTSTYLES + MAX_LIGHTSTYLES);
csNum_t CS::WEAPONS = (PLAYERS + MAX_CLIENTS);
csNum_t CS::TEAMS = 1892;
csNum_t CS::GENERAL_STRINGS = 1893;
csNum_t CS::SPECTATORS = 1894;
csNum_t CS::ALLIES = 1895;
csNum_t CS::AXIS = 1896;

csNum_t CS::SOUNDTRACK = 1881;
