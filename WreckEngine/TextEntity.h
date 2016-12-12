#pragma once

#include "Entity.h"
#include "Text.h"

class TextEntity : public Entity {
public:
    TextEntity(const std::string _message, const std::string& _font, Text::Justify _vertical, Text::Justify _horizontal, const uint32_t height, const uint32_t width = 0);
    
    Text::Justify vertical, horizontal = Text::Justify::START;
    void setFont(const std::string& _font, const uint32_t height, const uint32_t width = 0);

    inline void setColor(const vec4& color) {
        inst->setColor(color);
    }
    inline void setMessage(const std::string& message) {
        inst->setText(message);
    }

    inline void update(double dt) {
        //Text::draw(message, font.get(), vertical, horizontal, transform.position().x, transform.position().y, transform.scale().x, color);
        auto position = transform.position();
        auto scale = transform.scale();
        
        inst->setPosition(position.x, position.y);
        inst->setScale(scale.x);
    }
    void draw() {}
private:
    shared<Text::FontFace> font;
    shared<Text::Instance> inst;
};

