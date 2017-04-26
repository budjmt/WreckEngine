#version 450

layout (location = 0) out vec4 color;

in vec3 fragNormal;
in vec3 sunDir;
in vec3 rayleighColor;
in vec3 mieColor;

// rayleigh scattering, g = 0
float phaseApprox(in float c2) {
    return 0.75 * (1 + c2);
}

// mie scattering, g usually (-1, -0.75]
// DO NOT set g = 1 or -1, reduces to 0
float phaseExact(in float c, in float c2, in float g) {
    float g2 = g * g;
    return 1.5 * (1 - g2) * (1 + c2) / ((2 + g2) * pow(1 + g2 - 2 * g * c, 1.5));
}

void main() {
    float c = dot(sunDir, fragNormal), c2 = c * c;
    const float g = -0.8;
    
    vec3 scatterColor = phaseApprox(c2) * rayleighColor + phaseExact(c, c2, g);
    color = vec4(scatterColor, scatterColor.b);
    //color = vec4(scatterColor, 1);
}