#include "Text.h"

#include <iostream>

#include "External.h"

#include "safe_queue.h"

namespace {
	struct FT_Wrapper {
		
		void init() {
			auto error = FT_Init_FreeType(&lib);
			if (error) printf("FreeType initialization failed. Error Code: %d\n", error);
		}

		~FT_Wrapper() { if (lib) { FT_Done_FreeType(lib); lib = nullptr; } }
		FT_Library lib = nullptr;
	};
	FT_Wrapper FT;

	Text::Renderer renderer;
	thread_frame_vector<Text::Instance> instances;
	std::unordered_map<std::string, shared<Text::FontFace>> fontFaces;
	
	const std::string WIN_DIR = getEnvVar("windir");
}

bool Text::active = true;
void Text::init() { FT.init(); renderer.init(); }

Text::FontFace::FontFace(const std::string& font) {
	auto error = FT_New_Face(FT.lib, font.c_str(), loadedFonts++, &fontFace);
	if (error) {
		fontFace = nullptr;
		std::cout << "Font face \"" << font << "\" failed to load: " << std::endl;
		switch (error) {
		case FT_Err_Unknown_File_Format:
			std::cout << "The file format could not be read." << std::endl;
			break;
		default:
			std::cout << "Error code: " << error << std::endl;
		}
	}
	else {
		std::cout << "Successfully loaded \"" << font << "\"" << std::endl;
	}
}
Text::FontFace::~FontFace() { 
	// the lib, on destruction, takes care of all its children
	if (FT.lib) FT_Done_Face(fontFace);
	--loadedFonts;
}
FT_Long Text::FontFace::loadedFonts = 0;

shared<Text::FontFace> Text::loadWinFont(const std::string& font, const uint32_t height, const uint32_t width) { return loadFont(WIN_DIR + "\\Fonts\\" + font, height, width); }
shared<Text::FontFace> Text::loadFont(const std::string& font, const uint32_t height, const uint32_t width) {
	if (!fontFaces.count(font)) {
		auto f = make_shared<FontFace>(font);
		if (!f->fontFace) return shared<Text::FontFace>(nullptr);
		fontFaces.insert({ font, f });
	}
	auto f = fontFaces.at(font);
	f->setSize(height, width);
	return f;
}

void Text::FontFace::setSize(const uint32_t _height, const uint32_t _width) {
	if (height == _height && width == _width) return;
	FT_Set_Pixel_Sizes(fontFace, width = _width, height = _height);
	loadGlyphs();
}

void Text::FontFace::loadGlyphs() {
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	for (unsigned char c = 0; c < 128; ++c) {
		if (FT_Load_Char(fontFace, c, FT_LOAD_RENDER)) { std::cout << "Could not load glyph: " << c << std::endl; continue; }
		auto& glyph = fontFace->glyph;
		auto& bitmap = glyph->bitmap;
		GLtexture t;
		t.create();
		t.bind();
		t.set2D<GLubyte>(bitmap.buffer, bitmap.width, bitmap.rows, GL_RED, GL_RED);
		t.param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); t.param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		t.param(GL_TEXTURE_MIN_FILTER, GL_LINEAR); t.param(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glyphs[c] = { t, vec2(bitmap.width, bitmap.rows), vec2(glyph->bitmap_left, glyph->bitmap_top), (uint32_t) glyph->advance.x };
	}
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
}

#include "ShaderHelper.h"
Text::Renderer::Shader Text::Renderer::shader;
void Text::Renderer::init() {
	vao.create();
	vao.bind();

	buffer.create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
	buffer.bind();
	buffer.data(sizeof(GLfloat) * 4 * 6, nullptr);

	GLattrarr attrs;
	attrs.add<vec4>(1);
	attrs.apply();
	
	vao.unbind();

	shader.program = loadProgram("Shaders/text_v.glsl", "Shaders/text_f.glsl");
	shader.program.use();
	shader.sampler = shader.program.getUniform<GLsampler>("text");
	shader.sampler.update(0);
	shader.cam     = shader.program.getUniform<mat4>("camera");
	shader.color   = shader.program.getUniform<vec4>("textColor");
}

void Text::flush() {
    instances.flush();
}

void Text::preUpdate() {
    instances.get().unseal();
}

void Text::postUpdate() {
    instances.get().seal();
}

void Text::draw(const std::string& text, const FontFace* font, Justify vertical, Justify horizontal, float x, float y, float scale, const vec4& color) {
	if (!Text::active || font == nullptr) 
		return;

	float xoff = 0, yoff = 0;
	if (vertical != Justify::START || horizontal != Justify::START) {
		auto textDims = getDims(text, font, scale);
		
		if (vertical == Justify::MIDDLE)   y -= textDims.y * 0.5f;
		else if (vertical == Justify::END) y -= textDims.y;

		if (horizontal == Justify::MIDDLE)   x -= textDims.x * 0.5f;
		else if (horizontal == Justify::END) x -= textDims.x;
	}

	Instance i;
	i.text = text;
	i.font = font;
	i.x = x; i.y = Window::height - y;
	i.scale = scale;
	i.color = color;
	instances.get().push_back(i);
}

void Text::render(Render::MaterialPass* matRenderer) {
    renderer.renderer = matRenderer;
    if (Text::active) { instances.consumeAll([](auto& instances) { for (auto& instance : instances) renderer.draw(instance); }); }
    else instances.consumeAll([](auto&) {}); // clears the list without doing anything
}

vec2 Text::getDims(const std::string& text, const FontFace* font, float scale) {
	
	float x = 0, above = 0, below = 0;

	const auto updateY = [&](const Text::glyph& glyph) {
		auto a = glyph.bearing.y * scale
		   , b = glyph.size.y * scale - a;
		if (a > above) above = a;
		if (b > below) below = b;
	};
	
	auto len = text.length() - 1;
	for (size_t i = 0; i < len; ++i) {
		const auto glyph = font->glyphs.at(text[i]);	
		x += (glyph.advance >> 6) * scale;
		updateY(glyph);
	}

	const auto glyph = font->glyphs.at(text[len]);
	x += (glyph.size.x + glyph.bearing.x) * scale;
	updateY(glyph);

	return vec2(x, above + below);
}

// TODO update this to use a method that doesn't rely on sharing a buffer that changes between draw calls
void Text::Renderer::draw(Text::Instance& instance) {
	shader.program.use();
	shader.color.update(instance.color);
	shader.cam.update(glm::ortho(0.f, (float)Window::width, 0.f, (float)Window::height));

	vao.bind();
    buffer.bind();
	for (const auto c : instance.text) {

		const auto glyph = instance.font->glyphs.at(c);

		const auto tx = instance.x + glyph.bearing.x * instance.scale
			     , ty = instance.y - (glyph.size.y - glyph.bearing.y) * instance.scale;
		const auto dims = glyph.size * instance.scale;
		float verts[] = {
			tx,          ty + dims.y, 0, 0,
			tx,          ty,          0, 1,
			tx + dims.x, ty,          1, 1,
			tx,          ty + dims.y, 0, 0,
			tx + dims.x, ty,          1, 1,
			tx + dims.x, ty + dims.y, 1, 0
		};

		glyph.tex.bind();
		buffer.subdata(verts, sizeof(verts));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6)); // this works, but ignores the rendering pipeline; BAD

		instance.x += (glyph.advance >> 6) * instance.scale;// the advance is measured in 1/64 pixels, i.e. 1/2^6
	}
}