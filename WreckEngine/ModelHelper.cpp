#include "ModelHelper.h"

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <fstream>

using namespace std;

namespace {
	ostream& operator<<(ostream& os, vec3 v) {
		os << "(" << to_string(v) << ")";
		return os;
	}
}

shared<Mesh> loadOBJ(const char* file) {
	//cout << "Loading " << file << endl;
	ifstream infile;
	infile.open(file, ios::in);
	if (!infile.is_open()) {
		cout << "Error! File " << file << " could not be read." << endl;
		return shared<Mesh>(nullptr);
	}
	else
		cout << "File " << file << " Loading..." << endl;

	vector<vec3> verts, normals, uvs;
	Mesh::Face faces;

	std::string line;
	while (getline(infile, line)) {
		auto tokens = tokenize(line, " ");
		if (line[0] == 'v') {
			//vertices
			if (line[1] == ' ') {
				verts.push_back(vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])));
			}
			//normals
			else if (line[1] == 'n') {
				normals.push_back(vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])));
			}
			//UVs
			else if (line[1] == 't') {
				uvs.push_back(vec3(stof(tokens[1]), stof(tokens[2]), 0));
			}
		}
		else if (line[0] == 'f') {
			for (size_t i = 1, numTokens = tokens.size(); i < numTokens; i++) {
				auto faceTokens = tokenize(tokens[i], "/");
				GLuint v = (GLuint)stoi(faceTokens[0]) - 1
					 , u = (GLuint)stoi(faceTokens[1]) - 1
					 , n = (GLuint)stoi(faceTokens[2]) - 1;
				faces.verts.push_back(v);
				faces.uvs.push_back(u);
				faces.normals.push_back(n);
			}
		}
	}
	
	cout << "Complete!" << endl;
	return make_shared<Mesh>(verts, normals, uvs, faces);
}

