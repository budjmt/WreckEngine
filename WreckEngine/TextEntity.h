#pragma once

#include "Entity.h"
#include "Text.h"

class TextEntity : public Entity {
public:
	TextEntity(const std::string _message, const std::string& _font, Text::Justify _vertical, Text::Justify _horizontal, const uint32_t height, const uint32_t width = 0);
	
	std::string message;
	Text::Justify vertical, horizontal = Text::Justify::START;
	vec4 color = vec4(1,1,1,1);
	void setFont(const std::string& _font, const uint32_t height, const uint32_t width = 0);

	void update(double dt) { Text::draw(message, font.get(), vertical, horizontal, transform.position().x, transform.position().y, transform.scale().x, color); }
	void draw() {}
private:
	shared<Text::FontFace> font;
};

