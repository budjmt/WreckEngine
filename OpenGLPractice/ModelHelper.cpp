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

Mesh* loadOBJ(const char* file) {
	//cout << "Loading " << file << endl;
	ifstream infile;
	infile.open(file, ios::in);
	if (!infile.is_open()) {
		cout << "Error! File " << file << " could not be read." << endl;
		return 0;
	}
	else
		cout << "File " << file << " Loaded" << endl;

	vector<glm::vec3> verts,normals,uvs;
	Face faces = Face();

	std::string line;
	while (getline(infile, line)) {
		
		std::vector<std::string> tokens = tokenize(line, " ");
		
		//vertices
		if(line.find("v ", 0) == 0) {
			verts.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])));
			//verts.push_back(1); verts.push_back(1); verts.push_back(1);
		}

		//normals
		else if (line.find("vn", 0) == 0) {
			normals.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])));
		}
		//uvs
		else if (line.find("vt", 0) == 0) {
			uvs.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), 0));
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
	//cout << "Complete!" << endl;
	Mesh* mesh = new Mesh(verts, normals, uvs, faces);
	return mesh;
}

void genOBJ(const char* file, std::vector<GLfloat>& verts, std::vector<GLfloat>& uvs, std::vector<GLfloat>& norms
							, std::vector<GLuint>& vertFaces, std::vector<GLuint>& uvFaces, std::vector<GLuint>& normFaces) {

	//cout << "Generating " << file << endl;

	string fileContents = "";
	for (unsigned int i = 0; i < verts.size(); i += FLOATS_PER_VERT) {
		fileContents += "v";
		for (int j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(verts[i + j]);
		}
		fileContents += "\n";
	}
	fileContents += "# ";
	fileContents += to_string(verts.size() / FLOATS_PER_VERT);
	fileContents += " vertices\n\n";

	for (unsigned int i = 0; i < uvs.size(); i += FLOATS_PER_VERT) {
		fileContents += "vt";
		for (int j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(uvs[i + j]);//it's only floats per vert because otherwise I'd need special case for UV
		}
		fileContents += "\n";
	}
	fileContents += "# ";
	fileContents += to_string(uvs.size() / FLOATS_PER_VERT);
	fileContents += " UVs\n\n";

	for (unsigned int i = 0; i < norms.size(); i += FLOATS_PER_VERT) {
		fileContents += "vn";
		for (int j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(norms[i + j]);
		}
		fileContents += "\n";
	}
	fileContents += "# ";
	fileContents += to_string(norms.size() / FLOATS_PER_VERT);
	fileContents += " normals\n\n";

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
	fileContents += "# ";
	fileContents += to_string(vertFaces.size());
	fileContents += " faces\n\n";

	//cout << "Writing..." << endl;
	//write fileContents to the file, overwrite its contents
	std::ofstream ofile;
	ofile.open(file, ios::out | ios::trunc);
	ofile.write(fileContents.c_str(), fileContents.length());
	ofile.close();
	//cout << "File successfully generated." << endl;
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
		2, 1, 3,		2, 4, 3,
		6, 2, 4,		6, 8, 4,
		5, 6, 8,		5, 7, 8,
		1, 5, 7,		1, 3, 7,
		3, 7, 8,		3, 4, 8,
		2, 6, 5,		2, 1, 5
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
	
	t0 = glm::vec3(0.5f, 0.5f, 0);
	b0 = glm::vec3(0.5f, -0.5f, 0);
	verts.push_back(t0.x); verts.push_back(t0.y); verts.push_back(t0.z);
	verts.push_back(b0.x); verts.push_back(b0.y); verts.push_back(b0.z);

	tprev = t0;
	bcurr = b0;
	for (int i = 1; i < res; i++) {
		float a = (float)(i * M_PI * 2 / res);
		tcurr = glm::vec3(cosf(a), 1.0f, sinf(a)) * 0.5f;
		bcurr = glm::vec3(cosf(a), -1.0f, sinf(a)) * 0.5f;
		
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
		vertFaces.push_back(2 * i + 2 + 1); vertFaces.push_back(1); vertFaces.push_back(2 * i + 1);
		//bottom face
		vertFaces.push_back(2 * i + 3 + 1); vertFaces.push_back(2); vertFaces.push_back(2 * i + 1 + 1);
		//side faces
		vertFaces.push_back(2 * i + 2 + 1); vertFaces.push_back(2 * i + 1); vertFaces.push_back(2 * i + 1 + 1);
		vertFaces.push_back(2 * i + 2 + 1); vertFaces.push_back(2 * i + 3 + 1); vertFaces.push_back(2 * i + 1 + 1);

		tprev = tprev;
		bprev = bprev;
	}
	//final top face
	vertFaces.push_back(3); vertFaces.push_back(1); vertFaces.push_back(2 * (res - 1) + 2 + 1);
	//final bottom face
	vertFaces.push_back(4); vertFaces.push_back(2); vertFaces.push_back(2 * (res - 1) + 3 + 1);
	//final side faces
	vertFaces.push_back(3); vertFaces.push_back(2 * (res - 1) + 2 + 1); vertFaces.push_back(2 * (res - 1) + 3 + 1);
	vertFaces.push_back(3); vertFaces.push_back(4); vertFaces.push_back(2 * (res - 1) + 3 + 1);

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
	//cout << "Row 1" << endl;
	for (int j = 0; j < res; j++) {
		float x = cosf((float)(M_PI / res * j)) * 0.5f;
		float y = sinf((float)(M_PI / res * j)) * 0.5f;
		float z = 0;
		verts.push_back(x); verts.push_back(y); verts.push_back(z);
	}

	int numVerts = (res - 1) * res + 1;
	for (int i = 1; i < res; i++) {
		//cout << "Row " << i << endl;
		//one notch down on the current half circle
		float x1 = cosf((float)(M_PI / res)) * 0.5f;
		float y1 = sinf((float)(M_PI / res)) * cosf((float)(2 * M_PI / res * i)) * 0.5f;
		float z1 = sinf((float)(M_PI / res)) * sinf((float)(2 * M_PI / res * i)) * 0.5f;
		
		int ind = 0;//1,0,0 is always the first vertex added to verts; ind begins at the top and moves down
		int ind1 = i * (res - 1) + 1;//the offset accounts for the top and bottom
		int indp = 0;//prev half circle, begins at top and moves down
		int indp1 = (i - 1) * (res - 1) + 1;//one notch down prev half circle
		
		verts.push_back(x1); verts.push_back(y1); verts.push_back(z1);
		vertFaces.push_back(ind + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(indp1 + 1);

		indp = indp1; ind = ind1;

		for (int j = 1; j < res - 1; j++) {
			int nextCircle = j + 1;
			//one notch down on the current half circle
			x1 = cosf((float)(M_PI / res * nextCircle)) * 0.5f;
			y1 = sinf((float)(M_PI / res * nextCircle)) * cosf((float)(2 * M_PI / res * i)) * 0.5f;
			z1 = sinf((float)(M_PI / res * nextCircle)) * sinf((float)(2 * M_PI / res * i)) * 0.5f;

			ind1 = i * (res - 1) + 1 + j;//x1, y1, z1 is always new and always the last one added
			indp1 = (i - 1) * (res - 1) + 1 + j;//one notch down on previous half circle

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
			vertFaces.push_back(ind + 1); vertFaces.push_back(indp + 1); vertFaces.push_back(indp1 + 1);
			vertFaces.push_back(indp1 + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(ind + 1);
			//update current with next
			indp = indp1; ind = ind1;
		}
		ind1 = numVerts;
		indp1 = numVerts;
		vertFaces.push_back(ind + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(indp + 1);
	}
	//bottom of sphere; goes in last
	verts.push_back(-0.5f); verts.push_back(0); verts.push_back(0);

	//add the last few vertFaces
	//i is assumed to be res
	int ind = 0;
	int ind1 = res * (res - 1) + 1;//the offset accounts for the top and bottom
	int indp = 0;//prev half circle, begins at top and moves down
	int indp1 = (res - 1) * (res - 1) + 1;//one notch down prev half circle
	
	//to account for the loop back
	ind1++;//this part I don't really get, but it works
	ind1 %= numVerts;
	indp1 %= numVerts;
	
	vertFaces.push_back(ind + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(indp1 + 1);
	
	indp = indp1; ind = ind1;

	//cout << "Row " << res << endl;
	for (int j = 1; j < res - 1; j++) {
		ind1 = res * (res - 1) + 1 + j;//x1, y1, z1 is always new and always the last one added
		indp1 = (res - 1) * (res - 1) + 1 + j;//one notch down on previous half circle
		//account for the loop back
		ind1++;
		ind1 %= numVerts;
		indp1 %= numVerts;
		//add the vertFace information
		vertFaces.push_back(ind + 1); vertFaces.push_back(indp + 1); vertFaces.push_back(indp1 + 1);
		vertFaces.push_back(indp1 + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(ind + 1);
		//update current with next
		//these keep their data from the previous loop
		indp = indp1; ind = ind1;
	}
	ind1 = numVerts;
	indp1 = numVerts;
	vertFaces.push_back(ind + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(indp + 1);

	//generate normals
	genNormals(verts, vertFaces, norms, normFaces);
	//cout << "Normals done" << endl;
	//generate uvs
	genUVSpherical(verts, vertFaces, uvs, uvFaces);
	//cout << "UVs done" << endl;

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genBezierSurface(const char* file, int ures, int vres, std::vector<std::vector<glm::vec3>>& k) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;
	
	//first u0, i.e. 0
	for (int vi = 0; vi < vres + 1; vi++) {
		float v0 = (float)vi / vres;
		glm::vec3 a = bezierSurface(0, v0, k);//c is no longer a thing, as this handles it
		verts.push_back(a.x); verts.push_back(a.y); verts.push_back(a.z);
		uvs.push_back(0); uvs.push_back(v0); uvs.push_back(0);
	}
	for (int ui = 0; ui < ures; ui++) {
		//float u0 = (float)ui / ures;
		float u1 = (float)(ui + 1) / ures;

		glm::vec3 b = bezierSurface(u1, 0, k);
		verts.push_back(b.x); verts.push_back(b.y); verts.push_back(b.z);

		uvs.push_back(u1); uvs.push_back(0); uvs.push_back(0);

		for (int vi = 0; vi < vres; vi++) {
			//float v0 = (float)vi / vres;
			float v1 = (float)(vi + 1) / vres;

			//glm::vec3 c = bezierSurface(u0, v1, k);
			glm::vec3 d = bezierSurface(u1, v1, k);

			//verts.push_back(c.x); verts.push_back(c.y); verts.push_back(c.z);
			verts.push_back(d.x); verts.push_back(d.y); verts.push_back(d.z);
			
			int prev = ui * (vres + 1) + vi;//prev would be a, prev + 1 would be c
			int curr = prev + vres + 1;//curr would be b, curr + 1 would be d
			vertFaces.push_back(curr + 1); vertFaces.push_back(prev + 1); vertFaces.push_back(prev + 1 + 1);
			vertFaces.push_back(prev + 1 + 1); vertFaces.push_back(curr + 1 + 1); vertFaces.push_back(curr + 1);

			//uvs.push_back(u0); uvs.push_back(v1); uvs.push_back(0);
			uvs.push_back(u1); uvs.push_back(v1); uvs.push_back(0);

			uvFaces.push_back(curr + 1); uvFaces.push_back(prev + 1); uvFaces.push_back(prev + 1 + 1);
			uvFaces.push_back(prev + 1 + 1); uvFaces.push_back(curr + 1 + 1); uvFaces.push_back(curr + 1);
		}
	}

	genNormals(verts, vertFaces, norms, normFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

glm::vec3 bezierSurface(float u, float v, std::vector<std::vector<glm::vec3>>& k) {
	glm::vec3 p;
	int n = k.size();
	int m = k[0].size();
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			p += k[i][j] * bernsteinPolynomial(i, n - 1, u) * bernsteinPolynomial(j, m - 1, v);
		}
	}
	return p;
}

float bernsteinPolynomial(int i, int n, float u) {
	return binomialCoeff(n, i) * pow(u, i) * pow(1 - u, n - i);
}

float binomialCoeff(int n, int i) {
	//this is the partial factorial, cancelling out part of the denominator
	int num = 1;
	int start, den;
	if (i > n - i) { start = i; den = n - i; }
	else { start = n - i; den = i; }
	for (int k = start + 1; k < n + 1; k++)
		num *= k;
	return (float)num / factorial(den);
}

int factorial(int n) {
	int fact = 1;
	for (int i = 2; i < n + 1; i++) 
		fact *= i;
	return fact;
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
	glm::vec3 e2 = v3 - v2;
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