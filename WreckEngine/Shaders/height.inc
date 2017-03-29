#ifndef HEIGHT_INC
#define HEIGHT_INC

vec3 getRawHeight(in vec3 dirFromCenter, in float height) {
    return dirFromCenter * height;
}

vec3 getHeight(in vec3 dirFromCenter, in float height, in float camDist) {
    //return dirFromCenter * (height / camDist);
    return getRawHeight(dirFromCenter, height / camDist);
}

#endif
