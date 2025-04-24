#include "fonts.hpp"

#include <iostream>

Font::Font() {}

Font::~Font() {
    // Delete all textures created for each character.
    for (auto& pair : Characters) {
        GLuint textureID = pair.second.TextureID;
        glDeleteTextures(1, &textureID);
    }
    Characters.clear();

    // Delete the Vertex Array Object (VAO) if it was created.
    if (font_vao != 0) {
        glDeleteVertexArrays(1, &font_vao);
        font_vao = 0;
    }

    // Delete the Vertex Buffer Object (VBO) if it was created.
    if (font_vbo != 0) {
        glDeleteBuffers(1, &font_vbo);
        font_vbo = 0;
    }
}

void Font::init(FONT_ASSET_ID font_id) {
	fid = font_id;
	initFreeType();
	initBuffers();
}

void Font::load() {
	if (font_vao == 0) {
		std::cerr << "ERROR::FREETYPE: Font not initialized" << std::endl;
		assert(false);
	}
	
	if (Characters.empty()) {
		std::cerr << "ERROR::FREETYPE: Font not loaded" << std::endl;
		assert(false);
	}
	
	if (font_vbo == 0) {
		std::cerr << "ERROR::FREETYPE: Font VBO not initialized" << std::endl;
		assert(false);
	}

	glBindVertexArray(font_vao);
	glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
}

void Font::centerLine(TextLine& line) {
	line.x -= line.width / 2.f;
	line.y -= line.height / 2.f;
}

void Font::breakLines(std::string text, float x, float y, float width, float scale, std::vector<TextLine>& lines) {
	float textWidth = 0.0f;
	float textHeight = 0.0f;
	int line_start = 0;
	
	std::string::const_iterator c;
	
	for (c = text.begin(); c != text.end(); c++) {
		auto ch = Characters[*c];
		textWidth += int(ch.Advance >> 6) * scale;
		textHeight = max(textHeight, (float)ch.Size.y);
		
		if (textWidth > width && width != 0.0f && *c == ' ') {
			std::string line_text = text.substr(line_start, (c - text.begin()) - line_start);			
			auto lastCh = text.end() - 1;
			textWidth -= scale * ((Characters[*lastCh].Advance >> 6) / 2.f);
			lines.push_back({
				line_text, 
				textWidth, 
				textHeight,
				x,
				y,
			});
			y -= textHeight;
			
			line_start = c - text.begin();
			textWidth = 0.0f;
			textHeight = 0.0f;
		}
	}
	
	if (textWidth > 0.0f) {
		auto lastCh = text.end() - 1;
        textWidth -= scale * ((Characters[*lastCh].Advance >> 6) / 2.f);
        
        std::string line_text = text.substr(line_start, (c - text.begin()) - line_start);	
		lines.push_back({
			line_text, 
			textWidth, 
			textHeight,
			x,
			y,
		});
	}
}

void Font::renderLine(TextLine& line, float scale) {
	float x = line.x;
	float y = line.y;
	
	std::string::const_iterator c;
	
	for (c = line.text.begin(); c != line.text.end(); c++) {
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		
		// update VBO for each character
		float vertices[6][4] = {
			{xpos, ypos + h, 0.0f, 0.0f},
			{xpos, ypos, 0.0f, 1.0f},
			{xpos + w, ypos, 1.0f, 1.0f},

			{xpos, ypos + h, 0.0f, 0.0f},
			{xpos + w, ypos, 1.0f, 1.0f},
			{xpos + w, ypos + h, 1.0f, 0.0f}};
			
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		
		// update content of VBO memory
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale;  // bitshift by 6 to get value in pixels (2^6 = 64)
	}
}

void Font::render(TextRenderRequest& request) {
	load();
	glActiveTexture(GL_TEXTURE0);

	std::vector<TextLine>& lines = request.lines;
	
	if (lines.empty() || request.isDynamic) {
		lines.clear();
		breakLines(request.text, request.position.x, request.position.y, request.width, request.scale, lines);

		// Text centering reference: https://gamedev.stackexchange.com/questions/178035/how-do-i-render-a-word-in-the-middle-of-the-screen-using-freetype-and-opengl
		if (request.center_text) {
			for (auto& line : lines) {
				centerLine(line);
			}
		}
	}
	

	for (auto& line : lines) {
		renderLine(line, request.scale);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Font::initFreeType() 
{
    FT_Library ft;
	FT_Face face;

	if (FT_Init_FreeType(&ft)) {
		std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		assert(false);
	}
	// Load a font face (update the path to your font file)
	if (FT_New_Face(ft, fonts_path(font_files[fid]).c_str(), 0, &face)) {
		std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
		assert(false);
	}
	// Set the desired font size
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Load the first 128 characters of the ASCII set
	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) {
		std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		assert(false);
	}

	loadFont(ft, face);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void Font::loadFont(FT_Library &ft, FT_Face &face) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // disable byte-alignment restriction

	for (unsigned char c = 0; c < 128; c++) {
		// load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer);
			
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<unsigned int>(face->glyph->advance.x)
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}
}

void Font::initBuffers() {
    // Save the currently bound VAO
	GLint prev_vao = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prev_vao);

	// Create a VAO and VBO for the text rendering
	glGenVertexArrays(1, &font_vao);
	glGenBuffers(1, &font_vbo);

	// Bind the VAO and VBO
	glBindVertexArray(font_vao);
	glBindBuffer(GL_ARRAY_BUFFER, font_vbo);

	// The 2D quad requires 6 vertices of 4 floats each, so we reserve 6 * 4 floats of memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	// Unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);	

	// Restore the previously bound VAO
	glBindVertexArray(prev_vao);
}
