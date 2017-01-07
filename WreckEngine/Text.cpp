#include "Text.h"
#include "ShaderHelper.h"
#include "External.h"
#include FT_ERRORS_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include <array>
#include <iostream>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

namespace
{
    struct FT_Wrapper
    {
        void init()
        {
            if (lib)
                return;

            auto error = FT_Init_FreeType(&lib);
            if (error)
                printf("FreeType initialization failed. Error Code: %d\n", error);
        }

        ~FT_Wrapper()
        {
            if (lib)
            {
                FT_Done_FreeType(lib);
                lib = nullptr;
            }
        }

        FT_Library lib = nullptr;
    };

    FT_Wrapper FT;

    Text::Renderer renderer;
    std::vector<Text::Instance*> instances;
    std::unordered_map<std::string, shared<Text::FontFace>> fontFaces;

    inline size_t findInstance(const Text::Instance* inst) {
        for (size_t i = 0, size = instances.size(); i < size; ++i) {
            if (instances[i] == inst) {
                return i;
            }
        }
        return -1;
    }

    const std::string WIN_DIR = getEnvVar("windir");
}

static const float kerningScale = 1.0f / (1 << 6);
static FT_Long nextFontIndex = 0;
bool Text::active = true;
Text::Renderer::Shader Text::Renderer::shader;

// TODO - We'll eventually need to figure out a way to break up text based off of the color
// of words to massively cut down on the amount of data we send to the GPU
struct CharVertex {
    vec2 pos;
    vec2 uv;
    uint32_t color;
};

struct GlyphData {
    std::vector<unsigned char> bitmap;
    Text::Glyph glyph;
    uint32_t cp;
};

void Text::init()
{
    FT.init();
    renderer.init();
}

Text::FontFace::FontFace(const std::string& font)
{
    // Attempt to load the font
    auto error = FT_New_Face(FT.lib, font.c_str(), nextFontIndex++, &fontFace);
    if (error)
    {
        fontFace = nullptr;
        std::cout << "Font face \"" << font << "\" failed to load: " << std::endl;
        switch (error)
        {
            case FT_Err_Unknown_File_Format:
                std::cout << "The file format could not be read." << std::endl;
                break;
            default:
                std::cout << "Error code: " << error << std::endl;
        }
    }
    else
    {
        tex.create(GL_TEXTURE_2D);
        std::cout << "Successfully loaded \"" << font << "\"" << std::endl;
    }
}

Text::FontFace::~FontFace()
{
    // the lib, on destruction, takes care of all its children
    if (FT.lib)
        FT_Done_Face(fontFace);

    // We don't decrement the next font index because what if we have the following situation:
    // Load fonts A, B, C. Unload font B. Load font D. Fonts C and D now share an index.
}

shared<Text::FontFace> Text::loadWinFont(const std::string& font, const uint32_t height, const uint32_t width)
{
    return loadFont(WIN_DIR + "\\Fonts\\" + font, height, width);
}

shared<Text::FontFace> Text::loadFont(const std::string& font, const uint32_t height, const uint32_t width)
{
    shared<Text::FontFace> f;

    if (fontFaces.count(font))
    {
        f = fontFaces.at(font);
    }
    else
    {
        f = make_shared<FontFace>(font);
        if (!f->fontFace)
            return nullptr;
        fontFaces.insert({font, f});
    }

    f->setSize(height, width);
    return f;
}

void Text::FontFace::setSize(const uint32_t _height, const uint32_t _width)
{
    if (height == _height && width == _width)
        return;

    FT_Set_Pixel_Sizes(fontFace, width = _width, height = _height);

    // Get the size of a space
    FT_Glyph glyph = nullptr;
    if (!FT_Load_Char(fontFace, ' ', FT_LOAD_DEFAULT) &&
        !FT_Get_Glyph(fontFace->glyph, &glyph))
    {
        spaceWidth = fontFace->glyph->metrics.horiAdvance * kerningScale;
    }

    // Get the line height
    lineHeight = fontFace->size->metrics.height * kerningScale;

    // TODO - We may not want to immediately load glyphs here, or we may want to specify an overload for a glyph range
    loadGlyphs();
}

void Text::FontFace::loadGlyphs()
{
    // Loads all basic, printable ASCII characters
    loadGlyphRange(0, 255);
}

