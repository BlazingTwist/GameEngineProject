#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 uv_coord;
layout(location = 2) in vec3 normal_vec;

out vertexData{
    vec2 uv_coord;
    vec3 normal_vec;
    vec3 world_position;
}vertex;

uniform mat4 object_to_world_matrix;
uniform mat4 world_to_camera_matrix;

void main()
{
    vec4 worldPosition = object_to_world_matrix * vec4(in_position, 1.0f);
    gl_Position = world_to_camera_matrix * worldPosition;

    vertex.uv_coord = uv_coord;
    vertex.normal_vec = normalize(vec3(object_to_world_matrix * vec4(normal_vec, 1.0f)));
    vertex.world_position = worldPosition.xyz;
}