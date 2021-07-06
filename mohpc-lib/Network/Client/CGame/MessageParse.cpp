#include <MOHPC/Network/Client/CGame/MessageParse.h>
#include <MOHPC/Network/Client/CGame/Hud.h>
#include <MOHPC/Network/Client/CGame/Effect.h>
#include <MOHPC/Common/Vector.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

#include <MOHPC/Network/Client/CGame/MessageInterfaces/IBullet.h>
#include <MOHPC/Network/Client/CGame/MessageInterfaces/IEffect.h>
#include <MOHPC/Network/Client/CGame/MessageInterfaces/IHUD.h>
#include <MOHPC/Network/Client/CGame/MessageInterfaces/IImpact.h>
#include <MOHPC/Network/Client/CGame/MessageInterfaces/IEvent.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

class CommonMessageHandler
{
public:
	CommonMessageHandler(MSG& msgRef, const MessageInterfaces& interfacesRef)
		: msg(msgRef)
		, interfaces(interfacesRef)
	{

	}

	void impactMelee()
	{
		MsgTypesHelper msgHelper(msg);

		const Vector vecStart = msgHelper.ReadVectorCoord();
		const Vector vecEnd = msgHelper.ReadVectorCoord();

		interfaces.impact->MeleeImpact(vecStart, vecEnd);
	}

	void debrisCrate()
	{
		MsgTypesHelper msgHelper(msg);

		const Vector vecStart = msgHelper.ReadVectorCoord();
		const uint8_t numDebris = msg.ReadByte();

		interfaces.effect->SpawnDebris(debrisType_e::crate, vecStart, numDebris);
	}

	void debrisWindow()
	{
		MsgTypesHelper msgHelper(msg);

		const Vector vecStart = msgHelper.ReadVectorCoord();
		const uint8_t numDebris = msg.ReadByte();

		interfaces.effect->SpawnDebris(debrisType_e::window, vecStart, numDebris);
	}

	void huddrawShader()
	{
		const uint8_t index = msg.ReadByte();
		const StringMessage strVal = msg.ReadString();

		interfaces.hud->SetShader(index, strVal.c_str());
	}

	void huddrawAlign()
	{
		const uint8_t index = msg.ReadByte();
		const uint8_t hAlign = msg.ReadNumber<uint8_t>(2);
		const uint8_t vAlign = msg.ReadNumber<uint8_t>(2);

		interfaces.hud->SetAlignment(index, horizontalAlign_e(hAlign), verticalAlign_e(vAlign));
	}

	void huddrawRect()
	{
		const uint8_t index = msg.ReadByte();
		const uint16_t x = msg.ReadUShort();
		const uint16_t y = msg.ReadUShort();
		const uint16_t width = msg.ReadUShort();
		const uint16_t height = msg.ReadUShort();

		interfaces.hud->SetRect(index, x, y, width, height);
	}

	void huddrawVirtualScreen()
	{
		const uint8_t index = msg.ReadByte();
		const bool virtualScreen = msg.ReadBool();

		interfaces.hud->SetVirtualScreen(index, virtualScreen);
	}

	void huddrawColor()
	{
		const uint8_t index = msg.ReadByte();
		// Divide by 255 to get float color
		const Vector col =
		{
			(float)msg.ReadByte() / 255.f,
			(float)msg.ReadByte() / 255.f,
			(float)msg.ReadByte() / 255.f
		};

		interfaces.hud->SetColor(index, col);
	}

	void huddrawAlpha()
	{
		const uint8_t index = msg.ReadByte();
		const float alpha = (float)msg.ReadByte() / 255.f;

		interfaces.hud->SetAlpha(index, alpha);
	}

	void huddrawString()
	{
		const uint8_t index = msg.ReadByte();
		const StringMessage strVal = msg.ReadString();

		interfaces.hud->SetString(index, strVal.c_str());
	}

