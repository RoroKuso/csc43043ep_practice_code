#include "scene.hpp"

#include "terrain.hpp"
#include "tree.hpp"

using namespace cgp;




void scene_structure::initialize()
{
	camera_control.initialize(inputs, window); // Give access to the inputs and window global state to the camera controler
	camera_control.set_rotation_axis_z();
	camera_control.look_at({ 15.0f,6.0f,6.0f }, {0,0,0});
	

	// General information
	display_info();

	global_frame.initialize_data_on_gpu(mesh_primitive_frame());

	int N_terrain_samples = 1000;
	float terrain_length = 20.0f;
	terrain_mesh = create_terrain_mesh(N_terrain_samples, terrain_length, parameters);
	terrain.initialize_data_on_gpu(terrain_mesh);
	terrain.material.color = { 0.6f,0.85f,0.5f };
	terrain.material.phong.specular = 0.0f; // non-specular terrain material

	// update_terrain(terrain_mesh, terrain, parameters);

	mesh const tree_mesh = create_tree();
	tree.initialize_data_on_gpu(tree_mesh);
	tree_positions = generate_positions_on_terrain(30, terrain_length, parameters);

	mesh quad_mesh = mesh_primitive_quadrangle({ -0.5f,0,0 }, { 0.5f,0,0 }, { 0.5f,0,1 }, { -0.5f,0,1 });
	quad.initialize_data_on_gpu(quad_mesh);
	quad.texture.load_and_initialize_texture_2d_on_gpu(project::path+"assets/grass.png");
	quad.material.phong = { 0.4f, 0.6f,0,1 };
	grass_positions = generate_positions_on_terrain(50, terrain_length, parameters);

	terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/texture_grass.jpg",
		GL_REPEAT,
		GL_REPEAT);

}

void scene_structure::display_frame()
{
	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	
	if (gui.display_frame)
		draw(global_frame, environment);

	draw(terrain, environment);
	
	for (auto pos: tree_positions) {
		tree.model.translation = pos;
		draw(tree, environment);
	}
	if (gui.display_wireframe) {
		draw_wireframe(terrain, environment);
		draw_wireframe(tree, environment);
		draw_wireframe(quad, environment);
	}

	// Semi transparent objects
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false);
	auto const& camera = camera_control.camera_model;
	vec3 const right = camera.right();
	rotation_transform R = rotation_transform::from_frame_transform({ 1,0,0 }, { 0,0,1 }, right, { 0,0,1 });
	quad.model.rotation = R;
	for (auto pos: grass_positions) {
		quad.model.translation = pos;
		draw(quad, environment);
	}
	glDepthMask(true);
	glDisable(GL_BLEND);
}

void scene_structure::display_semiTransparent()
{
	// Enable use of alpha component as color blending for transparent elements
	//  alpha = current_color.alpha
	//  new color = previous_color * alpha + current_color * (1-alpha)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Disable depth buffer writing
	//  - Transparent elements cannot use depth buffer
	//  - They are supposed to be display from furest to nearest elements
	glDepthMask(false);

	auto const& camera = camera_control.camera_model;

	// Re-orient the grass shape to always face the camera direction
	vec3 const right = camera.right();
	// Rotation such that the grass follows the right-vector of the camera, while pointing toward the z-direction
	rotation_transform R = rotation_transform::from_frame_transform({ 1,0,0 }, { 0,0,1 }, right, { 0,0,1 });
	quad.model.rotation = R;


	// // Sort transparent shapes by depth to camera
	// //   This step can be skipped, but it will be associated to visual artifacts

	// // Transform matrix (the same matrix which is applied in the vertices in the shader: T = Projection x View)
	// mat4 T = camera_projection.matrix() * camera.matrix_view();
	// // Projected vertices (center of quads) in homogeneous coordinates
	// vec4 p1 = T * vec4{ 0, -0.5f, 0, 1 };
	// vec4 p2 = T * vec4{ 0, +0.5f, 0, 1 };
	// // Depth to camera
	// float z1 = p1.z / p1.w;
	// float z2 = p2.z / p2.w;

	// // Display the quads relative to their depth
	// if (z1 <= z2) {
	// 	draw(quad_2, environment);
	// 	draw(quad_1, environment);
	// }
	// else {
	// 	draw(quad_1, environment);
	// 	draw(quad_2, environment);
	// }

	// Don't forget to re-activate the depth-buffer write
	glDepthMask(true);
	glDisable(GL_BLEND);
}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
}

void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click(environment.camera_view);
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
}

void scene_structure::display_info()
{
	std::cout << "\nCAMERA CONTROL:" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;
	std::cout << camera_control.doc_usage() << std::endl;
	std::cout << "-----------------------------------------------\n" << std::endl;
}
