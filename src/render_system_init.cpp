// stdlib
#include <iostream>
#include <sstream>
#include <array>
#include <fstream>

// internal
#include "../ext/stb_image/stb_image.h"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"


// Render initialization
bool RenderSystem::init(GLFWwindow* window_arg)
{
	this->window = window_arg;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	const int is_fine = gl3w_init();
	assert(is_fine == 0);

	// Create a frame buffer
	frame_buffer = 0;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// For some high DPI displays (ex. Retina Display on Macbooks)
	// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	if (frame_buffer_width_px != WINDOW_WIDTH_PX)
	{
		printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
		printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
		printf("requested window width,height = %d,%d\n", WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
	}

	// Hint: Ask your TA for how to setup pretty OpenGL error callbacks. 
	// This can not be done in mac os, so do not enable
	// it unless you are on Linux or Windows. You will need to change the window creation
	// code to use OpenGL 4.3 (not suported on mac) and add additional .h and .cpp
	// glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

	// We are not really using VAO's but without at least one bound we will crash in
	// some systems.
	glGenVertexArrays(1, &global_vao);
	glBindVertexArray(global_vao);
	gl_has_errors();

	initScreenTexture();

	limited_vision_object_buffer = 0;
	glGenFramebuffers(1, &limited_vision_object_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, limited_vision_object_buffer);
	gl_has_errors();

	initLimitedVisionObjectTexture();
	
	initializeGlTextures();
	initializeGlEffects();
	initializeGlGeometryBuffers();

	font_renderer.init(effects[(int)EFFECT_ASSET_ID::FONT]);
	
	initializeGLInstanceBuffers();

    // Change window icon to chilli pepper
    GLFWimage images[1];
    // RGBA -> 4 channels
    std::string icon_path = data_path() + "/textures/player/player.png";
    images[0].pixels = stbi_load(icon_path.c_str(), &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window_arg, 1, images);
    stbi_image_free(images[0].pixels);

	// Enable depth testing
	// glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_LESS);

	player_screen_position = { 0.5f, 0.5f };

	return true;
}

void RenderSystem::initializeGlTextures()
{
	glGenTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());

	for(uint i = 0; i < texture_paths.size(); i++)
	{
		const std::string& path = texture_paths[i];
		ivec2& dimensions = texture_dimensions[i];

		stbi_uc* data;
		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

		if (data == NULL)
		{
			const std::string message = "Could not load the file " + path + ".";
			fprintf(stderr, "%s", message.c_str());
			assert(false);
		}
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		if (i >= (uint)TEXTURE_ASSET_ID::FIRE_1 && i <= (uint)TEXTURE_ASSET_ID::FIRE_14) {
			// Potentially use mipmaps for bloom
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		gl_has_errors();
		stbi_image_free(data);
	}
	gl_has_errors();
}

void RenderSystem::initializeGlEffects()
{
	for(uint i = 0; i < effect_paths.size(); i++)
	{
		const std::string vertex_shader_name = effect_paths[i] + ".vs.glsl";
		const std::string fragment_shader_name = effect_paths[i] + ".fs.glsl";

		bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, effects[i]);
		assert(is_valid && (GLuint)effects[i] != 0);
	}
}