void Text::FontFace::loadGlyphRange(uint32_t begin, uint32_t end)
{
    // TODO - (M)SDF conversion?
    // TODO - Currently assumes the texture is empty. Expected functionality would be to have
    //        multiple glyph ranges loaded at a time

    constexpr int rectPaddingPerSide = 1;
    constexpr int rectPadding = rectPaddingPerSide * 2;

    // Load each of the glyphs in the range
    std::vector<stbrp_rect> packRects;
    std::unordered_map<uint32_t, shared<GlyphData>> loadedGlyphs;
    for (uint32_t cp = begin; cp <= end; ++cp)
    {
        // Load code point
        if (FT_Load_Char(fontFace, cp, FT_LOAD_FORCE_AUTOHINT))
        {
            std::cout << "Could not load code point " << cp << " (" << static_cast<char>(cp) << ')' << std::endl;
            continue;
        }

        // Load glyph of code point
        FT_Glyph glyph = nullptr;
        if (FT_Get_Glyph(fontFace->glyph, &glyph))
        {
            std::cout << "Failed to get glyph for " << cp << " (" << static_cast<char>(cp) << ')' << std::endl;
            continue;
        }

        // Render glyph to bitmap
        if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1))
        {
            std::cout << "Failed to render glyph for " << cp << " (" << static_cast<char>(cp) << ')' << std::endl;
            continue;
        }
        auto& bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap;
        const auto bitmapWidth = bitmap.width;
        const auto bitmapHeight = bitmap.rows;

        // Create pack rect (inflate it a bit for padding)
        stbrp_rect rect = {0};
        rect.id = cp;
        rect.w = static_cast<stbrp_coord>(bitmapWidth + rectPadding);
        rect.h = static_cast<stbrp_coord>(bitmapHeight + rectPadding);
        packRects.push_back(rect);

        // Create glyph data
        auto glyphData = std::make_shared<GlyphData>();
        auto& glyphMetrics = fontFace->glyph->metrics;
        glyphData->cp = cp;
        glyphData->bitmap.resize(rect.w * rect.h, 0); // texture is only alpha values
        glyphData->glyph.advance = static_cast<float>(fontFace->glyph->metrics.horiAdvance) * kerningScale;
        glyphData->glyph.bearing = {
            glyphMetrics.horiBearingX * kerningScale,
            glyphMetrics.horiBearingY * kerningScale
        };
        glyphData->glyph.size = {
            glyphMetrics.width        * kerningScale,
            glyphMetrics.height       * kerningScale
        };

        // Copy glyph buffer data (all pixels are white with varying alpha levels)
        const unsigned char* pixels = bitmap.buffer;
        auto& glyphBuffer = glyphData->bitmap;
        for (uint32_t y = 0; y < bitmapHeight; ++y)
        {
            for (uint32_t x = 0; x < bitmapWidth; ++x)
            {
                const size_t index = (x + rectPaddingPerSide) + (y + rectPaddingPerSide) * rect.w;
                if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                {
                    // Pixels are 1 bit monochrome values
                    auto isBitSet = ((pixels[x / 8]) & (1 << (7 - (x % 8))));
                    glyphBuffer[index] = isBitSet ? 255 : 0;
                }
                else
                {
                    // Pixels are 8-bit gray scale
                    glyphBuffer[index] = pixels[x];
                }
            }
            pixels += bitmap.pitch;
        }

        // Register the glyph data
        loadedGlyphs.insert({cp, glyphData});
    }

    // Now let's pack the glyphs into a texture
    // TODO - Need to ensure packing succeeded. If it didn't, try with a larger texture?
    std::array<stbrp_node, TEX_SIZE> packNodes;
    stbrp_context packCtx;
    stbrp_init_target(&packCtx, TEX_SIZE, TEX_SIZE, packNodes.data(), packNodes.size());
    stbrp_pack_rects(&packCtx, &packRects[0], packRects.size());
    
    // Create the texture
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    tex.bind();
    tex.set2D<GLubyte>(nullptr, TEX_SIZE, TEX_SIZE, GL_RED, GL_RED, 0);
    tex.param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    tex.param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    tex.param(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    tex.param(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Iterate the packed rectangles and build the texture
    for (const auto& rect : packRects)
    {
        auto cp = static_cast<uint32_t>(rect.id);
        auto glyphData = loadedGlyphs.at(cp);
        auto& glyph = glyphData->glyph;

        // Set the glyph's texture bounds, then record the glyph
        constexpr float uvScale = 1.0f / FontFace::TEX_SIZE;
        glyph.texBearing = {rect.x * uvScale, rect.y * uvScale};
        glyph.texSize    = {rect.w * uvScale, rect.h * uvScale};
        glyphs[cp] = glyph;

        // Update the sub-region of the texture
        auto& pixelBuffer = glyphData->bitmap;
        if (pixelBuffer.size() && rect.was_packed)
        {
            tex.setSub2D<GLubyte>(&pixelBuffer[0], rect.x, rect.y, rect.w, rect.h, GL_RED, 0);
        }
#if defined(_DEBUG)
        else
        {
            std::cout << "[WARN] Character " << cp << " ('" << static_cast<char>(cp) << "') does not have bitmap data!" << std::endl;
        }
#endif
    }
    
    // Unbind the texture
    tex.unbind();
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
}

Text::Instance::Instance() {
    vao.create();
    vao.bind();

    buffer.create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
    buffer.bind();
    buffer.data(0, nullptr);

    GLattrarr attrs;
    attrs.add<vec2>(2);
    attrs.add<uint32_t>(1);
    attrs.apply();

    buffer.unbind();
    vao.unbind();

    auto& shader = Renderer::shader;
    fullOffset = GLresource<vec2>(shader.offset);
    scale = GLresource<float>(shader.scale);
    scale.value = 1;
    renderInfo.setShaders(shader.program, &shader.cam, &fullOffset, &scale);
}

void Text::Instance::queueForDraw()
{
    instances.push_back(this);
}

void Text::Instance::updateAlignment() {
    if (!dirtyAlign)
        return;

    alignOffset = vec2(0);

    if (vert != START || horiz != START) {

        int lineCount; // Guaranteed to at least be 1
        auto dims = getDims(text, font, scale.value, lineCount);
        
        switch (horiz)
        {
        case MIDDLE:
            alignOffset.x = dims.x * -0.5f;
            break;
        case END:
            alignOffset.x = -dims.x;
            break;
        }

        switch (vert)
        {
        case MIDDLE:
            alignOffset.y = (dims.y - font->lineHeight * (lineCount - 1)) * -0.5f;
            break;
        case END:
            alignOffset.y = -dims.y + font->lineHeight * (lineCount - 1);
            break;
        }
    }

    dirtyAlign = false;
}

void Text::Instance::updateBuffer() {
    if (!dirtyBuffer || !font) {
        return;
    }

    // Get some helper variables
    const uint32_t packedColor = Color::pack(color);
    const float xSpace = font->spaceWidth;
    const float ySpace = font->lineHeight;
    float x = 0.0f;
    float y = 0.0f;
    uint32_t prevCP = 0;
    std::vector<CharVertex> vertices;

    // Now go through and create one quad per character
    for (size_t i = 0, length = text.length(); i < length; ++i) {
        uint32_t currCP = text[i];

        // Apply the kerning between the previous and current character
        x += getKerning(prevCP, currCP, font);
        prevCP = currCP;

        // Handle special characters
        if (isspace(currCP)) {
            switch (currCP) {
                case ' ':
                    x += xSpace;
                    break;
                case '\t':
                    x += xSpace * 4;
                    break;
                case '\n':
                    y -= ySpace;
                    x = 0.0f;
                    break;
            }

            // Now continue to the next glyph
            continue;
        }

        // Get the glyph for the current character
        const auto& glyph = font->glyphs.at(currCP);

        const float left   = glyph.bearing.x;
        const float top    = glyph.bearing.y;
        const float right  = left + glyph.size.x;
        const float bottom = top  - glyph.size.y;

        const float u1 = glyph.texBearing.x;
        const float v1 = glyph.texBearing.y;
        const float u2 = glyph.texBearing.x + glyph.texSize.x;
        const float v2 = glyph.texBearing.y + glyph.texSize.y;

        // Now add the quad
        vertices.push_back({{x + left,  y + top   }, {u1, v1}, packedColor});
        vertices.push_back({{x + right, y + top   }, {u2, v1}, packedColor});
        vertices.push_back({{x + left,  y + bottom}, {u1, v2}, packedColor});
        vertices.push_back({{x + left,  y + bottom}, {u1, v2}, packedColor});
        vertices.push_back({{x + right, y + top   }, {u2, v1}, packedColor});
        vertices.push_back({{x + right, y + bottom}, {u2, v2}, packedColor});

        // Advance to the next character
        x += glyph.advance;
    }

    // Update the mesh
    const size_t  dataSize = sizeof(CharVertex) * vertices.size();
    const GLvoid* dataPtr  = dataSize ? &vertices[0] : nullptr;
    buffer.bind();
    buffer.invalidate();
    buffer.data(dataSize, dataPtr);
    buffer.unbind();

    arrayCount = vertices.size();
    dirtyBuffer = false;
}

void Text::Renderer::init()
{
    shader.program = loadProgram("Shaders/text_v.glsl", "Shaders/text_f.glsl");
    shader.program.use();
    shader.sampler  = shader.program.getUniform<GLsampler>("text");
    shader.cam      = GLresource<mat4> (shader.program, "camera"); // shader.program.getUniform<mat4>("camera");
    shader.offset   = shader.program.getUniform<vec2>("offset");
    shader.scale    = shader.program.getUniform<float>("scale");
}

void Text::render(Render::MaterialPass* matRenderer)
{
    if (Text::active)
    {
        renderer.renderer = matRenderer;
        renderer.draw();
    }
}

vec2 Text::getDims(char ch, const FontFace* font, float scale)
{
    return getDims(static_cast<uint32_t>(ch), font, scale);
}

vec2 Text::getDims(uint32_t cp, const FontFace* font, float scale)
{
    vec2 dims;

    if (cp == ' ')
    {
        dims.x = font->spaceWidth;
    }
    else if (cp == '\t')
    {
        dims.x = font->spaceWidth * 4;
    }
    else if (font->glyphs.count(cp))
    {
        const auto& glyph = font->glyphs.at(cp);
        dims = {glyph.advance, glyph.bearing.y - glyph.size.y};
    }

    return dims * scale;
}

vec2 Text::getDims(const std::string& text, const FontFace* font, float scale)
{
    int lineCount = 0;
    return getDims(text, font, scale, lineCount);
}

vec2 Text::getDims(const std::string& text, const FontFace* font, float scale, int& lineCount)
{
    float x = 0, y = 0;
    float maxX = 0;
    lineCount = 1;

    for (size_t i = 0, length = text.length(); i < length; ++i)
    {
        uint32_t cp = text[i];
        
        if (cp == '\n')
        {
            x = 0.0f;
            y = 0.0f;
            ++lineCount;
        }
        else if (cp == '\r' || cp == '\b')
        {
            continue;
        }
        else
        {
            auto dims = getDims(cp, font, scale);
            x += dims.x;
            y = std::max(y, dims.y);
        }

        maxX = std::max(maxX, x);
    }

    return vec2(maxX, font->lineHeight * scale * lineCount);
}

float Text::getKerning(char ch1, char ch2, const FontFace* font)
{
    return getKerning(static_cast<uint32_t>(ch1), static_cast<uint32_t>(ch2), font);
}

float Text::getKerning(uint32_t cp1, uint32_t cp2, const FontFace* font)
{
    float kerning = 0.0f;
    auto ff = font->fontFace;

    if (FT_HAS_KERNING(ff))
    {
        auto ind1 = FT_Get_Char_Index(ff, cp1);
        auto ind2 = FT_Get_Char_Index(ff, cp2);

        FT_Vector vec;
        FT_Get_Kerning(ff, ind1, ind2, FT_KERNING_DEFAULT, &vec);

        float scale = (FT_IS_SCALABLE(ff) ? kerningScale : 1.0f);
        kerning = vec.x * scale;
    }

    return kerning;
}

void Text::Renderer::draw() {

    shader.cam.value = glm::ortho(0.f, (float)Window::width, 0.f, (float)Window::height);

    for (auto inst : instances) {
        
        inst->updateAlignment();
        inst->updateBuffer();

        inst->fullOffset.value = inst->offset + inst->alignOffset;

        renderer->scheduleDrawArrays(0, &inst->vao, &inst->renderInfo, GL_TRIANGLES, inst->arrayCount);
    }

    instances.clear();
}
