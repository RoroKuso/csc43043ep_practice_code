#include "scene.hpp"

#include "terrain.hpp"
#include "tree.hpp"
#include "interpolation.hpp"

using namespace cgp;


// Spring force applied on particle p_i with respect to position p_j.
vec3 spring_force(vec3 const& p_i, vec3 const& p_j, float L0, float K)
{
	vec3 const p = p_i - p_j;
	float const L = norm(p);
	vec3 const u = p / L;

	vec3 const F = -K * (L - L0) * u;
	return F;
}

void scene_structure::simulation_step(float dt)
{
	// Simulation parameters
	float m = 0.01f;       // particle mass
	float K = 5.0f;        // spring stiffness
	float mu = 0.08f;      // damping coefficient

	vec3 g = { 0,0,-9.81f }; // gravity

	std::vector<vec3> newpoints(points.size());
	std::vector<vec3> newspeeds(speeds.size());

	for (int i = 0; i < points.size()-2; i++) {
		vec3 f_spring1 = spring_force(points[i+1], points[i], L0s[i], K);
		vec3 f_spring2 = spring_force(points[i+1], points[i+2], L0s[i+2], K);
		vec3 f_weight = m * g;
		vec3 f_damping = -mu * speeds[i+1];
		vec3 f = f_spring1 + f_spring2 + f_weight + f_damping;

		newspeeds[i+1] = speeds[i+1] + dt * f / m;
		newpoints[i+1] = points[i+1] + dt * speeds[i+1];
	}

	for (int i = 1; i < points.size()-1; i++) {
		speeds[i] = newspeeds[i];
		points[i] = newpoints[i];
	}
}

void scene_structure::draw_segment(vec3 const& a, vec3 const& b)
{
	segment.vbo_position.update(numarray<vec3>{ a, b });
	draw(segment, environment);
}

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

	// Oiseau
	mesh_drawable corps_base;
	mesh_drawable head_base;
	mesh_drawable left_wing_base;
	mesh_drawable left_wing_son;
	mesh_drawable right_wing_base;
	mesh_drawable right_wing_son;
	mesh_drawable right_eye;
	mesh_drawable left_eye;
	mesh_drawable bec;
	
	corps_base.initialize_data_on_gpu(mesh_primitive_ellipsoid({1, 0.5f, 0.5f}));
	head_base.initialize_data_on_gpu(mesh_primitive_sphere(0.3f));
	left_wing_base.initialize_data_on_gpu(mesh_primitive_quadrangle({-0.5f,0,0}, {-0.5f,1,0}, {0.5f,1,0}, {0.5f,0,0}));
	right_wing_base.initialize_data_on_gpu(mesh_primitive_quadrangle({-0.5,0,0}, {-0.5,-1,0}, {0.5f,-1,0}, {0.5f,0,0}));
	left_wing_son.initialize_data_on_gpu(mesh_primitive_quadrangle({-0.5f,0,0}, {0.5f,0,0}, {0.25f,0.5f,0}, {-0.25f,0.5f,0}));
	right_wing_son.initialize_data_on_gpu(mesh_primitive_quadrangle({-0.5f,0,0}, {0.5f,0,0}, {0.25f,0.5f,0}, {-0.25f,0.5f,0}));
	right_wing_son.model.rotation = rotation_transform::from_axis_angle({1,0,0}, 3.14f);

	right_wing_son.model.rotation = rotation_transform::from_axis_angle({1,0,0}, 3.14f);
	hierarchy.add(corps_base, "Corps base");
	hierarchy.add(head_base, "Head base", "Corps base", {1.0f, 0, 0.5f});
	hierarchy.add(left_wing_base, "Left wing base", "Corps base", {0.0f, 0.2f, 0.0f});
	hierarchy.add(right_wing_base, "Right wing base", "Corps base", {0.0f, -0.2f, 0.0f});
	hierarchy.add(left_wing_son, "Left wing son", "Left wing base", {0, 1, 0});
	hierarchy.add(right_wing_son, "Right wing son", "Right wing base", {0, -1, 0});

	// Key 3D positions
	numarray<vec3> key_positions = 
	{ {-1,1,5}, {0,1,5}, {1,1,5}, {1,2,5}, {2,2,5}, {2,2,5}, {2,0,5}, {1.5,-1,5}, {1.5,-1,5}, {1,-1,5}, {0,-0.5,5}, {-1,-0.5,5} };

	// Key times (time at which the position must pass in the corresponding position)
	numarray<float> key_times = 
	{ 0.0f, 1.0f, 2.0f, 2.5f, 3.0f, 3.5f, 3.75f, 4.5f, 5.0f, 6.0f, 7.0f, 8.0f };

	// Initialize the helping structure to display/interact with these positions
	keyframe.initialize(key_positions, key_times);


	// Set timer bounds
	// The timer must span a time interval on which the interpolation can be conducted
	// By default, set the minimal time to be key_times[1], and the maximal time to be key_time[N-2] (enables cubic interpolation)
	int N = key_times.size();
	timer.t_min = key_times[1];
	timer.t_max = key_times[N - 2];
	timer.t = timer.t_min;

	int N_spheres = 20;
	float z = evaluate_terrain_height(0, 0, terrain_length, parameters);
	points.push_back( {0, 0, z} );
	speeds.push_back( {0, 0, 0} );
	L0s.push_back(0.05f);
	for (int n = 1; n < N_spheres; n++) {
		points.push_back( {0, n*0.01f, 0} );
		speeds.push_back( {0, 0, 0} );
		L0s.push_back(0.05f);
	}
	points.push_back(hierarchy["Corps base"].drawable.model.translation);
	speeds.push_back( {0, 0, 0} );
	L0s.push_back(0.3f);

	particle_sphere.initialize_data_on_gpu(mesh_primitive_sphere(0.05f));
	segment.display_type = curve_drawable_display_type::Segments;
	segment.initialize_data_on_gpu({ {0,0,0},{1,0,0} });

}

