#pragma once

#include <string>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "smart_ptr.h"
#include "unique_id.h"
#include "gl_structs.h"
#include "Color.h"
#include "Render.h"

namespace Text
{
    using glm::uvec4;

    void init();
    extern bool active;

    struct FontFace;
    shared<FontFace> loadFont(const std::string& font, const uint32_t height, const uint32_t width = 0);
    shared<FontFace> loadWinFont(const std::string& font, const uint32_t height, const uint32_t width = 0);

    struct Glyph {
        vec2 bearing;
        vec2 size;
        vec2 texBearing;
        vec2 texSize;
        float advance;
    };

    struct FontFace {
        static constexpr int TEX_SIZE = 512;

        FontFace() = default;
        FontFace(const std::string& font); ~FontFace();

        FT_Face fontFace = nullptr;
        uint32_t height = 0, width = 0;
        float spaceWidth = 0.0f;
        float lineHeight = 0.0f;
        GLtexture tex;
        std::unordered_map<uint32_t, Glyph> glyphs;

        void loadGlyphs();
        void loadGlyphRange(uint32_t begin, uint32_t end);
        void setSize(const uint32_t height, const uint32_t width = 0);
    };

    enum Justify : GLubyte {START, MIDDLE, END};

    vec2 getDims(char ch, const FontFace* font, float scale);
    vec2 getDims(uint32_t cp, const FontFace* font, float scale);
    vec2 getDims(const std::string& text, const FontFace* font, float scale);
    vec2 getDims(const std::string& text, const FontFace* font, float scale, int& lineCount);
    float getKerning(char ch1, char ch2, const FontFace* font);
    float getKerning(uint32_t cp1, uint32_t cp2, const FontFace* font);

    struct Instance {
        Instance();
        
        void queueForDraw();

        inline void alignHorizontal(Justify justify) {
            if (horiz != justify) {
                horiz = justify;
                dirtyAlign = true;
            }
        }
        inline void alignVertical(Justify justify) {
            if (vert != justify) {
                vert = justify;
                dirtyAlign = true;
            }
        }
        inline void setColor(const vec4& _color) {
            color = _color;
            dirtyBuffer = true;
        }
        inline void setFont(FontFace* _font) {
            if (font != _font) {
                font = _font;
                if (font) {
                    renderInfo.setTextures(font->tex);
                }
                dirtyBuffer = true;
            }
        }
        inline void setPosition(float _x, float _y) {
            offset.x = _x;
            offset.y = _y;
        }
        inline void setScale(float _scale) {
            if (scale.value != _scale) {
                scale.value = _scale;
                dirtyAlign = true;
            }
        }
        inline void setText(const std::string& _text) {
            if (text != _text) {
                text = _text;
                dirtyBuffer = true;
            }
        }

    private:
        Render::Info renderInfo;
        GLVAO vao;
        GLbuffer buffer;
        
        FontFace* font = nullptr;
        std::string text;
        vec4 color;

        GLresource<vec2> fullOffset;
        vec2 offset, alignOffset;
        GLresource<float> scale;
        
        GLuint arrayCount = 0;
        
        Justify horiz, vert;
        bool dirtyAlign = false;
        bool dirtyBuffer = true;

        void updateAlignment();
        void updateBuffer();
        
        friend struct Renderer;
    };

    struct Renderer {
        void init();
        void draw();
        Render::MaterialPass* renderer;
    private:
        struct Shader {
            GLprogram program;
            GLuniform<GLsampler> sampler;
            GLresource<mat4> cam;
            GLuniform<vec2> offset;
            GLuniform<float> scale;
        };
        static Shader shader;

        friend Instance;
    };

    //void draw(const std::string& text, const FontFace* font, Justify vertical, Justify horizontal, float x, float y, float scale, const vec4& color = vec4(0, 0, 0, 1));
    void render(Render::MaterialPass* renderer);
}
