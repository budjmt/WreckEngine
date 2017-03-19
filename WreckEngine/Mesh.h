#pragma once

#include "GL/glew.h"

#include <vector>

#include "MarchMath.h"

#include "Drawable.h"

class Mesh {
public:
    struct RenderData {
        std::vector<GLfloat> vbuffer;
        std::vector<GLuint>  ebuffer;
    };

    template<typename T> struct Face { std::vector<T> verts, uvs, normals; };
    typedef Face<vec3> FaceData;
    struct FaceIndex : Face<GLuint> {
        std::vector<vec3> combinations;//all the unique v/u/n index combinations
    };

    Mesh(FaceData fd, FaceIndex fi);

    // return the value of half dims
    vec3 getGrossDims();
    vec3 getPreciseDims();
    vec3 getCentroid();

    // return the full dims
    static float getGrossDim(const std::vector<vec3>& verts);
    static vec3 getPreciseDims(const std::vector<vec3>& verts);
    static vec3 getCentroid(const std::vector<vec3>& verts);

    void translate(const vec3 t);
    void translateTo(const vec3 t);
    void scale(const vec3 s);
    void scaleTo(const vec3 s);
    void rotate(const quat q);

    shared<RenderData> getRenderData();

protected:
    ACCS_GS_T(protected, FaceData, const FaceData&, const FaceData&, data);
    ACCS_GS_T(protected, FaceIndex, const FaceIndex&, const FaceIndex&, indices);

    shared<RenderData> renderData = shared<RenderData>(nullptr);

    vec3 h_dims = vec3(-1);
    friend class DrawMesh;
};