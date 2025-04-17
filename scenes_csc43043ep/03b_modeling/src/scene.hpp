#pragma once


#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "terrain.hpp"
#include "key_positions_structure.hpp"

// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// Variables associated to the GUI
struct gui_parameters {
	bool display_frame = true;
	bool display_wireframe = false;
};

// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	camera_controller_orbit_euler camera_control;
	camera_projection_perspective camera_projection;
	window_structure window;

	mesh_drawable global_frame;          // The standard global frame
	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	gui_parameters gui;                  // Standard GUI element storage
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	cgp::mesh terrain_mesh;
	cgp::mesh_drawable terrain;
	cgp::mesh_drawable tree;
	cgp::mesh_drawable quad;
	std::vector<cgp::vec3> tree_positions;
	std::vector<cgp::vec3> grass_positions;
	perlin_noise_parameters parameters;

	cgp::hierarchy_mesh_drawable hierarchy;
	cgp::timer_interval timer;
	keyframe_structure keyframe;

	timer_basic timer_b;
	mesh_drawable particle_sphere;
	curve_drawable segment;
	void simulation_step(float dt);
	void draw_segment(vec3 const& a, vec3 const& b);
	// Particles:
	std::vector<vec3> points;
	std::vector<vec3> speeds;
	std::vector<float> L0s;


	// ****************************** //
	// Functions
	// ****************************** //

	void initialize();    // Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop
	void display_semiTransparent();

	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();

	void display_info();
};





