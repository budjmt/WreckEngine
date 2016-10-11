#pragma once

#include "Drawable.h"
#include "Mesh.h"

class DrawMesh : public Drawable
{
public:
	DrawMesh(shared<Mesh> m, const char* texFile, GLprogram shader);
	DrawMesh(std::vector<vec3> v, std::vector<vec3> n, std::vector<vec3> u, Mesh::Face f, const char* texFile, GLprogram shader);
	
	void setup(const char* texFile, GLprogram shader);
	void draw(const mat4& world);
private:
	GLbuffer vertBuffer, elBuffer;
	GLuniform<GLsampler> textureLoc;
	std::vector<GLtexture> textures;
	ACCS_GS_T_C (private, shared<Mesh>, Mesh*, Mesh*, mesh, { return _mesh.get(); }, { _mesh.reset(value); });
};

