#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vertexData{
    vec2 uv_coord;
    vec3 normal_vec;
    vec3 render_position;
}vertices[];

out fragmentData{
    vec2 uv_coord;
    vec3 normal_vec;
    vec3 render_position;
}fragment;

void main() {
    int i;
    for (i = 0; i < 3; i++){
        fragment.uv_coord = vertices[i].uv_coord;
        fragment.normal_vec = vertices[i].normal_vec;
        fragment.render_position = vertices[i].render_position;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}