	void huddrawFont()
	{
		const uint8_t index = msg.ReadByte();
		const StringMessage strVal = msg.ReadString();

		interfaces.hud->SetFont(index, strVal.c_str());
	}

	void notifyHit()
	{
		interfaces.event->Hit();
	}

	void notifyKill()
	{
		interfaces.event->GotKill();
	}

	void playSoundEntity()
	{
		MsgTypesHelper msgHelper(msg);

		const Vector vecStart = msgHelper.ReadVectorCoord();

		const bool temp = msg.ReadBool();
		const uint8_t index = msg.ReadNumber<uint8_t>(6);
		const StringMessage strVal = msg.ReadString();

		interfaces.event->PlayVoice(vecStart, temp, index, strVal.c_str());
	}

private:
	MSG& msg;
	const MessageInterfaces& interfaces;
};

class MessageParseBase : public IMessageParser
{
public:
	void parseGameMessage(MSG& msg, const MessageInterfaces& interfaces) const override
	{
		// loop until there is no message
		bool hasMessage;
		do
		{
			uint8_t msgType = msg.ReadNumber<uint8_t>(6);

			handleCGMessage(msg, interfaces, msgType);
			hasMessage = msg.ReadBool();
		} while (hasMessage);
	}

	const char* getEffectModel(uint32_t id) const
	{
		return getEffectName(getEffectId(id));
	}
};

