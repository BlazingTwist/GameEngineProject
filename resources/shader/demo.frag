#version 450

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_SPOT 1
#define LIGHT_TYPE_POINT 2

in fragmentData{
    vec2 uv_coord;
    vec3 render_position;
    mat3 tbn_matrix;
}fragment;

layout(binding = 0) uniform sampler2D tx_color;
layout(binding = 1) uniform sampler2D tx_phong;// stores ambient/diffuse/specular/shinyness weights in r/g/b/a respectively
layout(binding = 2) uniform sampler2D tx_normal;// normal map in tangent space (r/g/b as right/up/out)
layout(binding = 3) uniform sampler2D tx_height;// stores height in range [0,1], 1 being on the surface, 0 being the maximum depth into the surface

layout(location = 0) out vec4 out_color;

uniform vec3 camera_position;
uniform vec3 ambient_light;

// TODO figure out what to do about multiple light sources (ideally with variable length - "Shader Storage Buffer Objects" ? )
uniform int light_type;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_range;
uniform float light_spot_angle;
uniform vec3 light_color;
uniform float light_intensity;

void main()
{
    vec4 texColor = texture(tx_color, fragment.uv_coord);
    vec4 phongData = texture(tx_phong, fragment.uv_coord);
    vec3 normal_vec = normalize(fragment.tbn_matrix * (texture(tx_normal, fragment.uv_coord).xyz * 2.0f - 1.0f));
    vec3 ambientColor = max(phongData.r * ambient_light, 0.0f);
    vec3 diffuseColor = vec3(0.0f, 0.0f, 0.0f);
    vec3 specularColor = vec3(0.0f, 0.0f, 0.0f);

    // TODO light bounces? occlusion?
    // TODO rewrite x/distance stuff to use (1+(x/distance)) instead of clamping (?).
    if (light_type == LIGHT_TYPE_DIRECTIONAL){
        // direction, color, intensity

        float diffuseDot = max(dot(-light_direction, normal_vec), 0.0f);
        float specularDot = max(dot(reflect(light_direction, normal_vec), normalize(camera_position - fragment.render_position)), 0.0f);
        diffuseColor = phongData.g * diffuseDot * light_color * light_intensity;
        specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity;

    } else if (light_type == LIGHT_TYPE_SPOT){
        // position, direction, range, spot_angle, color, intensity
        vec3 spotLightDirection = normalize(fragment.render_position - light_position);
        // here we're (ab-)using the fact that the dot product of two normalized vectors is the cosine of their angle
        float spotLightAngle = dot(light_direction, spotLightDirection); 
        float spotLightDistance = length(fragment.render_position - light_position);
        if (spotLightAngle > light_spot_angle && spotLightDistance <= light_range){
            float distanceFactor = min(0.01f / pow(spotLightDistance / light_range, 2), 4); // 4 is the distanceFactor at 'distance = 0.05 * range'
            float diffuseDot = max(dot(-spotLightDirection, normal_vec), 0.0f);
            float specularDot = max(dot(reflect(spotLightDirection, normal_vec), normalize(camera_position - fragment.render_position)), 0.0f);
            diffuseColor = phongData.g * diffuseDot * light_color * light_intensity * distanceFactor;
            specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity * distanceFactor;
        }

    } else if (light_type == LIGHT_TYPE_POINT){
        // position, range, color, intensity
        // light intensity should diminish proportionally to `1 / distance_squared`
        vec3 pointLightDirection = normalize(fragment.render_position - light_position);
        float pointLightDistance = length(fragment.render_position - light_position);
        if (pointLightDistance <= light_range){
            float distanceFactor = min(0.01f / pow(pointLightDistance / light_range, 2), 4); // 4 is the distanceFactor at 'distance = 0.05 * range'
            float diffuseDot = max(dot(-pointLightDirection, normal_vec), 0.0f);
            float specularDot = max(dot(reflect(pointLightDirection, normal_vec), normalize(camera_position - fragment.render_position)), 0.0f);
            diffuseColor = phongData.g * diffuseDot * light_color * light_intensity * distanceFactor;
            specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity * distanceFactor;
        }
    }

    out_color = texColor * vec4(ambientColor + diffuseColor + specularColor, 1.0f);
}