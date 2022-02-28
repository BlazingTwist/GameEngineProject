#version 450

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_SPOT 1
#define LIGHT_TYPE_POINT 2

in fragmentData{
    vec2 uv_coord;
    vec3 world_position;
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
    vec3 vec_to_eye_normalized = normalize(camera_position - fragment.world_position);

    // quality setting for heightMap // should we turn this into a uniform rather than hard-coding?
    float height_map_scale = 0.05f;
    float minLayers = 8.0f;
    float maxLayers = 64.0f;
    // test more layers for shallower view angles
    float normal_dot_vecToEye = dot(fragment.tbn_matrix[2], vec_to_eye_normalized);
    float numLayers = mix(maxLayers, minLayers, abs(normal_dot_vecToEye));
    float layerHeightStep = 1.0f / numLayers;
    vec3 tangent_view_vec = normalize(transpose(fragment.tbn_matrix) * -vec_to_eye_normalized);

    // calculate shifted UV coordinates using parallax occlusion mapping
    vec2 tangent_step_uv = tangent_view_vec.xy * height_map_scale / numLayers;
    float currentLayerHeight = 1.0f;
    vec2 currentUV = fragment.uv_coord;
    float currentHeightMapValue = texture(tx_height, currentUV).r;
    float previousHeightMapValue = currentHeightMapValue;

    // loop until tested layer is beneath (approximated) heightMap curve
    while (currentLayerHeight > currentHeightMapValue){
        currentUV += tangent_step_uv;
        previousHeightMapValue = currentHeightMapValue;
        currentHeightMapValue = texture(tx_height, currentUV).r;
        currentLayerHeight -= layerHeightStep;
    }

    float heightMapSampleWeight = (currentLayerHeight - currentHeightMapValue) / (previousHeightMapValue - currentHeightMapValue - layerHeightStep);
    currentUV = currentUV - (heightMapSampleWeight * tangent_step_uv);

    // discard UVs that are outside of the uv map
    if (currentUV.x > 1.0f || currentUV.x < 0.0f || currentUV.y > 1.0f || currentUV.y < 0.0f){
        discard;
    }

    // apply shifted UV coordinates to textures
    vec4 texColor = texture(tx_color, currentUV);
    vec4 phongData = texture(tx_phong, currentUV);
    vec3 normal_vec = normalize(fragment.tbn_matrix * (texture(tx_normal, currentUV).xyz * 2.0f - 1.0f));
    vec3 ambientColor = max(phongData.r * ambient_light, 0.0f);
    vec3 diffuseColor = vec3(0.0f, 0.0f, 0.0f);
    vec3 specularColor = vec3(0.0f, 0.0f, 0.0f);

    // TODO light bounces? occlusion?
    // TODO rewrite x/distance stuff to use (1+(x/distance)) instead of clamping (?).
    if (light_type == LIGHT_TYPE_DIRECTIONAL){
        // direction, color, intensity

        float diffuseDot = max(dot(-light_direction, normal_vec), 0.0f);
        float specularDot = max(dot(reflect(light_direction, normal_vec), vec_to_eye_normalized), 0.0f);
        diffuseColor = phongData.g * diffuseDot * light_color * light_intensity;
        specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity;

    } else if (light_type == LIGHT_TYPE_SPOT){
        // position, direction, range, spot_angle, color, intensity
        vec3 spotLightDirection = normalize(fragment.world_position - light_position);
        // here we're (ab-)using the fact that the dot product of two normalized vectors is the cosine of their angle
        float spotLightAngle = dot(light_direction, spotLightDirection);
        float spotLightDistance = length(fragment.world_position - light_position);
        if (spotLightAngle > light_spot_angle && spotLightDistance <= light_range){
            float distanceFactor = min(0.01f / pow(spotLightDistance / light_range, 2), 4);// 4 is the distanceFactor at 'distance = 0.05 * range'
            float diffuseDot = max(dot(-spotLightDirection, normal_vec), 0.0f);
            float specularDot = max(dot(reflect(spotLightDirection, normal_vec), vec_to_eye_normalized), 0.0f);
            diffuseColor = phongData.g * diffuseDot * light_color * light_intensity * distanceFactor;
            specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity * distanceFactor;
        }

    } else if (light_type == LIGHT_TYPE_POINT){
        // position, range, color, intensity
        // light intensity should diminish proportionally to `1 / distance_squared`
        vec3 pointLightDirection = normalize(fragment.world_position - light_position);
        float pointLightDistance = length(fragment.world_position - light_position);
        if (pointLightDistance <= light_range){
            float distanceFactor = min(0.01f / pow(pointLightDistance / light_range, 2), 4);// 4 is the distanceFactor at 'distance = 0.05 * range'
            float diffuseDot = max(dot(-pointLightDirection, normal_vec), 0.0f);
            float specularDot = max(dot(reflect(pointLightDirection, normal_vec), vec_to_eye_normalized), 0.0f);
            diffuseColor = phongData.g * diffuseDot * light_color * light_intensity * distanceFactor;
            specularColor = phongData.b * 8 * pow(specularDot, phongData.a * 255) * light_color * light_intensity * distanceFactor;
        }
    }

    out_color = texColor * vec4(ambientColor + diffuseColor + specularColor, 1.0f);
}