class MessageParse8 : public MessageParseBase
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	void handleCGMessage(MSG& msg, const MessageInterfaces& interfaces, uint32_t msgId) const override
	{
		StringMessage strVal;
		MsgTypesHelper msgHelper(msg);
		CommonMessageHandler commonMessage(msg, interfaces);
		uint32_t temp;
		uint32_t count;
		uint32_t large = false;
		Vector vecTmp;
		Vector vecStart, vecEnd;
		Vector vecArray[64];

		const cgmessage_e msgType((cgmessage_e)msgId);
		switch (msgType)
		{
		case cgmessage_e::bullet1:
			vecTmp = msgHelper.ReadVectorCoord();
		case cgmessage_e::bullet2:
		case cgmessage_e::bullet5:
		{
			vecStart = msgHelper.ReadVectorCoord();
			vecTmp = vecStart;
			vecArray[0] = msgHelper.ReadVectorCoord();
			large = msg.ReadBool();

			if (msgType == cgmessage_e::bullet1 || msgType == cgmessage_e::bullet2)
			{
				interfaces.bullet->CreateBulletTracer(
					vecStart,
					vecTmp,
					vecArray[0],
					1u,
					large,
					msgType == cgmessage_e::bullet1 ? 1u : 0u,
					1.f
				);
			}
			else {
				interfaces.bullet->CreateBubbleTrail(vecStart, vecEnd, large, 1.f);
			}
			break;
		}
		case cgmessage_e::bullet3:
		{
			vecTmp = msgHelper.ReadVectorCoord();
			temp = msg.ReadNumber<uint32_t>(6);
		}
		case cgmessage_e::bullet4:
		{
			vecStart = msgHelper.ReadVectorCoord();
			large = msg.ReadBool();
			count = msg.ReadNumber<uint32_t>(6);

			for (size_t i = 0; i < count; ++i) {
				vecArray[i] = msgHelper.ReadVectorCoord();
			}

			interfaces.bullet->CreateBulletTracer(
				vecTmp,
				vecTmp,
				vecArray[0],
				count,
				large,
				temp,
				1.f
			);
			break;
		}
		case cgmessage_e::impact1:
		case cgmessage_e::impact2:
		case cgmessage_e::impact3:
		case cgmessage_e::impact4:
		case cgmessage_e::impact5:
			vecStart = msgHelper.ReadVectorCoord();
			vecEnd = msgHelper.ReadDir();
			large = msg.ReadBool();

			interfaces.impact->Impact(
				vecStart,
				vecEnd,
				large
			);
			break;
		case cgmessage_e::impact_melee:
			commonMessage.impactMelee();
			break;
		case cgmessage_e::explo1:
		case cgmessage_e::explo2:
		{
			uint32_t effectId = msgId == 12 || msgId != 13 ? 63 : 64;
			vecStart = msgHelper.ReadVectorCoord();

			interfaces.impact->Explosion(
				vecStart,
				getEffectModel(effectId)
			);
			break;
		}
		case cgmessage_e::effect1:
		case cgmessage_e::effect2:
		case cgmessage_e::effect3:
		case cgmessage_e::effect4:
		case cgmessage_e::effect5:
		case cgmessage_e::effect6:
		case cgmessage_e::effect7:
		case cgmessage_e::effect8:
			vecStart = msgHelper.ReadVectorCoord();
			vecEnd = msgHelper.ReadDir();

			interfaces.effect->SpawnEffect(
				vecStart,
				vecEnd,
				getEffectModel(msgId + 67)
			);
			break;
		case cgmessage_e::debris_crate:
			commonMessage.debrisCrate();
			break;
		case cgmessage_e::debris_window:
			commonMessage.debrisWindow();
			break;
		case cgmessage_e::tracer_visible:
			vecTmp = msgHelper.ReadVectorCoord();
			vecStart = msgHelper.ReadVectorCoord();
			vecArray[0] = msgHelper.ReadVectorCoord();
			large = msg.ReadBool();

			interfaces.bullet->CreateBulletTracer(
				vecTmp,
				vecStart,
				vecArray[0],
				1u,
				large,
				1u,
				1.f
			);
			break;
		case cgmessage_e::tracer_hidden:
			vecTmp = vec_zero;
			vecStart = msgHelper.ReadVectorCoord();
			vecArray[0] = msgHelper.ReadVectorCoord();
			large = msg.ReadBool();

			interfaces.bullet->CreateBulletTracer(
				vecTmp,
				vecStart,
				vecArray[0],
				1u,
				large,
				0u,
				1.f
			);
			break;
		case cgmessage_e::huddraw_shader:
			commonMessage.huddrawShader();
			break;
		case cgmessage_e::huddraw_align:
			commonMessage.huddrawAlign();
			break;
		case cgmessage_e::huddraw_rect:
			commonMessage.huddrawRect();
			break;
		case cgmessage_e::huddraw_virtualscreen:
			commonMessage.huddrawVirtualScreen();
			break;
		case cgmessage_e::huddraw_color:
			commonMessage.huddrawColor();
			break;
		case cgmessage_e::huddraw_alpha:
			commonMessage.huddrawAlpha();
			break;
		case cgmessage_e::huddraw_string:
			commonMessage.huddrawString();
			break;
		case cgmessage_e::huddraw_font:
			commonMessage.huddrawFont();
			break;
		case cgmessage_e::notify_hit:
			commonMessage.notifyHit();
			break;
		case cgmessage_e::notify_kill:
			commonMessage.notifyKill();
			break;
		case cgmessage_e::playsound_entity:
			commonMessage.playSoundEntity();
			break;
		default:
			break;
		}
	}

	effects_e getEffectId(uint32_t effectId) const override
	{
		static effects_e effectList[] =
		{
		effects_e::bh_paper_lite,
		effects_e::bh_paper_hard,
		effects_e::bh_wood_lite,
		effects_e::bh_wood_hard,
		effects_e::bh_metal_lite,
		effects_e::bh_metal_hard,
		effects_e::bh_stone_lite,
		effects_e::bh_stone_hard,
		effects_e::bh_dirt_lite,
		effects_e::bh_dirt_hard,
		// 10
		effects_e::bh_metal_lite,
		effects_e::bh_metal_hard,
		effects_e::bh_grass_lite,
		effects_e::bh_grass_hard,
		effects_e::bh_mud_lite,
		// 15
		effects_e::bh_mud_hard,
		effects_e::bh_water_lite,
		effects_e::bh_water_hard,
		effects_e::bh_glass_lite,
		effects_e::bh_glass_hard,
		// 20
		effects_e::bh_stone_lite,
		effects_e::bh_stone_hard,
		effects_e::bh_sand_lite,
		effects_e::bh_sand_hard,
		effects_e::bh_foliage_lite,
		// 25
		effects_e::bh_foliage_hard,
		effects_e::bh_snow_lite,
		effects_e::bh_snow_hard,
		effects_e::bh_carpet_lite,
		effects_e::bh_carpet_hard,
		// 30
		effects_e::bh_human_uniform_lite,
		effects_e::bh_human_uniform_hard,
		effects_e::water_trail_bubble,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 35
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 40
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 45
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 50
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 55
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 60
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::grenexp_base,
		effects_e::bazookaexp_base,
		// 65
		effects_e::grenexp_paper,
		effects_e::grenexp_wood,
		effects_e::grenexp_metal,
		effects_e::grenexp_stone,
		effects_e::grenexp_dirt,
		// 70
		effects_e::grenexp_metal,
		effects_e::grenexp_grass,
		effects_e::grenexp_mud,
		effects_e::grenexp_water,
		effects_e::bh_stone_hard,
		// 75
		effects_e::grenexp_gravel,
		effects_e::grenexp_sand,
		effects_e::grenexp_foliage,
		effects_e::grenexp_snow,
		effects_e::grenexp_carpet,
		// 80
		effects_e::water_ripple_still,
		effects_e::water_ripple_moving,
		effects_e::barrel_oil_leak_big,
		effects_e::barrel_oil_leak_medium,
		effects_e::barrel_oil_leak_small,
		// 85
		effects_e::barrel_oil_leak_splat,
		effects_e::barrel_water_leak_big,
		effects_e::barrel_water_leak_medium,
		effects_e::barrel_water_leak_small,
		effects_e::barrel_water_leak_splat,
		// 90
		effects_e::fs_light_dust,
		effects_e::fs_heavy_dust,
		effects_e::fs_dirt,
		effects_e::fs_grass,
		effects_e::fs_mud,
		// 95
		effects_e::fs_puddle,
		effects_e::fs_sand,
		effects_e::fs_snow,
		effects_e::bh_stone_hard
		};

		static constexpr size_t numEffects = sizeof(effectList) / sizeof(effectList[0]);
		static_assert(numEffects == 99);

		if (effectId < numEffects) {
			return effectList[effectId];
		}

		return effects_e::bh_stone_hard;
	}

