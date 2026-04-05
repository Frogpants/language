#version 120

uniform vec3 cam;
uniform vec3 rot;
uniform float fov;
uniform float width;
uniform float height;

void main() {
    vec3 p = gl_Vertex.xyz - cam;

    float yaw = radians(rot.x);
    float pitch = radians(rot.y);

    vec3 p1;
    p1.x = p.x * cos(yaw) - p.z * sin(yaw);
    p1.y = p.y;
    p1.z = p.x * sin(yaw) + p.z * cos(yaw);

    vec3 p2;
    p2.x = p1.x;
    p2.y = p1.y * cos(pitch) - p1.z * sin(pitch);
    p2.z = p1.y * sin(pitch) + p1.z * cos(pitch);

    float nearZ = 0.01;
    if (p2.z <= nearZ) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        gl_FrontColor = gl_Color;
        return;
    }

    float f = (width * 0.5) / tan(radians(fov) * 0.5);
    float cx = f * (p2.x / p2.z);
    float cy = f * (p2.y / p2.z);

    float ndcX = cx / (width * 0.5);
    float ndcY = cy / (height * 0.5);

    gl_Position = vec4(ndcX, ndcY, 0.0, 1.0);
    gl_FrontColor = gl_Color;
}
