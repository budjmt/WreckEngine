#version 450

//layout(triangles, equal_spacing, ccw) in;
layout(triangles, fractional_odd_spacing, ccw) in;

in vec3 tcPosition[];
patch in vec3 teCamPos;

layout (location = 49) uniform int numSamples = 5;
layout (location = 50) uniform vec2 K_rm;
layout (location = 51) uniform vec2 atmosRadius;

layout (location = 52) uniform vec3 sunPos;
layout (location = 53) uniform vec3 sunColor;

layout (location = 54) uniform mat4 cameraMatrix;
layout (binding = 0) uniform sampler2D lookupTex;

const float heightScale = 1 / (atmosRadius.y - atmosRadius.x);

out vec3 fragNormal;
out vec3 sunDir;
out vec3 rayleighColor;
out vec3 mieColor;

vec2 lookup(in vec2 heightAngle) { return textureLod(lookupTex, heightAngle, 0).xy; }

float scaleHeight(in float h)  { return (h - atmosRadius.x) * heightScale; }
float atmosHeight(in vec3 loc) { return scaleHeight(length(loc)); }

float getLookupAngle(in vec3 dir, in vec3 point, float pointLen) {
    float angle = dot(dir, point) / pointLen;
    return 0.5 - 0.5 * angle;
}

vec2 getPtRadHeight(in vec3 point) {
    float rad = length(point);
    return vec2(rad, scaleHeight(rad));
}

float getInScatter(in vec3 samplePt, in vec2 radheight, in vec2 segOpticalDepth) {
    float sunAngle = getLookupAngle(sunDir, samplePt, radheight.x);
    vec2 sunLookup = lookup(vec2(radheight.y, sunAngle)); 
    return segOpticalDepth.x * exp(-(sunLookup.y + segOpticalDepth.y));
}

// writes to intersects if there are intersections
// assumes sphere centered at the origin
bool lineSphereIntersect(in vec3 pos, in vec3 ray, in float radius, out vec2 intersect) {  
    float pr = dot(pos, ray);
    float test = pr * pr + radius * radius - dot(pos, pos);
    if(test < 0) return false; // early exit if test is negative
    test = sqrt(test);
    intersect = -pr + vec2(-test, test);
    return true;
}

vec3 getVertPosition() {
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    
    fragNormal = normalize(p0 + p1 + p2);
    return fragNormal * atmosRadius.y; // normalizing makes the c-sphere
}

void main() {
    vec3 vertPos = getVertPosition();
    vec3 camPos = teCamPos;
    gl_Position = cameraMatrix * vec4(vertPos, 1);
    
    sunDir = normalize(sunPos - vertPos);
    
    float camHeight  = atmosHeight(camPos);
    float vertHeight = atmosHeight(vertPos);
    vec3 ray = normalize(vertPos - camPos);
  
    vec2 intersect;
    lineSphereIntersect(camPos, ray, atmosRadius.y, intersect);
    /* unnecessary for atmosphere, since there's always an intersection
    // exit early if there was no intersection or if both were behind the camera
    if(!lineSphereIntersect(camPos, ray, atmosRadius.y, intersect)
       || all(lessThan(intersect, vec2(0)))) {
        discard;
    }
    */
    
    vec3 end = (vertHeight > 1) ? camPos + intersect.y * ray : vertPos; // B

    float inScatterInt = 0;
    if(camHeight > 1) {
        vec3 start = camPos + intersect.x; // A
        float camAngle = getLookupAngle(ray, start, atmosRadius.y); // consistent across the ray
        
        float scaledLen = length(end - start) / float(numSamples);
        float currLen = scaledLen;
        
        vec3 samplePt = start, inc = ray * scaledLen;
        for(int i = 0; i < numSamples; ++i, currLen += scaledLen, samplePt += inc) { // numSamples is probably about 5
            vec2 radheight = getPtRadHeight(samplePt);
            vec2 camLookup = lookup(vec2(radheight.y, camAngle));
            
            inScatterInt += currLen * getInScatter(samplePt, radheight, camLookup);
        }
    }
    else {
        vec3 start = camPos; // A
        float outerSign = (vertHeight > camHeight) ? -1 : 1; // consistent when clamped to atmosphere
        float camAngle = getLookupAngle(ray * outerSign, start, atmosRadius.y); // consistent across the ray
        
        float scaledLen = length(end - start) / float(numSamples);
        float currLen = scaledLen;
        
        vec3 samplePt = start, inc = ray * scaledLen;
        for(int i = 0; i < numSamples; ++i, currLen += scaledLen) { // numSamples is probably about 5
            vec2 radheight = getPtRadHeight(samplePt);
            vec2 sampleLookup = lookup(vec2(radheight.y, camAngle));
            vec2 camLookup = lookup(vec2(camHeight, camAngle));
            
            // it doesn't matter whether the sun or sample lookup's x is used
            vec2 segOpticalDepth = vec2(sampleLookup.x, outerSign * (sampleLookup.y - camLookup.y));
            inScatterInt += currLen * getInScatter(samplePt, radheight, segOpticalDepth);
        }
    }

    const float brightness = 1;
    inScatterInt *= brightness;
    vec2 inScatter = inScatterInt * K_rm; // K contains the constants for Rayleigh and Mie scattering
    
    vec3 invWavelength = 1 / pow(sunColor, vec3(4));
    rayleighColor = inScatter.x * invWavelength;
    mieColor      = inScatter.y * invWavelength;
}
