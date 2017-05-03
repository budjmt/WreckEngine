#version 450
#define ID gl_InvocationID

layout(vertices = 3) out;

layout(location = 1) uniform vec3 camPos;
layout(location = 2) uniform float Radius;

in vec3 vPosition[];
in vec2 vUV[];
out vec3 tcPosition[];
out float tcDist[];

patch out vec2 tcUV;
patch out int id;
patch out float radius;
patch out vec3 teCamPos;

const float MaxDist = 10.0;
const vec2 tessLevel = vec2(16, 16);

void main()
{
    tcPosition[ID] = normalize(vPosition[ID]);
    tcUV = vUV[ID];

    radius = Radius;
    teCamPos = camPos;

    vec3 p0 = tcPosition[0] * radius;
    vec3 p1 = tcPosition[1] * radius;
    vec3 p2 = tcPosition[2] * radius;
    vec3 c = (p0 + p1 + p2) / 3;

    float distC = distance(c,  camPos);
    float dist0 = distance(p0, camPos);
    float dist1 = distance(p1, camPos);
    float dist2 = distance(p2, camPos);

    vec4 dist = vec4(vec3(dist1 + dist2, dist2 + dist0, dist0 + dist1) * 0.5, distC);
    vec4 distSc = 1.0 - min(dist / MaxDist, 1.0);

    vec4 levels = mix(vec4(1.0), vec4(vec3(tessLevel.y), tessLevel.x), distSc);

    gl_TessLevelOuter[0] = levels.x;
    gl_TessLevelOuter[1] = levels.y;
    gl_TessLevelOuter[2] = levels.z;
    gl_TessLevelInner[0] = levels.w;
}
