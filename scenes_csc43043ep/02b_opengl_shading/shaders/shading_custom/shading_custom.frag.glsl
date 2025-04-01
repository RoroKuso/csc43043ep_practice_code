#version 330 core


// Inputs coming from the vertex shader
in struct fragment_data
{
    vec3 position; // position in the world space
    vec3 normal;   // normal in the world space
    vec3 color;    // current color on the fragment
    vec2 uv;       // current uv-texture on the fragment

} fragment;

// Output of the fragment shader - output color
layout(location=0) out vec4 FragColor;

// View matrix
uniform mat4 view;

struct material_structure
{
	vec3 color;  // Uniform color of the object
};
uniform material_structure material;

// Ambiant uniform controled from the GUI
uniform float ambiant;
uniform float diffuse;
uniform float exp_specular;
uniform float specular;
uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 fog_color;



void main()
{
    mat3 O = transpose(mat3(view)); // get the orientation matrix
    vec3 last_col = vec3(view * vec4(0.0, 0.0, 0.0, 1.0)); // get the last column
    vec3 camera_position = -O * last_col;

    
    // Light decreasing with distance effect
    vec3 new_light_color;
    float d_light = length(light_position - fragment.position);
    float d_lmax = 4.0f;
    float decrease_scale = min(d_light / d_lmax, 1.0f);
    new_light_color = (1.0f - decrease_scale) * light_color;

    vec3 u_light = normalize(light_position - fragment.position);
    vec3 n = normalize(fragment.normal);
    vec3 u_reflect = reflect(-u_light, n);
    vec3 u_view = normalize(camera_position - fragment.position);

    vec3 C_diffuse_ambiant = (ambiant + diffuse * (max(0.0f, dot(n, u_light)))) * material.color * new_light_color;
    vec3 C_specular = specular * pow(max(0.0f, dot(u_reflect, u_view)), exp_specular) * new_light_color;
    vec3 phong_color = C_diffuse_ambiant + C_specular;

    // Fog effect
    float d = length(camera_position - fragment.position);
    float d_max = 12.0f;
    float fog_scale = min(1.0, d / d_max);
    vec3 fog_effect = (1.0 - fog_scale) * phong_color + fog_scale * fog_color;

    vec3 current_color = fog_effect;
	FragColor = vec4(current_color, 1.0); 	// Note: the last alpha component is not used here

}