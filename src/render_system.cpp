
#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>

// internal
#include "render_system.hpp"
#include "tinyECS/registry.hpp"

void RenderSystem::drawBox(Entity entity, const mat3& projection) {

	Box& box = registry.boxes.get(entity);

	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(box.position);
	transform.scale(box.scale);

	assert(registry.renderRequests.has(entity));
	const RenderRequest& render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	if (render_request.used_effect == EFFECT_ASSET_ID::BOX)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		gl_has_errors();

		GLint in_color_loc    = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec4 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec4(1);
	glUniform4fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

/**
 * Activates the instance VAO and initializes the instance VBO to the max size of particles
 */
void RenderSystem::updateInstanceDataVBO(TEXTURE_ASSET_ID tid, std::vector<InstanceItem>& instances)
{
	// Bind the instance VBO
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbos[(uint)tid]);
	gl_has_errors();

	// Build CPU array of InstancedVertex
	std::vector<InstancedVertex> ivs;
	ivs.reserve(instances.size());

	for (auto &it : instances)
	{
		Transform t;
		t.translate(it.position);
		t.scale(it.scale);
		t.rotate(radians(it.angle));

		InstancedVertex iv;
		iv.offset = std::move(t.mat);
		iv.alpha = it.alpha;

		ivs.push_back(iv);
	}

	// Update the instance data
	// Uses glBufferSubData to avoid reallocating entire memory on each iteration
	glBufferSubData(GL_ARRAY_BUFFER, 0, ivs.size() * sizeof(InstancedVertex), ivs.data());
	gl_has_errors();

	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	gl_has_errors();
}


void RenderSystem::renderInstances(TEXTURE_ASSET_ID tid, size_t instanceCount)
{
	// Bind the VAO for the texture
	glBindVertexArray(instance_vaos[(uint)tid]);
	gl_has_errors();

	glDrawElementsInstanced(
			GL_TRIANGLES,        // mode
			6,                   // index count for a quad
			GL_UNSIGNED_SHORT,   // index type
			0,                   // offset in index buffer
			(GLsizei)instanceCount // number of instances
	);
	gl_has_errors();
}


void RenderSystem::drawTexturedInstance(const mat3 &projection, const InstanceRequest &instance_request)
{
	// Bind the correct VAO for this texture
	glBindVertexArray(instance_vaos[(uint)instance_request.texture]);
	gl_has_errors();

	// Use the instanced shader program
	const GLuint program = effects[(GLuint)EFFECT_ASSET_ID::INSTANCED];
	glUseProgram(program);
	gl_has_errors();

	// Pass the projection matrix
	GLint projection_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Bind the texture to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_gl_handles[(uint)instance_request.texture]);
	gl_has_errors();

	// Actually draw the instances
	size_t instanceCount = instance_request.items.size();
	renderInstances(instance_request.texture, instanceCount);

	// Unbind VAO
	glBindVertexArray(global_vao);
	gl_has_errors();
}



