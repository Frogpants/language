#version 120

uniform vec3 cam;
uniform vec3 rot;
uniform float fov;
uniform float width;
uniform float height;
uniform float fogIntensity;
uniform sampler2D sceneTex;
uniform bool fullscreenPass;

struct Hit {
    float t;
    vec3 pos;
    vec3 normal;
    vec3 albedo;
    float roughness;
    bool hit;
};

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

bool intersectSphere(vec3 ro, vec3 rd, vec3 center, float radius, out float tHit) {
    vec3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;
    if (h < 0.0) {
        return false;
    }
    h = sqrt(h);
    float t0 = -b - h;
    float t1 = -b + h;
    float eps = 0.001;
    tHit = (t0 > eps) ? t0 : t1;
    return tHit > eps;
}

bool intersectPlane(vec3 ro, vec3 rd, float yPlane, out float tHit) {
    float denom = rd.y;
    if (abs(denom) < 0.00001) {
        return false;
    }
    tHit = (yPlane - ro.y) / denom;
    return tHit > 0.001;
}

Hit traceScene(vec3 ro, vec3 rd) {
    Hit outHit;
    outHit.hit = false;
    outHit.t = 1e20;
    outHit.pos = vec3(0.0);
    outHit.normal = vec3(0.0, 1.0, 0.0);
    outHit.albedo = vec3(0.0);
    outHit.roughness = 1.0;

    float t;

    // Ground plane
    if (intersectPlane(ro, rd, -100.0, t) && t < outHit.t) {
        vec3 p = ro + rd * t;
        float checker = mod(floor(p.x * 0.05) + floor(p.z * 0.05), 2.0);
        vec3 c0 = vec3(0.18, 0.20, 0.22);
        vec3 c1 = vec3(0.10, 0.11, 0.12);

        outHit.hit = true;
        outHit.t = t;
        outHit.pos = p;
        outHit.normal = vec3(0.0, 1.0, 0.0);
        outHit.albedo = mix(c0, c1, checker);
        outHit.roughness = 0.95;
    }

    // Simple analytic scene spheres
    vec3 s0 = vec3(0.0, -40.0, 220.0);
    vec3 s1 = vec3(-90.0, -65.0, 260.0);
    vec3 s2 = vec3(95.0, -75.0, 300.0);

    if (intersectSphere(ro, rd, s0, 60.0, t) && t < outHit.t) {
        vec3 p = ro + rd * t;
        outHit.hit = true;
        outHit.t = t;
        outHit.pos = p;
        outHit.normal = normalize(p - s0);
        outHit.albedo = vec3(0.95, 0.22, 0.18);
        outHit.roughness = 0.35;
    }
    if (intersectSphere(ro, rd, s1, 35.0, t) && t < outHit.t) {
        vec3 p = ro + rd * t;
        outHit.hit = true;
        outHit.t = t;
        outHit.pos = p;
        outHit.normal = normalize(p - s1);
        outHit.albedo = vec3(0.20, 0.85, 0.45);
        outHit.roughness = 0.55;
    }
    if (intersectSphere(ro, rd, s2, 25.0, t) && t < outHit.t) {
        vec3 p = ro + rd * t;
        outHit.hit = true;
        outHit.t = t;
        outHit.pos = p;
        outHit.normal = normalize(p - s2);
        outHit.albedo = vec3(0.20, 0.45, 0.95);
        outHit.roughness = 0.18;
    }

    return outHit;
}

float softShadow(vec3 ro, vec3 rd, float tMax) {
    Hit h = traceScene(ro, rd);
    if (h.hit && h.t < tMax) {
        return 0.0;
    }
    return 1.0;
}

void main() {
    if (!fullscreenPass) {
        // First pass: render triangles from main.ge into scene texture.
        gl_FragColor = vec4(gl_Color.rgb, 1.0);
        return;
    }

    vec2 screen = vec2(max(width, 1.0), max(height, 1.0));
    vec2 uv = (gl_FragCoord.xy / screen) * 2.0 - 1.0;
    uv.x *= screen.x / screen.y;

    float tanHalf = tan(radians(fov) * 0.5);

    // Match vert.glsl camera basis exactly so mouse look feels identical.
    vec3 forward = normalize(rotateYawPitch(vec3(0.0, 0.0, 1.0), rot.x, rot.y));
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), forward));
    vec3 up = normalize(cross(forward, right));
    vec3 rd = normalize(forward + right * (uv.x * tanHalf) + up * (uv.y * tanHalf));
    vec3 ro = cam;

    vec3 skyTop = vec3(0.55, 0.72, 0.92);
    vec3 skyHorizon = vec3(0.90, 0.95, 1.00);
    vec3 color = mix(skyHorizon, skyTop, clamp(rd.y * 0.5 + 0.5, 0.0, 1.0));

    Hit hit = traceScene(ro, rd);
    if (hit.hit) {
        vec3 lightPos = cam + vec3(120.0, 220.0, -80.0);
        vec3 L = lightPos - hit.pos;
        float distL = length(L);
        L /= max(distL, 0.0001);

        vec3 V = normalize(cam - hit.pos);
        vec3 H = normalize(L + V);

        float nDotL = max(dot(hit.normal, L), 0.0);
        float nDotH = max(dot(hit.normal, H), 0.0);

        float shadow = softShadow(hit.pos + hit.normal * 0.05, L, distL);
        float attenuation = 1.0 / (1.0 + distL * 0.01 + distL * distL * 0.00002);

        vec3 ambient = hit.albedo * 0.12;
        vec3 diffuse = hit.albedo * nDotL * shadow;

        float specPower = mix(128.0, 8.0, clamp(hit.roughness, 0.0, 1.0));
        float spec = pow(nDotH, specPower) * shadow;
        vec3 specCol = mix(vec3(1.0), hit.albedo, hit.roughness * 0.25) * spec;

        color = ambient + (diffuse + specCol) * attenuation * 3.5;

        // Subtle fog for depth cue.
        float fog = 1.0 - exp(-hit.t * max(fogIntensity, 0.0));
        color = mix(color, skyHorizon, clamp(fog, 0.0, 1.0));
    }

    vec2 texUv = gl_FragCoord.xy / screen;
    vec4 sceneSample = texture2D(sceneTex, texUv);
    color = mix(color, sceneSample.rgb, clamp(sceneSample.a, 0.0, 1.0));

    // Tone-map + gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    gl_FragColor = vec4(color, 1.0);
}
