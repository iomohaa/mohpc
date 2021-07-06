#include <MOHPC/Network/Client/CGame/Messages/HUD.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;
using namespace MOHPC::Network::CGame::Messages;

void HUDImpl::SetShader(uint8_t index, const char* name)
{
	setShaderHandler.broadcast(index, name);
}

void HUDImpl::SetAlignment(uint8_t index, horizontalAlign_e horizontalAlign, verticalAlign_e verticalAlign)
{
	setAlignmentHandler.broadcast(index, horizontalAlign, verticalAlign);
}

void HUDImpl::SetRect(uint8_t index, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	setRectHandler.broadcast(index, x, y, width, height);
}

void HUDImpl::SetVirtualScreen(uint8_t index, bool virtualScreen)
{
	setVirtualScreenHandler.broadcast(index, virtualScreen);
}

void HUDImpl::SetColor(uint8_t index, const Vector& color)
{
	setColorHandler.broadcast(index, color);
}

void HUDImpl::SetAlpha(uint8_t index, float alpha)
{
	setAlphaHandler.broadcast(index, alpha);
}

void HUDImpl::SetString(uint8_t index, const char* string)
{
	setStringHandler.broadcast(index, string);
}

void HUDImpl::SetFont(uint8_t index, const char* fontName)
{
	setFontHandler.broadcast(index, fontName);
}
