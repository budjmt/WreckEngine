#include "ModelHelper.h"

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <fstream>

using namespace std;

ostream& operator<<(ostream& os, glm::vec3 v) {
	os << "(" << v.x << "," << v.y << "," << v.z << ")";
	return os;
}

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
	cout << "Complete!" << endl;
	Mesh* mesh = new Mesh(verts, normals, uvs, faces, texture, shader);
	return mesh;
}

void genOBJ(const char* file, std::vector<GLfloat>& verts, std::vector<GLfloat>& uvs, std::vector<GLfloat>& norms
							, std::vector<GLuint>& vertFaces, std::vector<GLuint>& uvFaces, std::vector<GLuint>& normFaces) {

	cout << "Generating " << file << endl;

	cout << "Num Verts: " << verts.size() << endl;
	cout << "Num Vert Faces: " << vertFaces.size() << endl;
	cout << "Num UVs: " << uvs.size() << endl;
	cout << "Num UV Faces: " << uvFaces.size() << endl;
	cout << "Num Normals: " << norms.size() << endl;
	cout << "Num Normal Faces: " << normFaces.size() << endl;
	string fileContents = "";
	for (unsigned int i = 0; i < verts.size(); i += FLOATS_PER_VERT) {
		fileContents += "v";
		for (int j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(verts[i + j]);
		}
		fileContents += "\n";
	}
	for (unsigned int i = 0; i < uvs.size(); i += FLOATS_PER_VERT) {
		fileContents += "vt";
		for (int j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(uvs[i + j]);//it's only floats per vert because otherwise I'd need special case for UV
		}
		fileContents += "\n";
	}
	for (unsigned int i = 0; i < norms.size(); i += FLOATS_PER_VERT) {
		fileContents += "vn";
		for (int j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(norms[i + j]);
		}
		fileContents += "\n";
	}
	for (unsigned int i = 0; i < vertFaces.size(); i += FLOATS_PER_VERT) {
		fileContents += "f";
		for (int j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(vertFaces[i + j]);
			fileContents += "/";
			fileContents += to_string(uvFaces[i + j]);
			fileContents += "/";
			fileContents += to_string(normFaces[i + j]);
		}
		fileContents += "\n";
	}

	cout << "Writing..." << endl;
	//write fileContents to the file, overwrite its contents
	std::ofstream ofile;
	ofile.open(file, ios::out | ios::trunc);
	ofile.write(fileContents.c_str(), fileContents.length());
	ofile.close();
	cout << "File successfully generated." << endl;
}

