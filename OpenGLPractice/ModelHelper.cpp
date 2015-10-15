#include "ModelHelper.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

Mesh* loadOBJ(const char* file, char* texture, GLuint shader) {
	cout << "Loading " << file << endl;
	ifstream infile;
	infile.open(file, ios::in);
	if (!infile.is_open()) {
		cout << "Error! File " << file << " could not be read." << endl;
		return 0;
	}
	else
		cout << "File Loaded" << endl;

	vector<GLfloat> verts,normals,uvs;
	Face faces = Face();

	std::string line;
	while (getline(infile, line)) {
		
		std::vector<std::string> tokens = tokenize(line, " ");
		
		//vertices
		if(line.find("v ", 0) == 0) {
			verts.push_back(stof(tokens[1]));
			verts.push_back(stof(tokens[2]));
			verts.push_back(stof(tokens[3]));
			//verts.push_back(1); verts.push_back(1); verts.push_back(1);
		}

		//normals
		else if (line.find("vn", 0) == 0) {
			normals.push_back(stof(tokens[1]));
			normals.push_back(stof(tokens[2]));
			normals.push_back(stof(tokens[3]));
		}
		//uvs
		else if (line.find("vt", 0) == 0) {
			uvs.push_back(stof(tokens[1]));
			uvs.push_back(stof(tokens[2]));
			//uvs.push_back(stof(tokens[3]));//unnecessary
		}
		else if (line[0] == 'f') {
			for (unsigned int i = 1; i < tokens.size(); i++) {
				std::vector<std::string> faceTokens = tokenize(tokens[i], "/");
				GLuint v = (unsigned int)stoi(faceTokens[0]) - 1
					, u = (unsigned int)stoi(faceTokens[1]) - 1
					, n = (unsigned int)stoi(faceTokens[2]) - 1;
				faces.verts.push_back(v);
				faces.uvs.push_back(u);
				faces.normals.push_back(n);
			}
		}
	}
	
	/*for (unsigned int i = 0;i < faces.verts->size();i++) {
		cout << faces.vertInfo[0][i] << ",";
		if((i + 1) % 3 == 0)
			cout << endl;
	}*/

	Mesh* mesh = new Mesh(verts, normals, uvs, faces, texture, shader);
	return mesh;
}

void genOBJ(const char* file, std::vector<GLfloat> verts, std::vector<GLfloat> uvs, std::vector<GLfloat> norms, std::vector<GLuint> faces) {

}

void genCube(const char* file, float w, float h, float d) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces, faces;
	
	//generate verts
	verts = {
		w / 2, h / 2, d / 2,
		w / 2, h / 2, -d / 2,
		w / 2, -h / 2, d / 2,
		w / 2, -h / 2, -d / 2,
		-w / 2, h / 2, d / 2,
		-w / 2, h / 2, -d / 2,
		-w / 2, -h / 2, d / 2,
		-w / 2, -h / 2, d / 2,
	};

	//generate faces and normals
	vertFaces = {
		3, 1, 2,		2, 4, 3,
		4, 2, 6,		6, 8, 4,
		8, 6, 5,		5, 7, 8,
		5, 1, 3,		3, 7, 5,
		7, 3, 4,		4, 8, 7,
		6, 2, 1,		1, 5, 6
	};//that was tedious

	genNormals(verts, vertFaces, norms, normFaces);

	//generate uvs

	genOBJ(file, verts, uvs, norms, faces);
}

void genCylinder(const char* file, float w, float h, float d, int res) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> faces;
	//code
	genOBJ(file, verts, uvs, norms, faces);
}

void genSphere(const char* file, float r, int res) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> faces;
	//code
	genOBJ(file, verts, uvs, norms, faces);
}

void genBezierSurface(const char* file, float w, float h, float d) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> faces;
	//code
	genOBJ(file, verts, uvs, norms, faces);
}

std::vector<GLfloat> genUVs(std::vector<GLfloat>& verts) {
	return genCylindrical(verts);
}

std::vector<GLfloat> genCylindrical(std::vector<GLfloat>& verts) {
	std::vector<GLfloat> uvs;
	for (int i = 0; i < verts.size(); i += FLOATS_PER_VERT) {
		GLfloat u = atan2f(verts[i + 1], verts[i]);//azimuth, atan2 does a lot of the work for you
		GLfloat v = verts[i + 2];//just z
		uvs.push_back(u);
		uvs.push_back(v);
		uvs.push_back(0);
	}
	return uvs;
}

std::vector<GLfloat> genSpherical(std::vector<GLfloat>& verts) {
	std::vector<GLfloat> uvs;
	for (int i = 0; i < verts.size(); i += FLOATS_PER_VERT) {
		GLfloat u = (float)acos(verts[i + 2] / sqrt(pow(verts[i],2) + pow(verts[i + 1],2) + pow(verts[i + 2],2)));//theta
		GLfloat v = atan2f(verts[i + 1], verts[i]);//azimuth
		uvs.push_back(u);
		uvs.push_back(v);
		uvs.push_back(0);
	}
	return uvs;
}

std::vector<GLfloat> genNormals(std::vector<GLfloat>& verts, std::vector<GLfloat>& vertFaces
								, std::vector<GLfloat>& norms, std::vector<GLfloat>& normFaces) {
	for (int i = 0; i < vertFaces.size(); i += FLOATS_PER_VERT * 2) {
		glm::vec3 v1 = glm::vec3(verts[vertFaces[i] - 1], verts[vertFaces[i] - 1 + 1], verts[vertFaces[i] - 1 + 2]);
		glm::vec3 v2 = glm::vec3(verts[vertFaces[i + 1] - 1], verts[vertFaces[i + 1] - 1 + 1], verts[vertFaces[i + 1] - 1 + 2]);
		glm::vec3 v3 = glm::vec3(verts[vertFaces[i + 2] - 1], verts[vertFaces[i + 2] - 1 + 1], verts[vertFaces[i + 2] - 1 + 2]);
		GLfloat* n = genNormal(v1, v2, v3);
		//add check to make sure no repeats
		norms.push_back(n[0]);
		norms.push_back(n[1]);
		norms.push_back(n[2]);
		normFaces.push_back(i / 2 + 1);//not this simple apparently
		normFaces.push_back(i / 2 + 1 + 1);
		normFaces.push_back(i / 2 + 2 + 1);
	}
}

std::vector<std::string> tokenize(std::string str, std::string delimiter) {
	std::vector<std::string> tokens;
	int curr, prev;
	for (curr = str.find(delimiter, 0), prev = 0; curr != string::npos; prev = curr + 1, curr = str.find(delimiter, prev)) {
		//cout << curr << "," << prev << endl;
		if (curr - prev > 0) {
			tokens.push_back(str.substr(prev, curr - prev));
			//cout << str.substr(prev, curr - prev) << endl;
		}
	}
	curr = str.length();
	if(curr - prev > 0)
		tokens.push_back(str.substr(prev, curr - prev));
	//cout << prev << "," << curr << endl;
	//cout << str.substr(prev, curr - prev) << endl;
	return tokens;
}