void RenderSystem::drawTexturedMesh(Entity entity, const mat3 &projection, uint8_t object_id)
{
	// std::cout << "RenderSystem::drawTexturedMesh" << std::endl;
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.scale(motion.scale);
	// transform.rotate(radians(motion.angle));

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// texture-mapped entities - use data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED ||
		render_request.used_effect == EFFECT_ASSET_ID::POWERUP ||
		render_request.used_effect == EFFECT_ASSET_ID::FIRE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		// GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
								sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

        // Check if EFFECT ID is POWERUP, if so then attach the colors to each vertex, if not only attach texCoords
        if (render_request.used_effect == EFFECT_ASSET_ID::POWERUP) {
		    GLint in_color_loc = glGetAttribLocation(program, "in_color");
            glEnableVertexAttribArray(in_texcoord_loc);
            glVertexAttribPointer(
                in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
                (void *)sizeof(
                    vec3)); // note the stride to skip the preceeding vertex position
            
            glEnableVertexAttribArray(in_color_loc);
            glVertexAttribPointer(
                in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)(sizeof(vec3) + sizeof(vec2))
            );

            // Pass in time as uniform for linear interpolation
            GLuint time_uloc = glGetUniformLocation(program, "time");
            glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
        } else if (render_request.used_effect == EFFECT_ASSET_ID::FIRE) {
            glEnableVertexAttribArray(in_texcoord_loc);
            glVertexAttribPointer(
                in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
                (void *)sizeof(
                    vec3)); // note the stride to skip the preceeding vertex position
			
			// Fire light flickering effect
			if (registry.fireBlocks.has(entity)) {
				FireBlock& fire = registry.fireBlocks.get(entity);
				GLint scale_uloc = glGetUniformLocation(program, "u_scale");
				glUniform1f(scale_uloc, fire.light_radius);
			}
		} else {
            glEnableVertexAttribArray(in_texcoord_loc);
            glVertexAttribPointer(
                in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
                (void *)sizeof(
                    vec3)); // note the stride to skip the preceeding vertex position
			
			// assign normal strength uniform
			GLint normal_strength_uloc = glGetUniformLocation(program, "normal_strength");
			if (normal_strength_uloc > -1) glUniform1f(normal_strength_uloc, render_request.used_normal_strength);
			gl_has_errors();
        }

		GLint object_id_uloc = glGetUniformLocation(program, "object_id");
		if (object_id_uloc > -1) glUniform1ui(object_id_uloc, object_id);

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id = 
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		// if normals are used for this RenderRequest, assign the texture
		if (render_request.used_normal_strength > 0.0f && render_request.used_normal_texture != TEXTURE_ASSET_ID::TEXTURE_COUNT) {
			glActiveTexture(GL_TEXTURE1);
			gl_has_errors();

			assert(registry.renderRequests.has(entity));
			GLuint normal_texture_id = 
				texture_gl_handles[(GLuint)render_request.used_normal_texture];

			glBindTexture(GL_TEXTURE_2D, normal_texture_id);
			gl_has_errors();
			
			GLint normal_uloc = glGetUniformLocation(program, "normal_sampler");
			glUniform1i(normal_uloc, 1);
			gl_has_errors();
		} else {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			gl_has_errors();
		}
	}
	// .obj entities
	else if (render_request.used_effect == EFFECT_ASSET_ID::MESH)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		// GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		// glEnableVertexAttribArray(in_color_loc);
		// glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
		// 					  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	} else {
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec4 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec4(1);
	glUniform4fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// first draw to an intermediate texture,
