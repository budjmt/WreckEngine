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
        vec4 bounds;     // Glyph bounds
        uvec4 texBounds; // Pixel bounds within the texture
        float advance;

        inline vec2 getBearing() const {
            return vec2(bounds.x, bounds.y);
        }
        inline vec2 getSize() const {
            return vec2(bounds.z, bounds.w);
        }
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

    struct Instance {
        Instance();
        ~Instance();
        
        inline void setColor(const vec4& _color) {
            color = _color;
            dirty = true;
        }
        void setFont(FontFace* _font) {
            if (font != _font) {
                font = _font;
                dirty = true;
            }
        }
        inline void setOffset(const vec2& _offset) {
            offset = _offset;
        }
        inline void setPosition(float _x, float _y) {
            x = _x;
            y = _y;
        }
        inline void setScale(float _scale) {
            scale = _scale;
        }
        inline void setText(const std::string& _text) {
            if (text != _text) {
                text = _text;
                dirty = true;
            }
        }

    private:
        GLVAO vao;
        GLbuffer buffer;
        std::string text;
        vec4 color;
        vec2 offset;
        FontFace* font = nullptr;
        GLuint arrayCount;
        float x = 0, y = 0, scale = 1;
        bool dirty = true;

        void rebuildBuffer();

        friend struct Renderer;
    };

    struct Renderer {
        void init();
        void draw();
        Render::MaterialPass* renderer;
    private:
        typedef struct {
            GLprogram program;
            GLuniform<GLsampler> sampler;
            GLuniform<mat4> cam;
            GLuniform<vec2> offset;
        } Shader;
        static Shader shader;
    };

    enum Justify : GLubyte { START, MIDDLE, END };

    //void draw(const std::string& text, const FontFace* font, Justify vertical, Justify horizontal, float x, float y, float scale, const vec4& color = vec4(0, 0, 0, 1));
    void render(Render::MaterialPass* renderer);

    vec2 getDims(char ch, const FontFace* font, float scale);
    vec2 getDims(uint32_t cp, const FontFace* font, float scale);
    vec2 getDims(const std::string& text, const FontFace* font, float scale);
    float getKerning(char ch1, char ch2, const FontFace* font);
    float getKerning(uint32_t cp1, uint32_t cp2, const FontFace* font);
}
