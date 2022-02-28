#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vertexData{
    vec2 uv_coord;
    vec3 normal_vec;
    vec3 world_position;
}vertices[];

out fragmentData{
    vec2 uv_coord;
    vec3 world_position;
    mat3 tbn_matrix;
}fragment;

void main() {
    // calculate tangent space tangent and bi-tangent
    // source: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    vec3 edge01 = vertices[1].world_position.xyz - vertices[0].world_position.xyz;
    vec3 edge02 = vertices[2].world_position.xyz - vertices[0].world_position.xyz;
    vec2 deltaUV01 = vertices[1].uv_coord - vertices[0].uv_coord;
    vec2 deltaUV02 = vertices[2].uv_coord - vertices[0].uv_coord;
    float inverseDeterminant = 1.0f / (deltaUV01.x * deltaUV02.y - deltaUV02.x * deltaUV01.y);

    vec3 tangent = inverseDeterminant * (deltaUV02.y *  edge01 - deltaUV01.y * edge02);
    vec3 bi_tangent = inverseDeterminant * (-deltaUV02.x * edge01 + deltaUV01.x * edge02);

    fragment.uv_coord = vertices[0].uv_coord;
    fragment.world_position = vertices[0].world_position;
    fragment.tbn_matrix = mat3(tangent, bi_tangent, vertices[0].normal_vec);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    fragment.uv_coord = vertices[1].uv_coord;
    fragment.world_position = vertices[1].world_position;
    fragment.tbn_matrix = mat3(tangent, bi_tangent, vertices[1].normal_vec);
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    fragment.uv_coord = vertices[2].uv_coord;
    fragment.world_position = vertices[2].world_position;
    fragment.tbn_matrix = mat3(tangent, bi_tangent, vertices[2].normal_vec);
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}