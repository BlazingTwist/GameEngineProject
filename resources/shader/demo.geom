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
    vec3 render_position;
    mat3 tbn_matrix;
}fragment;

uniform mat4 object_to_world_matrix;

void main() {
    // calculate tangent space tangent and bi-tangent
    // source: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    vec3 edge01 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 edge02 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec2 deltaUV01 = vertices[1].uv_coord - vertices[0].uv_coord;
    vec2 deltaUV02 = vertices[2].uv_coord - vertices[0].uv_coord;
    float inverseDeterminant = 1.0f / (deltaUV01.x * deltaUV02.y - deltaUV02.x * deltaUV01.y);

    vec3 object_tangent = inverseDeterminant * (deltaUV02.y *  edge01 - deltaUV01.y * edge02);
    vec3 object_bi_tangent = inverseDeterminant * (-deltaUV02.x * edge01 + deltaUV01.x * edge02);
    vec3 tangent = normalize(vec3(object_to_world_matrix * vec4(object_tangent, 0.0f)));
    vec3 bi_tangent = normalize(vec3(object_to_world_matrix * vec4(object_bi_tangent, 0.0f)));

    fragment.uv_coord = vertices[0].uv_coord;
    fragment.render_position = vertices[0].render_position;
    fragment.tbn_matrix = mat3(tangent, bi_tangent, normalize(vec3(object_to_world_matrix * vec4(vertices[0].normal_vec, 0.0f))));
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    fragment.uv_coord = vertices[1].uv_coord;
    fragment.render_position = vertices[1].render_position;
    fragment.tbn_matrix = mat3(tangent, bi_tangent, normalize(vec3(object_to_world_matrix * vec4(vertices[1].normal_vec, 0.0f))));
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    fragment.uv_coord = vertices[2].uv_coord;
    fragment.render_position = vertices[2].render_position;
    fragment.tbn_matrix = mat3(tangent, bi_tangent, normalize(vec3(object_to_world_matrix * vec4(vertices[2].normal_vec, 0.0f))));
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}