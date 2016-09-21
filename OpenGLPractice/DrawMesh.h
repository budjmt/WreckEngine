#pragma once

#include "Drawable.h"
#include "Mesh.h"

class DrawMesh : public Drawable
{
public:
	DrawMesh(shared<Mesh> m, const char* texFile, GLprogram shader);
	DrawMesh(std::vector<vec3> v, std::vector<vec3> n, std::vector<vec3> u, Face f, const char* texFile, GLprogram shader);
	
	void setup(const char* texFile, GLprogram shader);
	void draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale);
	void draw(vec3 pos, vec3 scale, vec3 rotAxis, float rot);
private:
	GLbuffer vertBuffer, elBuffer;
	GLuniform<GLsampler> textureLoc;
	std::vector<GLuint> textures;
	ACCS_GS_T_C (private, shared<Mesh>, Mesh*, Mesh*, mesh, { return _mesh.get(); }, { _mesh.reset(value); });
};