void genOBJ(const char* file, std::vector<GLfloat>& verts, std::vector<GLfloat>& uvs, std::vector<GLfloat>& norms
							, std::vector<GLuint>& vertFaces, std::vector<GLuint>& uvFaces, std::vector<GLuint>& normFaces) {

	//cout << "Generating " << file << endl;

	string fileContents = "";
	for (size_t i = 0, numVerts = verts.size(); i < numVerts; i += FLOATS_PER_VERT) {
		fileContents += "v";
		for (size_t j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(verts[i + j]);
		}
		fileContents += "\n";
	}
	fileContents += "# ";
	fileContents += to_string(verts.size() / FLOATS_PER_VERT);
	fileContents += " vertices\n\n";

	for (size_t i = 0, numUvs = uvs.size(); i < numUvs; i += FLOATS_PER_VERT) {
		fileContents += "vt";
		for (size_t j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(uvs[i + j]);//it's only floats per vert because otherwise I'd need special case for UV
		}
		fileContents += "\n";
	}
	fileContents += "# ";
	fileContents += to_string(uvs.size() / FLOATS_PER_VERT);
	fileContents += " UVs\n\n";

	for (size_t i = 0, numNorms = norms.size(); i < numNorms; i += FLOATS_PER_VERT) {
		fileContents += "vn";
		for (size_t j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(norms[i + j]);
		}
		fileContents += "\n";
	}
	fileContents += "# ";
	fileContents += to_string(norms.size() / FLOATS_PER_VERT);
	fileContents += " normals\n\n";

	for (size_t i = 0, numFaces = vertFaces.size(); i < numFaces; i += FLOATS_PER_VERT) {
		fileContents += "f";
		for (size_t j = 0; j < FLOATS_PER_VERT; j++) {
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
	//generate faces and normals
	vertFaces = {
		2, 1, 3,		3, 4, 2,
		6, 2, 4,		4, 8, 6,
		5, 6, 8,		8, 7, 5,
		1, 5, 7,		7, 3, 1,
		3, 7, 8,		8, 4, 3,
		2, 6, 5,		5, 1, 2
	};//that was tedious

	genNormals(verts,vertFaces,norms,normFaces);

	//generate UVs
	genUVs(verts, vertFaces, uvs, uvFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genCylinder(const char* file, size_t res) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;
	
	//generate verts and vert faces
	auto top    = vec3(0,  0.5, 0);
	auto bottom = vec3(0, -0.5, 0);
	verts.push_back(top.x); verts.push_back(top.y); verts.push_back(top.z);//0 = top middle
	verts.push_back(bottom.x); verts.push_back(bottom.y); verts.push_back(bottom.z);//1 = bottom middle
	
	//these are the tops and bottoms for each pair of verts, 0 is the very first (and would be last) ones added
	vec3 tcurr, bcurr;
	
	tcurr = vec3(0.5f,  0.5f, 0);
	bcurr = vec3(0.5f, -0.5f, 0);
	verts.push_back(tcurr.x); verts.push_back(tcurr.y); verts.push_back(tcurr.z);
	verts.push_back(bcurr.x); verts.push_back(bcurr.y); verts.push_back(bcurr.z);

	for (size_t i = 1; i < res; i++) {
		auto a = (float)(i * PI * 2 / res);
		tcurr = vec3(cosf(a),  1.0f, sinf(a)) * 0.5f;
		bcurr = vec3(cosf(a), -1.0f, sinf(a)) * 0.5f;
		
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
		vertFaces.push_back(2 * i + 1); vertFaces.push_back(1); vertFaces.push_back(2 * i + 2 + 1);
		//bottom face
		vertFaces.push_back(2 * i + 3 + 1); vertFaces.push_back(2); vertFaces.push_back(2 * i + 1 + 1);
		//side faces
		vertFaces.push_back(2 * i + 1 + 1); vertFaces.push_back(2 * i + 1); vertFaces.push_back(2 * i + 2 + 1);
		vertFaces.push_back(2 * i + 2 + 1); vertFaces.push_back(2 * i + 3 + 1); vertFaces.push_back(2 * i + 1 + 1);
	}
	//final top face
	vertFaces.push_back(2 * (res - 1) + 2 + 1); vertFaces.push_back(1); vertFaces.push_back(3);
	//final bottom face
	vertFaces.push_back(4); vertFaces.push_back(2); vertFaces.push_back(2 * (res - 1) + 3 + 1);
	//final side faces
	vertFaces.push_back(2 * (res - 1) + 3 + 1); vertFaces.push_back(2 * (res - 1) + 2 + 1); vertFaces.push_back(3);
	vertFaces.push_back(3); vertFaces.push_back(4); vertFaces.push_back(2 * (res - 1) + 3 + 1);

	//generate normals
	genNormals(verts, vertFaces, norms, normFaces);
	
	//generate UVs
	genUVCylindrical(verts, vertFaces, uvs, uvFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genCone(const char* file, size_t res) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;

	//generate verts and vert faces
	auto top = vec3(0, 0.5f, 0);
	auto bottom = vec3(0, -0.5f, 0);
	verts.push_back(top.x); verts.push_back(top.y); verts.push_back(top.z);//0 = top middle
	verts.push_back(bottom.x); verts.push_back(bottom.y); verts.push_back(bottom.z);//1 = bottom middle

	//these are the verts around the bottom

	auto p0 = vec3(0.5f, -0.5f, 0);
	verts.push_back(p0.x); verts.push_back(p0.y); verts.push_back(p0.z);

	for (size_t i = 1; i < res; ++i) {
		auto a = (float)(i * PI * 2 / res);
		auto curr = vec3(cosf(a), -1.0f, sinf(a)) * 0.5f;
		verts.push_back(curr.x); verts.push_back(curr.y); verts.push_back(curr.z);

		/*faces, side and bottom:
(top middle) 0 ---------- i + 2
		     |			/	  |
		     |		  /		  |
		     |		/		  |
		     |	  /			  |
		   i + 1 ------------ 1 (bottom middle)
		*/
		//bottom face
		vertFaces.push_back(i + 3); vertFaces.push_back(2); vertFaces.push_back(i + 2);
		//side face
		vertFaces.push_back(i + 2); vertFaces.push_back(1); vertFaces.push_back(i + 3);
	}
	//final bottom face
	vertFaces.push_back(3); vertFaces.push_back(2); vertFaces.push_back(res + 2);
	//final side face
	vertFaces.push_back(res + 2); vertFaces.push_back(1); vertFaces.push_back(3);

	//generate normals
	genNormals(verts, vertFaces, norms, normFaces);

	//generate UVs
	genUVCylindrical(verts, vertFaces, uvs, uvFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genSphere(const char* file, size_t res) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;

	//this adds the vertices in the first half circle, to prevent redundancy later
	//i is implied to be 0
	//once this is done, there are res vertices in verts, with the first and last ones not being repeated in subsequent ones, so res - 2 unique ones
	//top vert index is 0, bottom index is res
	for (size_t j = 0; j < res; j++) {
		auto x = cosf(PI / res * j) * 0.5f;
		auto y = sinf(PI / res * j) * 0.5f;
		auto z = 0.f;
		verts.push_back(x); verts.push_back(y); verts.push_back(z);
	}

	auto numVerts = (res - 1) * res + 1;
	for (size_t i = 1; i < res; i++) {
		//one notch down on the current half circle
		auto x1 = cosf(PI / res) * 0.5f;
		auto y1 = sinf(PI / res) * cosf((float)(2 * M_PI / res * i)) * 0.5f;
		auto z1 = sinf(PI / res) * sinf((float)(2 * M_PI / res * i)) * 0.5f;
		
		size_t ind = 0;//1,0,0 is always the first vertex added to verts; ind begins at the top and moves down
		size_t ind1 = i * (res - 1) + 1;//the offset accounts for the top and bottom
		size_t indp = 0;//prev half circle, begins at top and moves down
		size_t indp1 = (i - 1) * (res - 1) + 1;//one notch down prev half circle
		
		verts.push_back(x1); verts.push_back(y1); verts.push_back(z1);
		vertFaces.push_back(indp1 + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(ind + 1);

		indp = indp1; ind = ind1;

		for (size_t j = 1; j < res - 1; j++) {
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
		vertFaces.push_back(indp + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(ind + 1);
	}
	//bottom of sphere; goes in last
	verts.push_back(-0.5f); verts.push_back(0); verts.push_back(0);

	//add the last few vertFaces
	//i is assumed to be res
	size_t ind = 0;
	size_t ind1 = res * (res - 1) + 1;//the offset accounts for the top and bottom
	size_t indp = 0;//prev half circle, begins at top and moves down
	size_t indp1 = (res - 1) * (res - 1) + 1;//one notch down prev half circle
	
	//to account for the loop back
	ind1++;//this part I don't really get, but it works
	ind1 %= numVerts;
	indp1 %= numVerts;
	
	vertFaces.push_back(indp1 + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(ind + 1);
	
	indp = indp1; ind = ind1;

	for (size_t j = 1; j < res - 1; j++) {
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
	vertFaces.push_back(indp + 1); vertFaces.push_back(ind1 + 1); vertFaces.push_back(ind + 1);

	//generate normals
	genNormals(verts, vertFaces, norms, normFaces);
	//generate UVs
	genUVSpherical(verts, vertFaces, uvs, uvFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

void genBezierSurface(const char* file, size_t ures, size_t vres, std::vector<std::vector<vec3>>& k) {
	std::vector<GLfloat> verts, uvs, norms;
	std::vector<GLuint> vertFaces, uvFaces, normFaces;
	
	//first u0, i.e. 0
	for (size_t vi = 0; vi < vres + 1; vi++) {
		auto v0 = (float)vi / vres;
		auto a = bezierSurface(0, v0, k);//c is no longer a thing, as this handles it
		verts.push_back(a.x); verts.push_back(a.y); verts.push_back(a.z);
		uvs.push_back(0); uvs.push_back(v0); uvs.push_back(0);
	}
	for (size_t ui = 0; ui < ures; ui++) {
		auto u1 = (float)(ui + 1) / ures;

		vec3 b = bezierSurface(u1, 0, k);
		verts.push_back(b.x); verts.push_back(b.y); verts.push_back(b.z);

		uvs.push_back(u1); uvs.push_back(0); uvs.push_back(0);

		for (size_t vi = 0; vi < vres; vi++) {
			auto v1 = (float)(vi + 1) / vres;

			vec3 d = bezierSurface(u1, v1, k);

			verts.push_back(d.x); verts.push_back(d.y); verts.push_back(d.z);
			
			auto prev = ui * (vres + 1) + vi;//prev would be a, prev + 1 would be c
			auto curr = prev + vres + 1;//curr would be b, curr + 1 would be d
			vertFaces.push_back(curr + 1); vertFaces.push_back(prev + 1); vertFaces.push_back(prev + 1 + 1);
			vertFaces.push_back(prev + 1 + 1); vertFaces.push_back(curr + 1 + 1); vertFaces.push_back(curr + 1);

			uvs.push_back(u1); uvs.push_back(v1); uvs.push_back(0);

			uvFaces.push_back(curr + 1); uvFaces.push_back(prev + 1); uvFaces.push_back(prev + 1 + 1);
			uvFaces.push_back(prev + 1 + 1); uvFaces.push_back(curr + 1 + 1); uvFaces.push_back(curr + 1);
		}
	}

	genNormals(verts, vertFaces, norms, normFaces);

	genOBJ(file, verts, uvs, norms, vertFaces, uvFaces, normFaces);
}

vec3 bezierSurface(float u, float v, std::vector<std::vector<vec3>>& k) {
	vec3 p;
	int n = k.size(), m = k[0].size();
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
	//this is the partial factorial, canceling out part of the denominator
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
	for (size_t i = 0, numFaces = vertFaces.size(); i < numFaces; i += 3) {
		auto vert1 = (vertFaces[i]     - 1) * FLOATS_PER_VERT;
		auto vert2 = (vertFaces[i + 1] - 1) * FLOATS_PER_VERT;
		auto vert3 = (vertFaces[i + 2] - 1) * FLOATS_PER_VERT;
		auto v1 = vec3(verts[vert1], verts[vert1 + 1], verts[vert1 + 2]);
		auto v2 = vec3(verts[vert2], verts[vert2 + 1], verts[vert2 + 2]);
		auto v3 = vec3(verts[vert3], verts[vert3 + 1], verts[vert3 + 2]);
		auto n = genNormal(v1, v2, v3);

		//check to make sure this normal isn't redundant
		auto index = findIndexIn(norms, FLOATS_PER_VERT, n);

		//if it's a new normal
		if (index == norms.size()) { norms.push_back(n.x); norms.push_back(n.y); norms.push_back(n.z); }
		index /= FLOATS_PER_NORM;

		normFaces.push_back(index + 1);
		normFaces.push_back(index + 1);
		normFaces.push_back(index + 1);
	}
}

// depends on CLOCKWISE winding
vec3 genNormal(const vec3 a, const vec3 b, const vec3 c) {
	const auto e1 = b - a;
	const auto e2 = c - a;
	return glm::normalize(glm::cross(e1, e2));
}

size_t findIndexIn(const std::vector<GLfloat>& vecs, const size_t stride, const vec3 vec) {
	size_t index = 0;
	for (const auto numVecs = vecs.size(); index < numVecs; index += stride) {
		if (vec.x == vecs[index] && vec.y == vecs[index + 1] && vec.z == vecs[index + 2])
			return index;
	}
	return index;
}

std::vector<std::string> tokenize(std::string str, std::string delimiter) {
	std::vector<std::string> tokens;
	int curr, prev;
	for (curr = str.find(delimiter, 0), prev = 0; curr != string::npos; prev = curr + 1, curr = str.find(delimiter, prev)) {
		if (curr - prev > 0) {
			tokens.push_back(str.substr(prev, curr - prev));
		}
	}
	curr = str.length();
	if(curr - prev > 0)
		tokens.push_back(str.substr(prev, curr - prev));
	return tokens;
}