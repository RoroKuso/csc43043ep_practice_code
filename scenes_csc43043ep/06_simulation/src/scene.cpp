#include "scene.hpp"


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
	camera_control.look_at({ 3.0f, 2.0f, 2.0f }, { 0,0,0 }, { 0,0,1 });

	display_info();
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());

	int N = 10;
	for (int n = 0; n < N; n++) {
		points.push_back( {0, n*0.01f, 0} );
		speeds.push_back( {0, 0, 0} );
		L0s.push_back(0.3f);
	}
	points.push_back( {0, 2, 0} );
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
	
	if (gui.display_frame)
		draw(global_frame, environment);

	// Update the current time
	timer.update();

	for (int i = 0; i < 10; i++)
		simulation_step(timer.scale * 0.001f);

	for (auto point: points) {
		particle_sphere.model.translation = point;
		particle_sphere.material.color = { 1,0,0 };
		draw(particle_sphere, environment);	
	}

	for (int i = 0; i < points.size()-1; i++)
		draw_segment(points[i], points[i+1]);
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