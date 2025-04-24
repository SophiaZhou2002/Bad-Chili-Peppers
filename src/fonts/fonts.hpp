
#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>

#include "../common.hpp"
#include "../utils/debug_log.hpp"
#include "../tinyECS/components.hpp"

/*
* Font rendering code based on the following tutorial:
* https://learnopengl.com/In-Practice/Text-Rendering
* 
* You do not need to use any of the code below on your own
* Instead, you should create a text render request using world_init.cpp::createText()
* Ensure you include this header "fonts/fonts.hpp" where relevant (for acces to the FONT_ASSET_ID enum)
* Ensure to remove the entity once you want to remove the text being displayed
*/
namespace GameText {

	inline std::map<FONT_ASSET_ID, std::string> font_files = {
		{FONT_ASSET_ID::ARIAL, "ArialNarrow7.ttf"},
        {FONT_ASSET_ID::FUTURE_NARROW, "Kenney_Future_Narrow.ttf"},
		{FONT_ASSET_ID::SUPER_PIXEL, "SuperPixel-m2L8j.ttf"},
		{FONT_ASSET_ID::PIXELLARI, "Pixellari.ttf"},
		{FONT_ASSET_ID::BUBBLE, "04B_30__.ttf"}
	};

	struct Character {
		unsigned int 	TextureID;	// ID handle of the glyph texture
		glm::ivec2 	 	Size;		// Size of glyph
		glm::ivec2 		Bearing;	// Offset from baseline to left/top of glyph
		unsigned int 	Advance;	// Offset to advance to next glyph
	};

    class Font {
    private:
        GLuint font_vao, font_vbo;
        FONT_ASSET_ID fid;
        std::map<char, Character> Characters;
        
        void initBuffers();
        void initFreeType();
        void loadFont(FT_Library &ft, FT_Face &face);
        void load();
        void breakLines(std::string text, float x, float y, float width, float scale, std::vector<TextLine>& lines);
        void centerLine(TextLine& line);
        void renderLine(TextLine& line, float scale);
            
    public:
        Font();
        ~Font();
        
        void init(FONT_ASSET_ID font_id);
        void render(TextRenderRequest& request);
    };

        class FontRenderer {
        private:
            mat4 createProjectionMatrix(float width, float height);
            
            GLuint shader;
            std::map<FONT_ASSET_ID, Font> fonts;

    public:
        FontRenderer();
        ~FontRenderer();
        
        void init(GLuint shader);
        void use(int width, int height);
        void render(TextRenderRequest& request);
    };
}
using namespace GameText;