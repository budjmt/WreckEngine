#include "TextEntity.h"

TextEntity::TextEntity(const std::string _message, const std::string& _font, const uint32_t height, const uint32_t width)
	: Entity(), font(Text::loadWinFont(_font, height, width)), message(_message) {}

void TextEntity::setFont(const std::string& _font, const uint32_t height, const uint32_t width) { font = Text::loadWinFont(_font, height, width); }