// apply the "vignette" texture, when requested
// then draw the intermediate texture
void RenderSystem::drawToScreen(GAME_SCREEN game_screen)
{
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::POST_PROCESS_LIMITED_VISION]);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]
	); 	// Note, GL_ELEMENT_ARRAY_BUFFER associates
			// indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	
	const GLuint limited_vision_program = effects[(GLuint)EFFECT_ASSET_ID::POST_PROCESS_LIMITED_VISION];

	// =================================== LIMITED VISION ===================================
	// Check if player is playing, if so apply limited vision shader
	GLuint limited_vision_uloc = glGetUniformLocation(limited_vision_program, "limited_vision");
	Map map;
	if (registry.maps.components.size() > 0) {
		map = registry.maps.components[0];
	}
	bool limited_vision = map.hasLimitedVision && (game_screen == GAME_SCREEN::PLAYING || game_screen == GAME_SCREEN::TUTORIAL_PLAYING);
	glUniform1i(limited_vision_uloc, limited_vision);
	
	GLuint player_world_position_uloc = glGetUniformLocation(limited_vision_program, "player_world_position");
	glUniform2f(player_world_position_uloc, player_world_position.x, player_world_position.y);
	gl_has_errors();

	GLuint player_position_uloc = glGetUniformLocation(limited_vision_program, "player_position");
	GLuint shadow_color_uloc = glGetUniformLocation(limited_vision_program, "shadow_color");
	glUniform2f(player_position_uloc, player_screen_position.x, player_screen_position.y);
	glUniform3f(shadow_color_uloc, map.shadowColor.r, map.shadowColor.g, map.shadowColor.b);
	gl_has_errors();
	// ======================================================================================

	// Set the vertex position and vertex texture coordinates
	GLint in_position_loc = glGetAttribLocation(limited_vision_program, "in_position");
	gl_has_errors();	
	
	glEnableVertexAttribArray(in_position_loc);
	gl_has_errors();
	
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();
	
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screen_color_texture);
	gl_has_errors();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, limited_vision_object_color_texture);
	gl_has_errors();

	// glActiveTexture(GL_TEXTURE2);
	// glBindTexture(GL_TEXTURE_2D, screen_object_id_texture);
	// gl_has_errors();

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, screen_fire_radius_texture);
	gl_has_errors();

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, screen_position_texture);
	gl_has_errors();
	
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, screen_normal_texture);
	gl_has_errors();

	
	GLint screen_color_texture_uloc = glGetUniformLocation(limited_vision_program, "screen_color_texture");
	GLint limited_vision_object_texture_uloc = glGetUniformLocation(limited_vision_program, "limited_vision_object_texture");
	// GLint screen_object_id_uloc = glGetUniformLocation(limited_vision_program, "screen_object_id_texture");
	GLint screen_fire_radius_uloc = glGetUniformLocation(limited_vision_program, "screen_fire_radius_texture");
	GLint screen_position_uloc = glGetUniformLocation(limited_vision_program, "screen_position_texture");
	GLint screen_normal_uloc = glGetUniformLocation(limited_vision_program, "screen_normal_texture");
	glUniform1i(screen_color_texture_uloc, 0);
	glUniform1i(limited_vision_object_texture_uloc, 1);
	// glUniform1i(screen_object_id_uloc, 2);
	glUniform1i(screen_fire_radius_uloc, 2);
	glUniform1i(screen_position_uloc, 3);
	glUniform1i(screen_normal_uloc,4);
	gl_has_errors();

	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
					// no offset from the bound index buffer
	gl_has_errors();

	// clear texture parameters
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	gl_has_errors();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	gl_has_errors();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	gl_has_errors();
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	gl_has_errors();
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(GAME_SCREEN game_screen)
{
	// std::cout << "RenderSystem::draw()" <<std::endl;
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	
	glBindFramebuffer(GL_FRAMEBUFFER, limited_vision_object_buffer);
	glClearBufferfv(GL_COLOR, 0, clear_color_value);
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	
	// clear backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	
	// Check which screen user is currently on and change background accordingly
	switch(game_screen) {
		case GAME_SCREEN::START:
			glClearColor(0.9f, 0.45f, 0.4f, 1.0f);
			glfwSetWindowTitle(window, "START SCREEN");
			break;
        case GAME_SCREEN::TUTORIAL_PLAYING:
            glfwSetWindowTitle(window, "TUTORIAL PLAYING SCREEN");
            break;
		case GAME_SCREEN::STORY:
			glClearColor(0.9f, 0.45f, 0.4f, 1.0f);
			glfwSetWindowTitle(window, "STORY SCREEN");
			break;
		case GAME_SCREEN::TUTORIAL:
			glClearColor(0.4f, 0.2f, 0.7f, 1.0f);
			glfwSetWindowTitle(window, "TUTORIAL SCREEN");
			break;
		
		case GAME_SCREEN::SETTINGS:
			glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
			glfwSetWindowTitle(window, "SETTINGS SCREEN");
			break;

		case GAME_SCREEN::LEVEL_SELECT:
			glClearColor(0.5f, 0.1f, 0.1f, 1.0f);
			glfwSetWindowTitle(window, "LEVEL SELECT SCREEN");
			break;
		
		case GAME_SCREEN::PLAYING:
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			// glfwSetWindowTitle(window, "PLAYING SCREEN");
			// Render all popups
			break;
		
		case GAME_SCREEN::END:
			glClearColor(0.9f, 0.45f, 0.4f, 1.0f);
			glfwSetWindowTitle(window, "END SCREEN");
			break;
		
		default:
			break;
	}

	// glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferuiv(GL_COLOR, 1, clear_object_id_value);
	glClearBufferfv(GL_COLOR, 2, clear_color_value);
	glClearBufferfv(GL_COLOR, 3, clear_color_value);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
								// and alpha blending, one would have to sort
								// sprites back to front
	gl_has_errors();

    mat3 screen_projection_2D = createProjectionMatrix();

	// Check if screen is PLAYING and no popups need to be rendered
	if (game_screen == GAME_SCREEN::PLAYING || game_screen == GAME_SCREEN::TUTORIAL_PLAYING) {
		// Set the position and normal buffers to overwrite prior values as they are written to
		glBlendFunci(2, GL_ONE, GL_ZERO);
		glBlendFunci(3, GL_ONE, GL_ZERO);

		// Check if player and map exists
		// Get the current map level (get this from some sort of state from the game)
		// FOR THE TIME BEING, MAP LEVEL IS HARD CODED
		Entity map_entity = registry.maps.entities[0];
		Map& map = registry.maps.get(map_entity);

		Entity player = registry.players.entities[0];
		Motion& player_motion = registry.motions.get(player);

		// Dependent on MAP SIZE and GRID SIZE
		// Get number of rows and columns from map
		float max_cam_pos_x = (map.num_cols * GRID_CELL_WIDTH_PX) - (WINDOW_WIDTH_PX/2.0f);
		float max_cam_pos_y = (map.num_rows * GRID_CELL_HEIGHT_PX) - (WINDOW_HEIGHT_PX/2.0f);

		vec2 cam_pos = { std::clamp(player_motion.position.x, WINDOW_WIDTH_PX/2.0f, max_cam_pos_x),
						 std::clamp(player_motion.position.y, WINDOW_HEIGHT_PX/2.0f, max_cam_pos_y) };
		mat3 projection_2D = createProjectionMatrix(cam_pos);
		
		player_screen_position = (player_motion.position-cam_pos);
		player_screen_position = vec2{ player_screen_position.x/(float)WINDOW_WIDTH_PX+0.5f,
									  -player_screen_position.y/(float)WINDOW_HEIGHT_PX+0.5f };
		player_world_position = player_motion.position;

		// Draw floor texture for current level
		for (Entity entity : registry.floors.entities) {
			if (registry.renderRequests.has(entity)) {
				drawTexturedMesh(entity, projection_2D);
			}
		}

        float half_width = WINDOW_WIDTH_PX / 2.f;
        float half_height = WINDOW_HEIGHT_PX / 2.f;
        vec2 world_min = cam_pos - vec2(half_width, half_height);
        vec2 world_max = cam_pos + vec2(half_width, half_height);

		// Draw all wallblocks
		for (Entity entity : registry.wallBlocks.entities) {
			if (registry.motions.has(entity) && registry.renderRequests.has(entity)) {
                Motion& motion = registry.motions.get(entity);
                vec2 half_scale = motion.scale / 2.f;
                vec min_pos = motion.position - half_scale;
                vec2 max_pos = motion.position + half_scale;
                if (max_pos.x < world_min.x || min_pos.x > world_max.x || max_pos.y < world_min.y || min_pos.y > world_max.y) continue;
				drawTexturedMesh(entity, projection_2D);
			}
		}

		// Render to limited vision framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, limited_vision_object_buffer);

		// Clear the limited_vision_object_color_texture and object IDs
		glClearBufferfv(GL_COLOR, 0, clear_color_value);
		glClearBufferuiv(GL_COLOR, 1, clear_object_id_value);
		
		GameState& game_state = registry.game_state.components[0];
		
		// Draw all ingredients
		for (Entity entity : registry.ingredients.entities) {
			if (registry.motions.has(entity) && registry.renderRequests.has(entity)) {
                Motion& motion = registry.motions.get(entity);
                vec2 half_scale = motion.scale / 2.f;
                vec min_pos = motion.position - half_scale;
                vec2 max_pos = motion.position + half_scale;
                if (max_pos.x < world_min.x || min_pos.x > world_max.x || max_pos.y < world_min.y || min_pos.y > world_max.y) continue;
                
				if (registry.stages.has(entity)) {
					Stage& stage = registry.stages.get(entity);
					if (stage.value == game_state.cur_stage) {
						drawTexturedMesh(entity, projection_2D);
					}
				} else {
					drawTexturedMesh(entity, projection_2D);
				}
			}
		}
		
		// Draw all powerups
		for (Entity entity : registry.powerups.entities) {
			if (registry.motions.has(entity) && registry.renderRequests.has(entity)) {
				drawTexturedMesh(entity, projection_2D, 4);
			}
		}

		// Draw all meshes
		for (Entity entity : registry.meshPtrs.entities) {
			if (registry.motions.has(entity) && registry.renderRequests.has(entity)) {
                Motion& motion = registry.motions.get(entity);
                vec2 half_scale = motion.scale / 2.f;
                vec min_pos = motion.position - half_scale;
                vec2 max_pos = motion.position + half_scale;
                if (max_pos.x < world_min.x || min_pos.x > world_max.x || max_pos.y < world_min.y || min_pos.y > world_max.y) continue;

				RenderRequest& r = registry.renderRequests.get(entity);
				if (r.used_effect == EFFECT_ASSET_ID::MESH) {
					drawTexturedMesh(entity, projection_2D);
				}
			}
		}
		
		// Draw all enemies
		for (Entity entity : registry.enemies.entities) {
			if (registry.motions.has(entity) && registry.renderRequests.has(entity)) {
                Motion& motion = registry.motions.get(entity);
                vec2 half_scale = motion.scale / 2.f;
                vec min_pos = motion.position - half_scale;
                vec2 max_pos = motion.position + half_scale;
                if (max_pos.x < world_min.x || min_pos.x > world_max.x || max_pos.y < world_min.y || min_pos.y > world_max.y) continue;
				drawTexturedMesh(entity, projection_2D, 5);
			}
		}

		// Clear fire lighting buffer
		glClearBufferfv(GL_COLOR, 2, clear_color_value);
		// Additive blending so that fire on top of any entity additively blends with it
		glBlendFunc(GL_ONE, GL_ONE);
		// Additive blending so that fire lighting gradients don't overwrite each other
		glBlendFunci(2, GL_ONE, GL_ONE);
		
		// Draw all fire blocks
		for (Entity entity : registry.fireBlocks.entities) {
			if (registry.motions.has(entity) && registry.renderRequests.has(entity)) {
                Motion& motion = registry.motions.get(entity);
                vec2 half_scale = motion.scale / 2.f;
                vec min_pos = motion.position - half_scale;
                vec2 max_pos = motion.position + half_scale;
                if (max_pos.x < world_min.x || min_pos.x > world_max.x || max_pos.y < world_min.y || min_pos.y > world_max.y) continue;
                
				drawTexturedMesh(entity, projection_2D, 1);
			}
		}

		// Draw all players
		for (Entity entity : registry.players.entities) {
			if (registry.motions.has(entity) && registry.renderRequests.has(entity)) {
				drawTexturedMesh(entity, projection_2D);
			}
		}

		// Set blend function to this for correct blending between smoke and default framebuffer
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		
		// Handle all instances
		for (InstanceRequest &instance_request : registry.instanceRequests.components)
		{
            // Update instance buffer data with new transforms
            updateInstanceDataVBO(instance_request.texture, instance_request.items);

            // Perform instanced draw
            drawTexturedInstance(projection_2D, instance_request);
        }

		for (Entity entity : registry.highlightBlocks.entities) {
			drawTexturedMesh(entity, projection_2D);
		}

        // (TODO: render ui on top of entire game)
        // First go through all boxes and huds
        // Next go through all motions and huds

		// for (Entity e : registry.hud.entities) {
		// 	if (registry.boxes.has(e)) {
		// 		drawBox(e, screen_projection_2D);
		// 	} else if (registry.motions.has(e)) {
		// 		drawTexturedMesh(e, screen_projection_2D);
		// 	}
		// }


    }

	drawToScreen(game_screen);

	// Enable blending to render UI elements on top properly
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	if (game_screen == GAME_SCREEN::PLAYING || game_screen == GAME_SCREEN::TUTORIAL_PLAYING) {
		for (Entity e : registry.hud.entities) {
			if (registry.boxes.has(e)) {
				drawBox(e, screen_projection_2D);
			} else if (registry.motions.has(e)) {
				drawTexturedMesh(e, screen_projection_2D);
			}
		}
	}
    
    // Draw screen entities that are NOT hud
    for (Entity entity : registry.screens.entities) {
        GameScreen& screen = registry.screens.get(entity);
        if ((screen.screen == game_screen) && registry.renderRequests.has(entity) && !registry.hud.has(entity) && !registry.popups.has(entity)) {
            // First render any boxes, then render textured meshes
            if (registry.boxes.has(entity)) {
                drawBox(entity, screen_projection_2D);
            } else if (registry.motions.has(entity)) {
                drawTexturedMesh(entity, screen_projection_2D);
            }
            
        }
    }

	// Render all text requests on current screen
	font_renderer.use(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
	for (Entity e : registry.textRenderRequests.entities)	{
        if (registry.screens.has(e)) {
            GameScreen& screen = registry.screens.get(e);
            if (screen.screen == game_screen) {
                TextRenderRequest& text = registry.textRenderRequests.get(e);
                font_renderer.render(text);
            }
        }
	}

    // Special render case for settings help popup
    for (Entity e : registry.popups.entities) {
        if (registry.screens.has(e)) {
            GameScreen& screen = registry.screens.get(e);
            if (screen.screen == game_screen) {
                if (registry.boxes.has(e)) {
                    drawBox(e, screen_projection_2D);
                } else if (registry.motions.has(e)) {
                    drawTexturedMesh(e, screen_projection_2D);
                } else if (registry.textRenderRequests.has(e)) {
                    TextRenderRequest& text = registry.textRenderRequests.get(e);
                    font_renderer.render(text);
                }
            }
        }
    }
	
	// draw framebuffer to screen
	// adding "vignette" effect when applied
	// drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// fake projection matrix, scaled to window coordinates
	float left   = 0.f;
	float top    = 0.f;
	float right  = (float) WINDOW_WIDTH_PX;
	float bottom = (float) WINDOW_HEIGHT_PX;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {
		{ sx, 0.f, 0.f},
		{0.f,  sy, 0.f},
		{ tx,  ty, 1.f}
	};
}

mat3 RenderSystem::createProjectionMatrix(vec2 position)
{
	// fake projection matrix, scaled to window coordinates
	float left   = 0.f;
	float top    = 0.f;
	float right  = (float) WINDOW_WIDTH_PX;
	float bottom = (float) WINDOW_HEIGHT_PX;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(position.x*2) / WINDOW_WIDTH_PX;
	float ty = (position.y*2) / WINDOW_HEIGHT_PX;

	return {
		{ sx, 0.f, 0.f},
		{0.f,  sy, 0.f},
		{ tx,  ty, 1.f}
	};
}