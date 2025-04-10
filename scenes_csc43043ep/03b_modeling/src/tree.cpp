#include "tree.hpp"

using namespace cgp;


mesh create_cylinder_mesh(float radius, float height)
{
    // To do: fill this mesh ...
    mesh tree;
    int N = 20;
    tree.position.clear();

    for(int t=0; t<N; ++t)
    {
        float angle1 = 2.0f * (float(t) / N) * M_PI;
        float x1 = radius * cos(-angle1);
        float y1 = radius * sin(-angle1);
        tree.position.push_back(vec3{x1, y1, 0.0f});
        tree.position.push_back(vec3{x1, y1, height});
    }

    for(int t=0; t < N; ++t) {
        uint3 triangle_1 = {2*t, (2*t+1)%(2*N), (2*t+2)%(2*N)};
        uint3 triangle_2 = {(2*t+2)%(2*N), (2*t+1)%(2*N), (2*t+3)%(2*N)};
        tree.connectivity.push_back(triangle_1);
        tree.connectivity.push_back(triangle_2);
    }


    // Need to call fill_empty_field() before returning the mesh 
    //  this function fill all empty buffer with default values (ex. normals, colors, etc).
    tree.fill_empty_field();

    return tree;
}

mesh create_cone_mesh(float radius, float height, float z_offset)
{
    mesh m; 
    int N = 20;
    m.position.clear();

    // Base
    for (int k = 0; k < N; k++) {
        float angle1 = 2.0f * (float(k) / N) * M_PI;
        float x1 = radius * cos(-angle1);
        float y1 = radius * sin(-angle1);
        m.position.push_back(vec3{x1, y1, z_offset});
    }
    m.position.push_back(vec3{0.0f, 0.0f, z_offset});
    m.position.push_back(vec3{0.0f, 0.0f, z_offset + height});

    // Connectivity Base
    for(int t=0; t < N; ++t) {
        uint3 triangle_1 = {N, t, (t+1)%N};
        uint3 triangle_2 = {N, (t+1)%N, (t+2)%N};
        m.connectivity.push_back(triangle_1);
        m.connectivity.push_back(triangle_2);
    }
    // Connectivity Ceiling
    for(int t=0; t < N; ++t) {
        uint3 triangle_1 = {t, N+1, (t+1)%N};
        uint3 triangle_2 = {(t+1)%N, N+1, (t+1)%N};
        m.connectivity.push_back(triangle_1);
        m.connectivity.push_back(triangle_2);
    }

    m.fill_empty_field();
    return m;
}


mesh create_tree()
{
    float h = 0.7f; // trunk height
    float r = 0.1f; // trunk radius

    // Create a brown trunk
    mesh trunk = create_cylinder_mesh(r, h);
    trunk.color.fill({0.4f, 0.3f, 0.3f});


    // Create a green foliage from 3 cones
    mesh foliage = create_cone_mesh(4*r, 6*r, 0.0f);      // base-cone
    foliage.push_back(create_cone_mesh(4*r, 6*r, 2*r));   // middle-cone
    foliage.push_back(create_cone_mesh(4*r, 6*r, 4*r));   // top-cone
    foliage.translate({0,0,h});       // place foliage at the top of the trunk
    foliage.color.fill({0.4f, 0.6f, 0.3f});
       
    // The tree is composed of the trunk and the foliage
    mesh tree = trunk;
    tree.push_back(foliage);

    return tree;
}

// void update_tree_coord(mesh& terrain, float x, float y) {
    
// }