void genCube(const char* file) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;
	
	//cout << "Generating verts" << endl;
	//generate verts
	verts = {
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
	};
	//cout << "Generating vert faces" << endl;
	//generate faces and normals
	vertFaces = {
		3, 1, 2,		2, 4, 3,
		4, 2, 6,		6, 8, 4,
		8, 6, 5,		5, 7, 8,
		5, 1, 3,		3, 7, 5,
		7, 3, 4,		4, 8, 7,
		6, 2, 1,		1, 5, 6
	};//that was tedious

	//cout << "Generating normals" << endl;
	genNormals(verts,vertFaces,norms,normFaces);

	//cout << "Generating uvs" << endl;
	//generate uvs
	genUVs(verts, vertFaces, uvs, uvFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genCylinder(const char* file, int res) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;
	
	//generate verts and vert faces
	glm::vec3 top = glm::vec3(0, 0.5, 0);
	glm::vec3 bottom = glm::vec3(0, -0.5, 0);
	verts.push_back(top.x); verts.push_back(top.y); verts.push_back(top.z);//0 = top middle
	verts.push_back(bottom.x); verts.push_back(bottom.y); verts.push_back(bottom.z);//1 = bottom middle
	
	//these are the tops and bottoms for each pair of verts, 0 is the very first (and would be last) ones added
	glm::vec3 t0, b0, tprev, tcurr, bprev, bcurr;
	
	t0 = glm::vec3(1, 0.5f, 0);
	b0 = glm::vec3(1, -0.5f, 0);
	verts.push_back(t0.x); verts.push_back(t0.y); verts.push_back(t0.z);
	verts.push_back(b0.x); verts.push_back(b0.y); verts.push_back(b0.z);

	tprev = t0;
	bcurr = b0;
	for (int i = 1; i < res - 1; i++) {
		float a = (float)(i * M_PI * 2 / res);
		tcurr = glm::vec3(cosf(a), 0.5f, sinf(a));
		bcurr = glm::vec3(cosf(a), -0.5f, sinf(a));
		
		verts.push_back(tcurr.x); verts.push_back(tcurr.y); verts.push_back(tcurr.z);
		verts.push_back(bcurr.x); verts.push_back(bcurr.y); verts.push_back(bcurr.z);

		/*side faces:
		2 * i --- 2 * i + 2
		|			/	  |
		|		  /		  |
		|		/		  |
		|	  /			  |
		2 * i + 1 --- 2 * i + 3
		*/
		//top face
		vertFaces.push_back(2 * i + 2); vertFaces.push_back(0); vertFaces.push_back(2 * i);
		//bottom face
		vertFaces.push_back(2 * i + 3); vertFaces.push_back(1); vertFaces.push_back(2 * i + 1);
		//side faces
		vertFaces.push_back(2 * i + 2); vertFaces.push_back(2 * i); vertFaces.push_back(2 * i + 1);
		vertFaces.push_back(2 * i + 1); vertFaces.push_back(2 * i + 3); vertFaces.push_back(2 * i + 2);

		tprev = tprev;
		bprev = bprev;
	}
	//final top face
	vertFaces.push_back(2); vertFaces.push_back(0); vertFaces.push_back(2 * (res - 1));
	//final bottom face
	vertFaces.push_back(3); vertFaces.push_back(1); vertFaces.push_back(2 * (res - 1) + 1);
	//final side faces
	vertFaces.push_back(2 * (res - 1) + 1); vertFaces.push_back(2 * (res - 1)); vertFaces.push_back(2);
	vertFaces.push_back(2); vertFaces.push_back(3); vertFaces.push_back(2 * (res - 1) + 1);

	//generate normals
	genNormals(verts, vertFaces, norms, normFaces);
	
	//generate uvs
	genUVCylindrical(verts, vertFaces, uvs, uvFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genSphere(const char* file, int res) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;

	//this adds the vertices in the first half circle, to prevent redundancy later
	//i is implied to be 0
	//once this is done, there are res vertices in verts, with the first and last ones not being repeated in subsequent ones, so res - 2 unique ones
	//top vert index is 0, bottom index is res
	for (int j = 0; j < res + 1; j++) {
		float x = cosf((float)(M_PI / res * j));
		float y = sinf((float)(M_PI / res * j));
		float z = 0;
		verts.push_back(x); verts.push_back(y); verts.push_back(z);
	}

	for (int i = 1; i < res; i++) {
		//current half circle
		float x = 1, y = 0, z = 0;
		//one notch down on the current half circle
		float x1 = cosf((float)(M_PI / res));
		float y1 = sinf((float)(M_PI / res)) * cosf((float)(2 * M_PI / res * i));
		float z1 = sinf((float)(M_PI / res)) * sinf((float)(2 * M_PI / res * i));
		
		int ind = 0;//1,0,0 is always the first vertex added to verts; ind begins at the top and moves down
		int ind1 = verts.size();//x1, y1, z1 is always new and always the last one added.
		int indp = 0;//prev half circle, begins at top and moves down
		int indp1 = verts.size() - res;//one notch down prev half circle
		
		verts.push_back(x1); verts.push_back(y1); verts.push_back(z1);
		vertFaces.push_back(ind); vertFaces.push_back(ind1); vertFaces.push_back(indp1);

		x = x1; y = y1; z = z1;
		indp = indp1; ind = ind1;

		for (int j = 1; j < res; j++) {
			int nextCircle = j + 1;
			//one notch down on the current half circle
			x1 = cosf((float)(M_PI / res * nextCircle));
			y1 = sinf((float)(M_PI / res * nextCircle)) * cosf((float)(2 * M_PI / res * i));
			z1 = sinf((float)(M_PI / res * nextCircle)) * sinf((float)(2 * M_PI / res * i));

			ind1 = verts.size();//x1, y1, z1 is always new and always the last one added
			indp1 = verts.size() - res;//one notch down on previous half circle

			//add only the next verts, as current ones will always be already added
			verts.push_back(x1); verts.push_back(y1); verts.push_back(z1);
			
			/*
			indp	---		ind
			|			/	  |
			|		  /		  |
			|		/		  |
			|	  /			  |
			indp1	---		ind1
			*/
			vertFaces.push_back(ind); vertFaces.push_back(indp); vertFaces.push_back(indp1);
			vertFaces.push_back(indp1); vertFaces.push_back(ind1); vertFaces.push_back(ind);
			//update current with next
			x = x1; y = y1; z = z1;
			indp = indp1;	ind = ind1;
		}
		ind1 = res;
		indp1 = res;
		vertFaces.push_back(ind); vertFaces.push_back(ind1); vertFaces.push_back(indp);
	}

	//generate normals
	genNormals(verts, vertFaces, norms, normFaces);

	//generate uvs
	genUVSpherical(verts, vertFaces, uvs, uvFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genBezierSurface(const char* file) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;
	//code
	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genNormals(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces
								, std::vector<GLfloat>& norms, std::vector<GLuint>& normFaces) {
	for (unsigned int i = 0; i < vertFaces.size(); i += 3) {
		int vert1 = (vertFaces[i] - 1) * FLOATS_PER_VERT;
		int vert2 = (vertFaces[i + 1] - 1) * FLOATS_PER_VERT;
		int vert3 = (vertFaces[i + 2] - 1) * FLOATS_PER_VERT;
		glm::vec3 v1 = glm::vec3(verts[vert1], verts[vert1 + 1], verts[vert1 + 2]);
		glm::vec3 v2 = glm::vec3(verts[vert2], verts[vert2 + 1], verts[vert2 + 2]);
		glm::vec3 v3 = glm::vec3(verts[vert3], verts[vert3 + 1], verts[vert3 + 2]);
		//cout << vertFaces[i] << ", " << vertFaces[i + 1] << ", " << vertFaces[i + 2] << endl;
		glm::vec3 n = genNormal(v1, v2, v3);

		//check to make sure this normal isn't redundant
		int index = findIndexIn(norms, FLOATS_PER_VERT, n);

		if (index == norms.size()) { //if it's a new normal
			norms.push_back(n[0]);
			norms.push_back(n[1]);
			norms.push_back(n[2]);
		}
		index /= FLOATS_PER_NORM;

		normFaces.push_back(index + 1);
		normFaces.push_back(index + 2);
		normFaces.push_back(index + 3);
	}
}

glm::vec3 genNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
	glm::vec3 e1 = v1 - v2;
	glm::vec3 e2 = v2 - v3;
	/*
	cout << v1 << endl;
	cout << v2 << endl;
	cout << v3 << endl;
	cout << "Cross: " << glm::cross(e1, e2) << endl;
	cout << "Normalize: " << glm::normalize(glm::cross(e1, e2)) << endl;
	*/

	return glm::normalize(glm::cross(e1, e2));
}

int findIndexIn(std::vector<GLfloat>& vecs, int stride, glm::vec3 vec) {
	unsigned int index;
	for (index = 0; index < vecs.size(); index += stride) {
		if (vec.x == vecs[index] && vec.y == vecs[index + 1] && vec.z == vecs[index + 2])
			return index;
	}
	return index;
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