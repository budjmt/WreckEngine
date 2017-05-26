#pragma once

#include "Entity.h"
#include "Text.h"

class TextEntity : public Entity {
public:
    TextEntity(const std::string _message, const std::string& _font, Text::Justify _vertical, Text::Justify _horizontal, const uint32_t height, const uint32_t width = 0);

    Text::Justify vertical = Text::Justify::START,
                  horizontal = Text::Justify::START;
    void setFont(const std::string& _font, const uint32_t height, const uint32_t width = 0);

    inline void setColor(const vec4& color) {
        inst->setColor(color);
    }
    inline void setMessage(const std::string& message) {
        inst->setText(message);
    }

    inline virtual void update(double dt) {
        //Text::draw(message, font.get(), vertical, horizontal, transform.position().x, transform.position().y, transform.scale().x, color);
        auto position = transform.getComputed()->position();
        auto scale = transform.getComputed()->scale();

        inst->alignHorizontal(horizontal);
        inst->alignVertical(vertical);
        inst->setPosition(position.x, position.y);
        inst->setScale(scale.x);

        inst->queueForDraw(this);
    }
    void draw() {}
private:
    shared<Text::FontFace> font;
    shared<Text::Instance> inst;
};

class TimedTextEntity : public TextEntity {
public:
    using TextEntity::TextEntity;

    inline void setDuration(float dur) {
        duration = dur;
        active = true;
    }

    inline void show(float dur, const std::string& message) {
        setDuration(dur);
        setMessage(message);
    }

    inline void update(double dt) override {
        duration -= (float) Time::delta;
        if (duration <= 0.0f) {
            active = false;
        }
        else {
            TextEntity::update(dt);
        }
    }

private:
    float duration = 0.0f;
};