private:
	enum class cgmessage_e
	{
		bullet1 = 1,
		bullet2,
		bullet3,
		bullet4,
		bullet5,
		impact1,
		impact2,
		impact3,
		impact4,
		impact5,
		impact_melee,
		explo1,
		explo2,
		unk1,
		effect1,
		effect2,
		effect3,
		effect4,
		effect5,
		effect6,
		effect7,
		effect8,
		debris_crate,
		debris_window,
		tracer_visible,
		tracer_hidden,
		huddraw_shader,
		huddraw_align,
		huddraw_rect,
		huddraw_virtualscreen,
		huddraw_color,
		huddraw_alpha,
		huddraw_string,
		huddraw_font,
		notify_hit,
		notify_kill,
		playsound_entity,
	};
};

class MessageParse17 : public MessageParseBase
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	void handleCGMessage(MSG& msg, const MessageInterfaces& interfaces, uint32_t msgId) const override
	{
		StringMessage strVal;
		MsgTypesHelper msgHelper(msg);
		CommonMessageHandler commonMessage(msg, interfaces);
		uint32_t temp;
		uint32_t count;
		uint32_t effectId;
		uint32_t large = 0;
		Vector vecTmp;
		Vector vecStart, vecEnd;
		Vector vecArray[64];

		struct
		{
			float readBulletSize(MSG& msg)
			{
				static constexpr float MIN_BULLET_SIZE = 1.f / 512.f;

				const bool hasSize = msg.ReadBool();
				if (hasSize)
				{
					const uint16_t intSize = msg.ReadNumber<uint16_t>(10);

					float bulletSize = (float)intSize / 512.f;
					if (bulletSize < MIN_BULLET_SIZE) bulletSize = MIN_BULLET_SIZE;

					return bulletSize;
				}
				else {
					return 1.f;
				}
			}
		} utils;

		cgmessage_e msgType((cgmessage_e)msgId);
		switch (msgType)
		{
		case cgmessage_e::bullet1:
			vecTmp = msgHelper.ReadVectorCoord();
		case cgmessage_e::bullet2:
		case cgmessage_e::bullet5:
		{
			vecStart = msgHelper.ReadVectorCoord();
			vecTmp = vecStart;
			vecArray[0] = msgHelper.ReadVectorCoord();

			const uint8_t iLarge = msg.ReadNumber<uint8_t>(2);

			const float bulletSize = utils.readBulletSize(msg);

			if (msgType == cgmessage_e::bullet1 || msgType == cgmessage_e::bullet2)
			{
				interfaces.bullet->CreateBulletTracer(
					vecStart,
					vecTmp,
					vecArray[0],
					1u,
					large,
					msgType == cgmessage_e::bullet1 ? 1u : 0u,
					1.f
				);
			}
			else
			{
				interfaces.bullet->CreateBubbleTrail(
					vecStart,
					vecEnd,
					large,
					1.f
				);
			}
			break;
		}
		case cgmessage_e::bullet3:
		case cgmessage_e::bullet4:
		{
			if (msgType == cgmessage_e::bullet3)
			{
				vecTmp = msgHelper.ReadVectorCoord();
				temp = msg.ReadNumber<uint32_t>(6);
			}
			else {
				temp = 0;
			}

			vecStart = msgHelper.ReadVectorCoord();

			large = msg.ReadNumber<uint32_t>(2);

			const float bulletSize = utils.readBulletSize(msg);

			count = msg.ReadNumber<uint32_t>(6);

			for (size_t i = 0; i < count; ++i) {
				vecArray[i] = msgHelper.ReadVectorCoord();
			}

			if (count)
			{
				interfaces.bullet->CreateBulletTracer(
					vecStart,
					vecTmp,
					vecArray[0],
					count,
					large,
					temp,
					1.f
				);
			}
			break;
		}
		case cgmessage_e::impact1:
		case cgmessage_e::impact2:
		case cgmessage_e::impact3:
		case cgmessage_e::impact4:
		case cgmessage_e::impact5:
		{
			vecStart = msgHelper.ReadVectorCoord();
			vecEnd = msgHelper.ReadDir();

			const uint8_t iLarge = msg.ReadNumber<uint8_t>(2);

			interfaces.impact->Impact(
				vecStart,
				vecEnd,
				large
			);
			break;
		}
		case cgmessage_e::impact_melee:
			commonMessage.impactMelee();
			break;
		case cgmessage_e::explo1:
		case cgmessage_e::explo2:
		case cgmessage_e::explo3:
		case cgmessage_e::explo4:
			vecStart = msgHelper.ReadVectorCoord();

			switch (msgType)
			{
			case cgmessage_e::explo1:
				effectId = 63;
				break;
			case cgmessage_e::explo2:
				effectId = 64;
				break;
			case cgmessage_e::explo3:
				effectId = 65;
				break;
			case cgmessage_e::explo4:
				effectId = 66;
				break;
			default:
				effectId = 63;
			}

			interfaces.impact->Explosion(
				vecStart,
				getEffectModel(effectId)
			);
			break;
		case cgmessage_e::effect1:
		case cgmessage_e::effect2:
		case cgmessage_e::effect3:
		case cgmessage_e::effect4:
		case cgmessage_e::effect5:
		case cgmessage_e::effect6:
		case cgmessage_e::effect7:
		case cgmessage_e::effect8:
			vecStart = msgHelper.ReadVectorCoord();
			vecEnd = msgHelper.ReadDir();

			interfaces.effect->SpawnEffect(
				vecStart,
				vecEnd,
				getEffectModel(msgId + 75)
			);
			break;
		case cgmessage_e::debris_crate:
			commonMessage.debrisCrate();
			break;
		case cgmessage_e::debris_window:
			commonMessage.debrisWindow();
			break;
		case cgmessage_e::tracer_visible:
		{
			vecTmp = msgHelper.ReadVectorCoord();
			vecStart = msgHelper.ReadVectorCoord();
			vecArray[0] = msgHelper.ReadVectorCoord();

			const uint8_t iLarge = msg.ReadNumber<uint8_t>(2);
			const float bulletSize = utils.readBulletSize(msg);

			interfaces.bullet->CreateBulletTracer(
				vecTmp,
				vecStart,
				vecArray[0],
				1u,
				large,
				1u,
				1.f
			);
			break;
		}
		case cgmessage_e::tracer_hidden:
		{
			vecTmp = vec_zero;
			vecStart = msgHelper.ReadVectorCoord();
			vecArray[0] = msgHelper.ReadVectorCoord();

			const uint8_t iLarge = msg.ReadNumber<uint8_t>(2);
			const float bulletSize = utils.readBulletSize(msg);

			interfaces.bullet->CreateBulletTracer(
				vecTmp,
				vecStart,
				vecArray[0],
				1u,
				large,
				0u,
				1.f
			);
			break;
		}
		case cgmessage_e::huddraw_shader:
			commonMessage.huddrawShader();
			break;
		case cgmessage_e::huddraw_align:
			commonMessage.huddrawAlign();
			break;
		case cgmessage_e::huddraw_rect:
			commonMessage.huddrawRect();
			break;
		case cgmessage_e::huddraw_virtualscreen:
			commonMessage.huddrawVirtualScreen();
			break;
		case cgmessage_e::huddraw_color:
			commonMessage.huddrawColor();
			break;
		case cgmessage_e::huddraw_alpha:
			commonMessage.huddrawAlpha();
			break;
		case cgmessage_e::huddraw_string:
			commonMessage.huddrawString();
			break;
		case cgmessage_e::huddraw_font:
			commonMessage.huddrawFont();
			break;
		case cgmessage_e::notify_hit:
			commonMessage.notifyHit();
			break;
		case cgmessage_e::notify_kill:
			commonMessage.notifyKill();
			break;
		case cgmessage_e::playsound_entity:
			commonMessage.playSoundEntity();
			break;
		case cgmessage_e::effect9:
		{
			vecStart = msgHelper.ReadVectorCoord();
			vecEnd = msgHelper.ReadVectorCoord();

			const uint8_t val1 = msg.ReadByte();
			const uint8_t val2 = msg.ReadByte();
			// FIXME: not sure what it does
		}
		default:
			break;

		}
	}

	effects_e getEffectId(uint32_t effectId) const override
	{
		static effects_e effectList[] =
		{
		effects_e::bh_paper_lite,
		effects_e::bh_paper_hard,
		effects_e::bh_wood_lite,
		effects_e::bh_wood_hard,
		effects_e::bh_metal_lite,
		effects_e::bh_metal_hard,
		effects_e::bh_stone_lite,
		effects_e::bh_stone_hard,
		effects_e::bh_dirt_lite,
		effects_e::bh_dirt_hard,
		// 10
		effects_e::bh_metal_lite,
		effects_e::bh_metal_hard,
		effects_e::bh_grass_lite,
		effects_e::bh_grass_hard,
		effects_e::bh_mud_lite,
		// 15
		effects_e::bh_mud_hard,
		effects_e::bh_water_lite,
		effects_e::bh_water_hard,
		effects_e::bh_glass_lite,
		effects_e::bh_glass_hard,
		// 20
		effects_e::bh_stone_lite,
		effects_e::bh_stone_hard,
		effects_e::bh_sand_lite,
		effects_e::bh_sand_hard,
		effects_e::bh_foliage_lite,
		// 25
		effects_e::bh_foliage_hard,
		effects_e::bh_snow_lite,
		effects_e::bh_snow_hard,
		effects_e::bh_carpet_lite,
		effects_e::bh_carpet_hard,
		// 30
		effects_e::bh_human_uniform_lite,
		effects_e::bh_human_uniform_hard,
		effects_e::water_trail_bubble,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 35
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 40
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 45
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 50
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 55
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		// 60
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::bh_stone_hard,
		effects_e::grenexp_base,
		effects_e::bazookaexp_base,
		// 65
		effects_e::heavyshellexp_base,
		effects_e::tankexp_base,
		effects_e::grenexp_paper,
		effects_e::grenexp_wood,
		effects_e::grenexp_metal,
		// 70
		effects_e::grenexp_stone,
		effects_e::grenexp_dirt,
		effects_e::grenexp_metal,
		effects_e::grenexp_grass,
		effects_e::grenexp_mud,
		// 75
		effects_e::grenexp_water,
		effects_e::bh_stone_hard,
		effects_e::grenexp_gravel,
		effects_e::grenexp_sand,
		effects_e::grenexp_foliage,
		// 80
		effects_e::grenexp_snow,
		effects_e::grenexp_carpet,
		effects_e::heavyshellexp_dirt,
		effects_e::heavyshellexp_stone,
		effects_e::heavyshellexp_snow,
		// 85
		effects_e::tankexp_dirt,
		effects_e::tankexp_stone,
		effects_e::tankexp_snow,
		effects_e::bazookaexp_dirt,
		effects_e::bazookaexp_stone,
		// 90
		effects_e::bazookaexp_snow,
		effects_e::water_ripple_still,
		effects_e::water_ripple_moving,
		effects_e::barrel_oil_leak_big,
		effects_e::barrel_oil_leak_medium,
		// 95
		effects_e::barrel_oil_leak_small,
		effects_e::barrel_oil_leak_splat,
		effects_e::barrel_water_leak_big,
		effects_e::barrel_water_leak_medium,
		effects_e::barrel_water_leak_small,
		// 100
		effects_e::barrel_water_leak_splat,
		effects_e::fs_light_dust,
		effects_e::fs_heavy_dust,
		effects_e::fs_dirt,
		effects_e::fs_grass,
		// 105
		effects_e::fs_mud,
		effects_e::fs_puddle,
		effects_e::fs_sand,
		effects_e::fs_snow,
		effects_e::fx_fence_wood,
		// 110
		effects_e::bh_stone_hard,
		};

		static constexpr size_t numEffects = sizeof(effectList) / sizeof(effectList[0]);
		static_assert(numEffects == 111);

		if (effectId < numEffects) {
			return effectList[effectId];
		}

		return effects_e::bh_stone_hard;
	}

private:
	enum class cgmessage_e
	{
		bullet1 = 1,
		bullet2,
		bullet3,
		bullet4,
		bullet5,
		impact1,
		impact2,
		impact3,
		impact4,
		impact5,
		impact6,
		impact_melee,
		explo1,
		explo2,
		explo3,
		explo4,
		unk1,
		effect1,
		effect2,
		effect3,
		effect4,
		effect5,
		effect6,
		effect7,
		effect8,
		debris_crate,
		debris_window,
		tracer_visible,
		tracer_hidden,
		huddraw_shader,
		huddraw_align,
		huddraw_rect,
		huddraw_virtualscreen,
		huddraw_color,
		huddraw_alpha,
		huddraw_string,
		huddraw_font,
		notify_hit,
		notify_kill,
		playsound_entity,
		// FIXME: must analyze precisely what this one does
		effect9
	};
};

MessageParse8 messageParse8;
MessageParse17 messageParse17;