void RenderSystem::initInstanceAttribs(TEXTURE_ASSET_ID tid, GEOMETRY_BUFFER_ID gid)
{
	// Use the instanced shader program
	const GLuint used_effect_enum = (GLuint)EFFECT_ASSET_ID::INSTANCED;
	const GLuint program = effects[used_effect_enum];

	// Bind the VAO for this texture
	glBindVertexArray(instance_vaos[(uint)tid]);

	// Set up the base geometry (aPos, aTexCoord)
	GLint aPos_loc      = glGetAttribLocation(program, "aPos");
	GLint aTexCoord_loc = glGetAttribLocation(program, "aTexCoord");

	// Bind the sprite VBO and IBO (base geometry)
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)gid]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)gid]);

	glEnableVertexAttribArray(aPos_loc);
	glVertexAttribPointer(
		aPos_loc,
		3,                // vec3
		GL_FLOAT,
		GL_FALSE,
		sizeof(TexturedVertex),
		(void*)offsetof(TexturedVertex, position)
	);

	glEnableVertexAttribArray(aTexCoord_loc);
	glVertexAttribPointer(
		aTexCoord_loc,
		2,                // vec2
		GL_FLOAT,
		GL_FALSE,
		sizeof(TexturedVertex),
		(void*)offsetof(TexturedVertex, texcoord)
	);
	gl_has_errors();

	// Bind and configure the instance VBO (mat3 + alpha)
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbos[(uint)tid]);

	// Set up the alpha attribute
	GLuint instance_alpha_loc = glGetAttribLocation(program, "instance_alpha");
	if (instance_alpha_loc == (GLuint)-1) {
			std::cerr << "Error: instance_alpha not found in shader!" << std::endl;
			assert(false);
	}
	glEnableVertexAttribArray(instance_alpha_loc);
	glVertexAttribPointer(
			instance_alpha_loc,
			1,           // float
			GL_FLOAT,
			GL_FALSE,
			sizeof(InstancedVertex),
			(void*)offsetof(InstancedVertex, alpha)
	);
	glVertexAttribDivisor(instance_alpha_loc, 1); // update per instance

	// Set up the transform matrix attributes (each row of the mat3)
	GLint transform_loc_0 = glGetAttribLocation(program, "instance_transform_0");
	GLint transform_loc_1 = glGetAttribLocation(program, "instance_transform_1");
	GLint transform_loc_2 = glGetAttribLocation(program, "instance_transform_2");
	if (transform_loc_0 == -1 || transform_loc_1 == -1 || transform_loc_2 == -1) {
		std::cerr << "Error: One or more transform attributes not found in shader!" << std::endl;
		assert(false);
	}
	
	std::array<GLint, 3> transform_locs = {
		transform_loc_0,
		transform_loc_1,
		transform_loc_2,
	};
	
	// Bind each row of the mat3 separately
	for (int i = 0; i < 3; i++) {
		GLint loc = transform_locs[i];
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(InstancedVertex), (void*)(sizeof(vec3) * i));
		glVertexAttribDivisor(loc, 1); // Update once per instance
	}

	gl_has_errors();

	// Unbind VAO to avoid accidental modifications
	glBindVertexArray(global_vao);
}


void RenderSystem::initInstanceDataVBO(TEXTURE_ASSET_ID tid) 
{	
	// bind to the instance's VAO
	glBindVertexArray(instance_vaos[(uint)tid]);
	
	// Create Instance VBO (Keeps track of the instance offset data)
	glGenBuffers(1, &instance_vbos[(uint)tid]);
	
	// Bind to the VBO (handles the individual instance values)
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbos[(uint)tid]);
	
	// Load dummy instance data to the instance's VBO to initialize size
	std::vector<InstancedVertex> instances(MAX_PARTICLES + 500); 
	size_t buffer_size = instances.size() * sizeof(InstancedVertex);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, instances.data(), GL_DYNAMIC_DRAW);
	
	// Unbind to avoid accidental augmentations
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	
	gl_has_errors();
	
	// Rebind to global VAO to avoid accidental augmentations
	glBindVertexArray(global_vao);
}

void RenderSystem::initializeGLInstanceBuffers() 
{	
	// Create Instance VAO (keeps track of entirety of instance data)
	glGenVertexArrays((GLsizei)instance_vaos.size(), instance_vaos.data());
	
	for (int i = 0; i < texture_count; i++) 
	{
		initInstanceDataVBO((TEXTURE_ASSET_ID)i);
		
		initInstanceAttribs((TEXTURE_ASSET_ID)i, GEOMETRY_BUFFER_ID::SPRITE);
		
	}
}

// One could merge the following two functions as a template function...
template <class T>
void RenderSystem::bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices)
{
	// Initial glGenBuffers call occurs in RenderSystem::initializeGlGeometryBuffers()
	// Initializes all buffers in each array of buffers
	
	// bind vector buffer object
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)gid]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	gl_has_errors();

	// bindindex buffer object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)gid]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
	gl_has_errors();
}

void RenderSystem::initializeGlMeshes()
{
	for (uint i = 0; i < mesh_paths.size(); i++)
	{
		// Initialize meshes
		GEOMETRY_BUFFER_ID geom_index = mesh_paths[i].first;
		std::string name = mesh_paths[i].second;
		Mesh::loadFromOBJFile(name, 
			meshes[(int)geom_index].vertices,
			meshes[(int)geom_index].vertex_indices,
			meshes[(int)geom_index].original_size);

		bindVBOandIBO(geom_index,
			meshes[(int)geom_index].vertices, 
			meshes[(int)geom_index].vertex_indices);
	}
}

