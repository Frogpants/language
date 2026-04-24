#version 120

uniform vec3 cam;
uniform vec3 rot;
uniform float fov;
uniform float width;
uniform float height;

vec3 rotateYawPitch(vec3 v, float yawDeg, float pitchDeg) {
    float yaw = radians(yawDeg);
    float pitch = radians(pitchDeg);

    vec3 v1;
    v1.x = v.x * cos(yaw) - v.z * sin(yaw);
    v1.y = v.y;
    v1.z = v.x * sin(yaw) + v.z * cos(yaw);

    vec3 v2;
    v2.x = v1.x;
    v2.y = v1.y * cos(pitch) - v1.z * sin(pitch);
    v2.z = v1.y * sin(pitch) + v1.z * cos(pitch);
    return v2;
}

void main() {
    vec3 rel = gl_Vertex.xyz - cam;

    // Build camera basis from the same yaw/pitch convention used in frag.glsl.
    vec3 forward = normalize(rotateYawPitch(vec3(0.0, 0.0, 1.0), rot.x, rot.y));
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), forward));
    vec3 up = normalize(cross(forward, right));

    vec3 p2;
    p2.x = dot(rel, right);
    p2.y = dot(rel, up);
    p2.z = dot(rel, forward);

    float nearZ = 0.01;
    float farZ = 5000.0;
    float fovRad = radians(fov);
    float tanHalf = max(tan(fovRad * 0.5), 0.00001);
    float aspect = max(width / max(height, 1.0), 0.00001);

    // Perspective projection in clip space (camera forward is +Z).
    float clipX = p2.x / (tanHalf * aspect);
    float clipY = p2.y / tanHalf;
    float clipW = p2.z;

    float a = (farZ + nearZ) / (farZ - nearZ);
    float b = (-2.0 * farZ * nearZ) / (farZ - nearZ);
    float clipZ = a * p2.z + b;

    gl_Position = vec4(clipX, clipY, clipZ, clipW);
    gl_FrontColor = gl_Color;
}
