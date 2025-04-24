#include "fonts.hpp"

#include <iostream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

FontRenderer::FontRenderer() {};

FontRenderer::~FontRenderer() {
	fonts.clear();
};

void FontRenderer::init(GLuint _shader) {
	shader = _shader;
	
	// Load fonts
	for (int i = 0; i < FONT_ASSET_ID::FONT_COUNT; i++) {
		fonts[(FONT_ASSET_ID)i] = Font();
		fonts[(FONT_ASSET_ID)i].init((FONT_ASSET_ID)i);
	}
	
	// Required for text rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
}

void FontRenderer::render(TextRenderRequest& request) {	
	GLint prev_vao = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prev_vao);
	
	glUseProgram(shader);
	glUniform3f(glGetUniformLocation(shader, "textColor"), request.color.x, request.color.y, request.color.z);
	
	// Load font
	Font& font = fonts[request.font];
	
	font.render(request);
	
	glBindVertexArray(prev_vao);
	glBindTexture(GL_TEXTURE_2D, 0);
}

mat4 FontRenderer::createProjectionMatrix(float width, float height) {
	return glm::ortho(0.0f, width, 0.0f, height);
}

void FontRenderer::use(int width, int height) {
	mat4 projection = createProjectionMatrix(width, height);
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}