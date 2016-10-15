#pragma once

#include <string>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "smart_ptr.h"
#include "unique_id.h"
#include "gl_structs.h"

namespace Text
{
	void init();
	extern bool active;

	struct FontFace;
	shared<FontFace> loadFont(const std::string& font, const uint32_t height, const uint32_t width = 0);
	shared<FontFace> loadWinFont(const std::string& font, const uint32_t height, const uint32_t width = 0);

	struct glyph {
		GLtexture tex;
		vec2 size, bearing;
		uint32_t advance;
	};

	struct FontFace {
		FontFace() = default;
		FontFace(const std::string& font); ~FontFace();
		FT_Face fontFace = nullptr;
		uint32_t height = 0, width = 0;
		std::unordered_map<char, glyph> glyphs;
		static FT_Long loadedFonts;

		void loadGlyphs();
		void setSize(const uint32_t height, const uint32_t width = 0);
	};

	struct Instance {
		std::string text;
		const FontFace* font;
		float x, y, scale;
		vec4 color;
	};

	struct Renderer {
		void init();
		// x and y are in pixels
		void draw(Instance& inst);
	private:
		GLVAO vao; GLbuffer buffer;
		typedef struct {
			GLprogram program;
			GLuniform<GLsampler> sampler;
			GLuniform<vec4> color;
			GLuniform<mat4> cam;
		} Shader;
		static Shader shader;
	};
	
	enum Justify : GLubyte { START, MIDDLE, END };

	void draw(const std::string& text, const FontFace* font, Justify vertical, Justify horizontal, float x, float y, float scale, const vec4& color = vec4(0, 0, 0, 1));
	void render();

	vec2 getDims(const std::string& text, const FontFace* font, float scale);
}
