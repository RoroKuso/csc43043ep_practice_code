
#include "terrain.hpp"
#include "tree.hpp"


using namespace cgp;

// Evaluate 3D position of the terrain for any (x,y)
float evaluate_terrain_height(float x, float y, float terrain_length, perlin_noise_parameters const& parameters)
{

    vec2 p_i[4] = { {-10,-10}, {5,5}, {-3,4}, {6,4} };
    float h_i[4] = {3.0f, -1.5f, 1.0f, 2.0f};
    float sigma_i[4] = {10, 3, 4, 4};
    float z = 0.0f;
    for (int i = 0; i < 4; i++) {
        float d_i = norm(vec2(x, y) - p_i[i]) / sigma_i[i];
        z += h_i[i] * std::exp(-d_i * d_i);
    }
    const float u = (x / terrain_length) + 0.5f;
    const float v = (y / terrain_length) + 0.5f;
    float const noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, parameters.frequency_gain);
    return z + parameters.terrain_height*noise;
}


mesh create_terrain_mesh(int N, float terrain_length, perlin_noise_parameters const& parameters)
{

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);
    terrain.uv.resize(N*N);

    // Fill terrain geometry
    for(int ku=0; ku<N; ++ku)
    {
        for(int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            float u = ku/(N-1.0f);
            float v = kv/(N-1.0f);

            // Compute the real coordinates (x,y) of the terrain in [-terrain_length/2, +terrain_length/2]
            float x = (u - 0.5f) * terrain_length;
            float y = (v - 0.5f) * terrain_length;

            // Compute the surface height function at the given sampled coordinate
            float z = evaluate_terrain_height(x,y, terrain_length, parameters);

            // Store vertex coordinates
            terrain.position[kv+N*ku] = {x,y,z};
            terrain.uv[kv + N*ku] = {5*u, 5*v};
        }
    }

    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    for(int ku=0; ku<N-1; ++ku)
    {
        for(int kv=0; kv<N-1; ++kv)
        {
            unsigned int idx = kv + N*ku; // current vertex offset

            uint3 triangle_1 = {idx, idx+1+N, idx+1};
            uint3 triangle_2 = {idx, idx+N, idx+1+N};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

    // need to call this function to fill the other buffer with default values (normal, color, etc)
	terrain.fill_empty_field(); 

    return terrain;
}

std::vector<cgp::vec3> generate_positions_on_terrain(int N_tree, float terrain_length, perlin_noise_parameters const& parameters) {
    std::vector<cgp::vec3> ret;
    for (int i = 0; i < N_tree; i++) {
        float x = rand_uniform(-terrain_length/2, terrain_length/2);
        float y = rand_uniform(-terrain_length/2, terrain_length/2);
        float z = evaluate_terrain_height(x, y, terrain_length, parameters);
        // float z = evaluate_terrainmesh_height(terrain, x, y, N_terrain, terrain_length);
        ret.push_back(vec3{x, y, z});
    }
    return ret;
}
