#pragma once

#include "Renderable.h"
#include "Mesh.h"

class DrawMesh : public Renderable {
public:
    DrawMesh(Render::MaterialPass* r, shared<Mesh> m, const char* texFile, GLprogram shader, bool hasTangent = false);
    DrawMesh(Render::MaterialPass* r, shared<Mesh> m, GLtexture tex, GLprogram shader, bool hasTangent = false);
    DrawMesh(Render::MaterialPass* r, Mesh::FaceData& fd, Mesh::FaceIndex& fi, const char* texFile, GLprogram shader, bool hasTangent = false);
    DrawMesh(Render::MaterialPass* r, Mesh::FaceData& fd, Mesh::FaceIndex& fi, GLtexture tex, GLprogram shader, bool hasTangent = false);
        
    void setup(GLtexture tex, GLprogram shader, bool hasTangent);
    void draw(const mat4& world, Entity* entity) override;

    GLenum tesselPrim = GL_TRIANGLES;
    size_t renderGroup = 0;
private:
    GLbuffer vertBuffer, elBuffer;
    ACCS_GS_T_C (private, shared<Mesh>, weak<Mesh>, shared<Mesh>, mesh, { return _mesh; }, { _mesh = value; });
};