void RenderSystem::initializeGlGeometryBuffers()
{
	// Vertex Buffer creation.
	glGenBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	// Index Buffer creation.
	glGenBuffers((GLsizei)index_buffers.size(), index_buffers.data());

	// Index and Vertex buffer data initialization.
	initializeGlMeshes();

	//////////////////////////
	// Initialize sprite
	// The position corresponds to the center of the texture.
	std::vector<TexturedVertex> textured_vertices(4);
	textured_vertices[0].position = { -1.f/2, +1.f/2, 0.f };
	textured_vertices[1].position = { +1.f/2, +1.f/2, 0.f };
	textured_vertices[2].position = { +1.f/2, -1.f/2, 0.f };
	textured_vertices[3].position = { -1.f/2, -1.f/2, 0.f };
	textured_vertices[0].texcoord = { 0.f, 1.f };
	textured_vertices[1].texcoord = { 1.f, 1.f };
	textured_vertices[2].texcoord = { 1.f, 0.f };
	textured_vertices[3].texcoord = { 0.f, 0.f };
    textured_vertices[0].color = { 1.0f, 0.0f, 0.0f };
    textured_vertices[1].color = { 0.0f, 1.1f, 1.1f };
    textured_vertices[2].color = { 0.0f, 1.0f, 0.0f };
    textured_vertices[3].color = { 0.0f, 0.0f, 1.0f };

	// Counterclockwise as it's the default OpenGL front winding direction.
	const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SPRITE, textured_vertices, textured_indices);

	//////////////////////////////////
	// Initialize debug line
	std::vector<ColoredVertex> line_vertices;
	std::vector<uint16_t> line_indices;

	constexpr float depth = 0.5f;
	// constexpr vec3 red = { 0.8, 0.1, 0.1 };
	constexpr vec3 red = { 1.0, 1.0, 1.0 };

	// Corner points
	line_vertices = {
		{{-0.5,-0.5, depth}, red},
		{{-0.5, 0.5, depth}, red},
		{{ 0.5, 0.5, depth}, red},
		{{ 0.5,-0.5, depth}, red},
	};

	// Two triangles
	line_indices = {0, 1, 3, 1, 2, 3};
	
	int geom_index = (int)GEOMETRY_BUFFER_ID::BOX;
	meshes[geom_index].vertices = line_vertices;
	meshes[geom_index].vertex_indices = line_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::BOX, line_vertices, line_indices);

	///////////////////////////////////////////////////////
	// Initialize screen triangle (yes, triangle, not quad; its more efficient).
	std::vector<vec3> screen_vertices(3);
	screen_vertices[0] = { -1, -6, 0.f };
	screen_vertices[1] = {  6, -1, 0.f };
	screen_vertices[2] = { -1,  6, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> screen_indices = { 0, 1, 2 };
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE, screen_vertices, screen_indices);
}

