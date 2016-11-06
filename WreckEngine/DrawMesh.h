#pragma once

#include "Drawable.h"
#include "Mesh.h"

class DrawMesh : public Drawable
{
public:
    DrawMesh(shared<Mesh> m, const char* texFile, GLprogram shader);
    DrawMesh(Mesh::FaceData& fd, Mesh::FaceIndex& fi, const char* texFile, GLprogram shader);
        
    void setup(const char* texFile, GLprogram shader);
    void draw(const mat4& world);

    static shared<DrawMesh> FromFile(shared<Mesh> m, const char* textureFile, GLprogram shader);
private:
    GLbuffer vertBuffer, elBuffer;
    GLuniform<GLsampler> textureLoc;
    std::vector<GLtexture> textures;
    ACCS_GS_T_C (private, shared<Mesh>, Mesh*, Mesh*, mesh, { return _mesh.get(); }, { _mesh.reset(value); });
};