void scene_structure::display_frame()
{
	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();

	// Update the current time
	timer.update();
	float t = timer.t;

	// clear trajectory when the timer restart
	if (t < timer.t_min + 0.1f)
		keyframe.trajectory.clear();

	// Display the key positions and lines b/w positions
	keyframe.display_key_positions(environment);

	// Compute the interpolated position
	//  This is this function that you need to complete
	vec3 p = interpolation(t, keyframe.key_positions, keyframe.key_times);

	// Display the interpolated position (and its trajectory)
	keyframe.display_current_position(p, environment);
	
	if (gui.display_frame)
		draw(global_frame, environment);

	// Update the current time
	timer.update();

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

	hierarchy["Corps base"].transform_local.translation = p;
	hierarchy["Left wing base"].transform_local.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, (3.14f / 4.0f) * cos(4*timer.t));
	hierarchy["Right wing base"].transform_local.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, -(3.14f / 4.0f) * cos(4*timer.t));
	hierarchy["Left wing son"].transform_local.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, (3.14f / 4.0f) * cos(4*timer.t));
	hierarchy["Right wing son"].transform_local.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, -(3.14f / 4.0f) * cos(4*timer.t));

	// This function must be called before the drawing in order to propagate the deformations through the hierarchy
	hierarchy.update_local_to_global_coordinates();

	// Draw the hierarchy as a single mesh
	draw(hierarchy, environment);
	if (gui.display_wireframe)
		draw_wireframe(hierarchy, environment);

	// Spring
	timer_b.update();
	points[points.size()-1] = hierarchy["Corps base"].transform_local.translation;
	for (int i = 0; i < 10; i++)
		simulation_step(timer_b.scale * 0.001f);

	for (int i = 0 ; i < points.size(); i++) {
		if (points[i].z < evaluate_terrain_height(points[i].x, points[i].y, 20.0f, parameters)) {
			points[i].z = evaluate_terrain_height(points[i].x, points[i].y, 20.0f, parameters);
		}
		particle_sphere.model.translation = points[i];
		particle_sphere.material.color = { 1,0,0 };
		draw(particle_sphere, environment);	
	}

	for (int i = 0; i < points.size()-1; i++)
		draw_segment(points[i], points[i+1]);

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