RenderSystem::~RenderSystem()
{
	// Don't need to free gl resources since they last for as long as the program,
	// but it's polite to clean after yourself.
	glDeleteBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	glDeleteBuffers((GLsizei)index_buffers.size(), index_buffers.data());
	glDeleteTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());
	glDeleteTextures(1, &screen_fire_radius_texture);
	glDeleteTextures(1, &screen_object_id_texture);
	glDeleteTextures(1, &limited_vision_object_color_texture);
	glDeleteTextures(1, &screen_normal_texture);
	glDeleteTextures(1, &screen_position_texture);
	glDeleteTextures(1, &screen_color_texture);
	// glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
	gl_has_errors();

	for(uint i = 0; i < effect_count; i++) {
		glDeleteProgram(effects[i]);
	}
	// delete allocated resources
	glDeleteFramebuffers(1, &limited_vision_object_buffer);
	glDeleteFramebuffers(1, &frame_buffer);
	gl_has_errors();

	// remove all entities created by the render system
	while (registry.renderRequests.entities.size() > 0)
			registry.remove_all_components_of(registry.renderRequests.entities.back());
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture()
{
	// create a single entry
	registry.screenStates.emplace(screen_state_entity);

	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &framebuffer_width, &framebuffer_height);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// Diffuse
	glGenTextures(1, &screen_color_texture);
	glBindTexture(GL_TEXTURE_2D, screen_color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexStorage2D(GL_TEXTURE_2D, 10, GL_RGBA8, framebuffer_width, framebuffer_height);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screen_color_texture, 0);
	gl_has_errors();

	// Object ID
	glGenTextures(1, &screen_object_id_texture);
	glBindTexture(GL_TEXTURE_2D, screen_object_id_texture);
	// 8-bit unsigned int texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, framebuffer_width, framebuffer_height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, screen_object_id_texture, 0);
	gl_has_errors();

	// Deferred rendering for lighting
	// Position
	glGenTextures(1, &screen_position_texture);
	glBindTexture(GL_TEXTURE_2D, screen_position_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, framebuffer_width, framebuffer_height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, screen_position_texture, 0);
	gl_has_errors();
	// Normal
	glGenTextures(1, &screen_normal_texture);
	glBindTexture(GL_TEXTURE_2D, screen_normal_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebuffer_width, framebuffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, screen_normal_texture, 0);
	gl_has_errors();

	// glGenRenderbuffers(1, &off_screen_render_buffer_depth);
	// glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
	// glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
	// glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
	// gl_has_errors();

	GLenum drawBuffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, drawBuffers);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return true;
}

bool RenderSystem::initLimitedVisionObjectTexture()
{
	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &framebuffer_width, &framebuffer_height);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	glGenTextures(1, &limited_vision_object_color_texture);
	glBindTexture(GL_TEXTURE_2D, limited_vision_object_color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, limited_vision_object_color_texture, 0);
	gl_has_errors();

	// Object ID (shared with frame_buffer)
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, screen_object_id_texture, 0);
	gl_has_errors();

	// Fire lighting radius
	// TODO turn this into an RGBA texture, storing the light positions as well
	glGenTextures(1, &screen_fire_radius_texture);
	glBindTexture(GL_TEXTURE_2D, screen_fire_radius_texture);
	// 8-bit float texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, framebuffer_width, framebuffer_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, screen_fire_radius_texture, 0);
	gl_has_errors();

	GLenum drawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, drawBuffers);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return true;
}

bool gl_compile_shader(GLuint shader)
{
	glCompileShader(shader);
	gl_has_errors();
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		std::vector<char> log(log_len);
		glGetShaderInfoLog(shader, log_len, &log_len, log.data());
		glDeleteShader(shader);

		gl_has_errors();

		fprintf(stderr, "GLSL: %s", log.data());
		return false;
	}

	return true;
}

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program)
{
	// Opening files
	std::ifstream vs_is(vs_path);
	std::ifstream fs_is(fs_path);
	if (!vs_is.good() || !fs_is.good())
	{
		fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
		assert(false);
		return false;
	}

	// Reading sources
	std::stringstream vs_ss, fs_ss;
	vs_ss << vs_is.rdbuf();
	fs_ss << fs_is.rdbuf();
	std::string vs_str = vs_ss.str();
	std::string fs_str = fs_ss.str();
	const char* vs_src = vs_str.c_str();
	const char* fs_src = fs_str.c_str();
	GLsizei vs_len = (GLsizei)vs_str.size();
	GLsizei fs_len = (GLsizei)fs_str.size();

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_src, &vs_len);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_src, &fs_len);
	gl_has_errors();

	// Compiling
	if (!gl_compile_shader(vertex))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}
	if (!gl_compile_shader(fragment))
	{
		fprintf(stderr, "Fragment compilation failed");
		assert(false);
		return false;
	}

	// Linking
	out_program = glCreateProgram();
	glAttachShader(out_program, vertex);
	glAttachShader(out_program, fragment);
	glLinkProgram(out_program);
	gl_has_errors();

	{
		GLint is_linked = GL_FALSE;
		glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
		if (is_linked == GL_FALSE)
		{
			GLint log_len;
			glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
			gl_has_errors();

			fprintf(stderr, "Link error: %s", log.data());
			assert(false);
			return false;
		}
	}

	// No need to carry this around. Keeping these objects is only useful if we recycle
	// the same shaders over and over, which we don't, so no need and this is simpler.
	glDetachShader(out_program, vertex);
	glDetachShader(out_program, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	gl_has_errors();

	return true;
}
