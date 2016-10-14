#include "Text.h"

#include <iostream>

#include "External.h"

void Text::FT_Wrapper::init() {
	auto error = FT_Init_FreeType(&lib); 
	if (error) printf("FreeType initialization failed. Error Code: %d\n", error);
}
Text::FT_Wrapper::~FT_Wrapper() { if (lib) { FT_Done_FreeType(lib); lib = nullptr; } }
Text::FT_Wrapper Text::FT;

Text::FontFace::FontFace(const std::string& font) {
	auto error = FT_New_Face(Text::FT.lib, font.c_str(), loadedFonts++, &fontFace);
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
	if (Text::FT.lib) FT_Done_Face(fontFace);
	--loadedFonts;
}
FT_Long Text::FontFace::loadedFonts = 0;

std::unordered_map<std::string, shared<Text::FontFace>> Text::fontFaces;

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
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

bool Text::active = true;
Text::Renderer Text::renderer;

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

std::vector<Text::Instance> Text::instances;
void Text::draw(const std::string& text, const FontFace* font, float x, float y, float scale, const vec4& color) {
	if (!Text::active || font == nullptr) 
		return;

	Instance i;
	i.text = text;
	i.font = font;
	i.x = x; i.y = Window::height - y;
	i.scale = scale;
	i.color = color;
	instances.push_back(i);
}

void Text::render() {
	if (Text::active) { for (auto& instance : instances) renderer.draw(instance); }
	instances.clear();
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

void Text::Renderer::draw(Text::Instance& instance) {
	shader.program.use();
	shader.color.update(instance.color);
	shader.cam.update(glm::ortho(0.f, (float)Window::width, 0.f, (float)Window::height));

	vao.bind();
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
		buffer.bind();
		buffer.subdata(verts, sizeof(verts));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		instance.x += (glyph.advance >> 6) * instance.scale;// the advance is measured in 1/64 pixels, i.e. 1/2^6
	}
}

const std::string Text::WIN_DIR = getEnvVar("windir");