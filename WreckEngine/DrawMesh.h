#pragma once

#include "Renderable.h"
#include "Mesh.h"

class DrawMesh : public Renderable {
public:
    DrawMesh(Render::MaterialPass* r, shared<Mesh> m, const char* texFile, GLprogram shader);
    DrawMesh(Render::MaterialPass* r, shared<Mesh> m, GLtexture tex, GLprogram shader);
    DrawMesh(Render::MaterialPass* r, Mesh::FaceData& fd, Mesh::FaceIndex& fi, const char* texFile, GLprogram shader);
    DrawMesh(Render::MaterialPass* r, Mesh::FaceData& fd, Mesh::FaceIndex& fi, GLtexture tex, GLprogram shader);
        
    void setup(GLtexture tex, GLprogram shader);
    void draw(const mat4& world) override;

    GLenum tesselPrim = GL_TRIANGLES;
    size_t renderGroup = 0;
private:
    GLbuffer vertBuffer, elBuffer;
    ACCS_GS_T_C (private, shared<Mesh>, Mesh*, Mesh*, mesh, { return _mesh.get(); }, { _mesh.reset(value); });
};

