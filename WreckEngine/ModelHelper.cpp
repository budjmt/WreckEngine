#include "ModelHelper.h"

#include <iostream>
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

	Mesh::FaceData data;
	Mesh::FaceIndex indices;

	std::string line;
	while (getline(infile, line)) {
		auto tokens = tokenize(line, " ");
        switch (line[0]) {
        case 'v':
            switch (line[1]) {
            case ' ': // vertices
                data.verts.push_back(vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])));
                break;
            case 'n': // normals
                data.normals.push_back(vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])));
                break;
            case 't': // UVs
                data.uvs.push_back(vec3(stof(tokens[1]), stof(tokens[2]), 0));
                break;
            }
            break;
        case 'f': // faces
            for (size_t i = 1, numTokens = tokens.size(); i < numTokens; i++) {
                auto faceTokens = tokenize(tokens[i], "/");
                auto v = (GLuint) stoi(faceTokens[0]) - 1
                   , u = (GLuint) stoi(faceTokens[1]) - 1
                   , n = (GLuint) stoi(faceTokens[2]) - 1;
                indices.verts.push_back(v);
                indices.uvs.push_back(u);
                indices.normals.push_back(n);
            }
            break;
        }
	}
	
	cout << "Complete!" << endl;
	return make_shared<Mesh>(data, indices);
}

void genOBJ(const char* file, Mesh::FaceData& data, Mesh::FaceIndex& indices) {

	//cout << "Generating " << file << endl;

	string fileContents = "";
	for (const auto vert : data.verts) {
		fileContents += "v " + to_string(vert.x) + " " + to_string(vert.y) + " " + to_string(vert.z) + "\n";
	}
	fileContents += "# " + to_string(data.verts.size()) + " vertices\n\n";

	for (const auto uv : data.uvs) {
		fileContents += "vt " + to_string(uv.x) + " " + to_string(uv.y) + " " + to_string(uv.z) + "\n";
	}
	fileContents += "# " + to_string(data.verts.size()) + " UVs\n\n";

	for (const auto normal : data.normals) {
		fileContents += "vn " + to_string(normal.x) + " " + to_string(normal.y) + " " + to_string(normal.z) + "\n";
	}
	fileContents += "# " + to_string(data.verts.size()) + " normals\n\n";

	for (size_t i = 0, numFaces = indices.verts.size(); i < numFaces; i += FLOATS_PER_VERT) {
		fileContents += "f";
		for (size_t j = 0; j < FLOATS_PER_VERT; j++) {
			fileContents += " ";
			fileContents += to_string(indices.verts[i + j]);
			fileContents += "/";
			fileContents += to_string(indices.uvs[i + j]);
			fileContents += "/";
			fileContents += to_string(indices.normals[i + j]);
		}
		fileContents += "\n";
	}
	fileContents += "# " + to_string(indices.verts.size() / FLOATS_PER_VERT) + " faces\n\n";

	//cout << "Writing..." << endl;
	//write fileContents to the file, overwrite its contents
	std::ofstream ofile;
	ofile.open(file, ios::out | ios::trunc);
	ofile.write(fileContents.c_str(), fileContents.length());
	ofile.close();
	//cout << "File successfully generated." << endl;
}

void genCube(const char* file) {
	Mesh::FaceData data;
	Mesh::FaceIndex indices;
	
	//generate verts
	data.verts = {
		{  0.5f,  0.5f,  0.5f },
		{  0.5f,  0.5f, -0.5f },
		{  0.5f, -0.5f,  0.5f },
		{  0.5f, -0.5f, -0.5f },
		{ -0.5f,  0.5f,  0.5f },
		{ -0.5f,  0.5f, -0.5f },
		{ -0.5f, -0.5f,  0.5f },
		{ -0.5f, -0.5f, -0.5f }
	};
	//generate faces and normals
	indices.verts = {
		2, 1, 3,		3, 4, 2,
		6, 2, 4,		4, 8, 6,
		5, 6, 8,		8, 7, 5,
		1, 5, 7,		7, 3, 1,
		3, 7, 8,		8, 4, 3,
		2, 6, 5,		5, 1, 2
	};//that was tedious

	genNormals(data, indices);

	//generate UVs
	genUVs(data, indices);

	genOBJ(file, data, indices);
}

void genCylinder(const char* file, size_t res) {
	Mesh::FaceData data;
	Mesh::FaceIndex indices;
	
	//generate verts and vert faces
	auto top    = vec3(0,  0.5, 0);
	auto bottom = vec3(0, -0.5, 0);
	data.verts.push_back(top);    // 0 = top middle
	data.verts.push_back(bottom); // 1 = bottom middle
	
	//these are the tops and bottoms for each pair of verts, 0 is the very first (and would be last) ones added
	vec3 tcurr, bcurr;
	
	tcurr = vec3(0.5f,  0.5f, 0);
	bcurr = vec3(0.5f, -0.5f, 0);
	data.verts.push_back(tcurr);
	data.verts.push_back(bcurr);

	for (size_t i = 1; i < res; ++i) {
		auto a = (float)(i * PI * 2 / res);
		tcurr = vec3(cosf(a),  1.0f, sinf(a)) * 0.5f;
		bcurr = vec3(cosf(a), -1.0f, sinf(a)) * 0.5f;
		
		data.verts.push_back(tcurr);
		data.verts.push_back(bcurr);

		/*side faces:
		2 * i --- 2 * i + 2
		|			/	  |
		|		  /		  |
		|		/		  |
		|	  /			  |
		2 * i + 1 --- 2 * i + 3
		*/
		//top face
		indices.verts.push_back(2 * i + 1); indices.verts.push_back(1); indices.verts.push_back(2 * i + 2 + 1);
		//bottom face
		indices.verts.push_back(2 * i + 3 + 1); indices.verts.push_back(2); indices.verts.push_back(2 * i + 1 + 1);
		//side faces
		indices.verts.push_back(2 * i + 1 + 1); indices.verts.push_back(2 * i + 1);     indices.verts.push_back(2 * i + 2 + 1);
		indices.verts.push_back(2 * i + 2 + 1); indices.verts.push_back(2 * i + 3 + 1); indices.verts.push_back(2 * i + 1 + 1);
	}
	//final top face
	indices.verts.push_back(2 * (res - 1) + 2 + 1); indices.verts.push_back(1); indices.verts.push_back(3);
	//final bottom face
	indices.verts.push_back(4); indices.verts.push_back(2); indices.verts.push_back(2 * (res - 1) + 3 + 1);
	//final side faces
	indices.verts.push_back(2 * (res - 1) + 3 + 1); indices.verts.push_back(2 * (res - 1) + 2 + 1); indices.verts.push_back(3);
	indices.verts.push_back(3);                     indices.verts.push_back(4);                     indices.verts.push_back(2 * (res - 1) + 3 + 1);

	//generate normals
	genNormals(data, indices);
	
	//generate UVs
	genUVCylindrical(data, indices);

	genOBJ(file, data, indices);
}

void genCone(const char* file, size_t res) {
	Mesh::FaceData data;
	Mesh::FaceIndex indices;

	//generate verts and vert faces
	auto top = vec3(0, 0.5f, 0);
	auto bottom = vec3(0, -0.5f, 0);
	data.verts.push_back(top);    // 0 = top middle
	data.verts.push_back(bottom); // 1 = bottom middle

	//these are the verts around the bottom
	auto p0 = vec3(0.5f, -0.5f, 0);
	data.verts.push_back(p0);

	for (size_t i = 1; i < res; ++i) {
		const auto a = (float)(i * PI * 2 / res);
		const auto curr = vec3(cosf(a), -1.0f, sinf(a)) * 0.5f;
		data.verts.push_back(curr);

		/*faces, side and bottom:
(top middle) 0 ---------- i + 2
		     |			/	  |
		     |		  /		  |
		     |		/		  |
		     |	  /			  |
		   i + 1 ------------ 1 (bottom middle)
		*/
		//bottom face
		indices.verts.push_back(i + 3); indices.verts.push_back(2); indices.verts.push_back(i + 2);
		//side face
		indices.verts.push_back(i + 2); indices.verts.push_back(1); indices.verts.push_back(i + 3);
	}
	//final bottom face
	indices.verts.push_back(3);       indices.verts.push_back(2); indices.verts.push_back(res + 2);
	//final side face
	indices.verts.push_back(res + 2); indices.verts.push_back(1); indices.verts.push_back(3);

	//generate normals
	genNormals(data, indices);

	//generate UVs
	genUVCylindrical(data, indices);

	genOBJ(file, data, indices);
}

void genSphere(const char* file, size_t res) {
	Mesh::FaceData data;
	Mesh::FaceIndex indices;

	//this adds the vertices in the first half circle, to prevent redundancy later
	//i is implied to be 0
	//once this is done, there are res vertices in verts, with the first and last ones not being repeated in subsequent ones, so res - 2 unique ones
	//top vert index is 0, bottom index is res
	const auto pi_res = PI / res, pi_res2 = 2 * pi_res;
	for (size_t j = 0; j < res; ++j) {
		data.verts.push_back(vec3(cosf(pi_res * j) * 0.5f, sinf(pi_res * j) * 0.5f, 0.f));
	}

	const auto numVerts = (res - 1) * res + 1;
	for (size_t i = 1; i < res; ++i) {
		// one notch down on the current half circle
		auto x1 = cosf(pi_res) * 0.5f;
		auto y1 = sinf(pi_res) * cosf(pi_res2 * i) * 0.5f;
		auto z1 = sinf(pi_res) * sinf(pi_res2 * i) * 0.5f;
		
		size_t ind   = 0; // 1,0,0 is always the first vertex added to verts; ind begins at the top and moves down
		size_t ind1  =  i      * (res - 1) + 1; // the offset accounts for the top and bottom
		size_t indp  = 0; // prev half circle, begins at top and moves down
		size_t indp1 = (i - 1) * (res - 1) + 1; // one notch down prev half circle
		
		data.verts.push_back(vec3(x1, y1, z1));
		indices.verts.push_back(indp1 + 1); indices.verts.push_back(ind1 + 1); indices.verts.push_back(ind + 1);

		indp = indp1; ind = ind1;

		for (size_t j = 1; j < res - 1; j++) {
			int nextCircle = j + 1;
			//one notch down on the current half circle
			x1 = cosf(pi_res * nextCircle) * 0.5f;
			y1 = sinf(pi_res * nextCircle) * cosf(pi_res2 * i) * 0.5f;
			z1 = sinf(pi_res * nextCircle) * sinf(pi_res2 * i) * 0.5f;

			ind1  =  i      * (res - 1) + 1 + j;//x1, y1, z1 is always new and always the last one added
			indp1 = (i - 1) * (res - 1) + 1 + j;//one notch down on previous half circle

			//add only the next verts, as current ones will always be already added
			data.verts.push_back(vec3(x1, y1, z1));
			
			/*
			indp	---		ind
			|			/	  |
			|		  /		  |
			|		/		  |
			|	  /			  |
			indp1	---		ind1
			*/
			indices.verts.push_back(ind + 1);   indices.verts.push_back(indp + 1); indices.verts.push_back(indp1 + 1);
			indices.verts.push_back(indp1 + 1); indices.verts.push_back(ind1 + 1); indices.verts.push_back(ind + 1);
			//update current with next
			indp = indp1; ind = ind1;
		}
		ind1  = numVerts;
		indp1 = numVerts;
		indices.verts.push_back(indp + 1); indices.verts.push_back(ind1 + 1); indices.verts.push_back(ind + 1);
	}
	//bottom of sphere; goes in last
	data.verts.push_back(vec3(-0.5f, 0, 0));

	//add the last few vertFaces
	//i is assumed to be res
	size_t ind   = 0;
	size_t ind1  = 1;//the offset accounts for the top and bottom
	size_t indp  = 0;//prev half circle, begins at top and moves down
	size_t indp1 = numVerts - res + 1, nV_1 = indp1;//one notch down prev half circle
	
	indices.verts.push_back(indp1 + 1); indices.verts.push_back(ind1 + 1); indices.verts.push_back(ind + 1);
	
	indp = indp1; ind = ind1;

	for (size_t j = 1; j < res - 1; ++j) {
		ind1  = numVerts + j;// x1, y1, z1 is always new and always the last one added
		indp1 = nV_1 + j;    // one notch down on previous half circle
		//account for the loop back
		++ind1;
		ind1  %= numVerts;
		indp1 %= numVerts;
		//add the vertFace information
		indices.verts.push_back(ind + 1);   indices.verts.push_back(indp + 1); indices.verts.push_back(indp1 + 1);
		indices.verts.push_back(indp1 + 1); indices.verts.push_back(ind1 + 1); indices.verts.push_back(ind + 1);
		//update current with next
		//these keep their data from the previous loop
		indp = indp1; ind = ind1;
	}
	ind1  = numVerts;
	indp1 = numVerts;
	indices.verts.push_back(indp + 1); indices.verts.push_back(ind1 + 1); indices.verts.push_back(ind + 1);

	//generate normals
	genNormals(data, indices);
	//generate UVs
	genUVSpherical(data, indices);

	genOBJ(file, data, indices);
}

void genBezierSurface(const char* file, size_t ures, size_t vres, std::vector<std::vector<vec3>>& k) {
	Mesh::FaceData data;
	Mesh::FaceIndex indices;
	
	//first u0, i.e. 0
	for (size_t vi = 0; vi < vres + 1; ++vi) {
		auto v0 = (float)vi / vres;
		auto a = bezierSurface(0, v0, k);//c is no longer a thing, as this handles it
		data.verts.push_back(a);
		data.uvs.push_back(vec3(0, v0, 0));
	}
	for (size_t ui = 0; ui < ures; ++ui) {
		auto u1 = (float)(ui + 1) / ures;

		auto b = bezierSurface(u1, 0, k);
		data.verts.push_back(b);

		data.uvs.push_back(vec3(u1, 0, 0));

		for (size_t vi = 0; vi < vres; ++vi) {
			auto v1 = (float)(vi + 1) / vres;

			auto d = bezierSurface(u1, v1, k);
			data.verts.push_back(d);
			
			auto prev = ui * (vres + 1) + vi;//prev would be a, prev + 1 would be c
			auto curr = prev + vres + 1;//curr would be b, curr + 1 would be d
			indices.verts.push_back(curr + 1);     indices.verts.push_back(prev + 1);     indices.verts.push_back(prev + 1 + 1);
			indices.verts.push_back(prev + 1 + 1); indices.verts.push_back(curr + 1 + 1); indices.verts.push_back(curr + 1);

			data.uvs.push_back(vec3(u1, v1, 0));

			indices.uvs.push_back(curr + 1);     indices.uvs.push_back(prev + 1);     indices.uvs.push_back(prev + 1 + 1);
			indices.uvs.push_back(prev + 1 + 1); indices.uvs.push_back(curr + 1 + 1); indices.uvs.push_back(curr + 1);
		}
	}

	genNormals(data, indices);

	genOBJ(file, data, indices);
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
	return binomialCoeff(n, i) * std::pow(u, i) * std::pow(1 - u, n - i);
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

void genNormals(Mesh::FaceData& data, Mesh::FaceIndex& indices) {
	for (size_t i = 0, numFaces = indices.verts.size(); i < numFaces; i += 3) {
		const auto& v1 = data.verts[indices.verts[i]     - 1];
		const auto& v2 = data.verts[indices.verts[i + 1] - 1];
		const auto& v3 = data.verts[indices.verts[i + 2] - 1];
		auto n = genNormal(v1, v2, v3);

		//check to make sure this normal isn't redundant
		auto index = find_index(data.normals, n);

		//if it's a new normal
		if (index >= data.normals.size()) data.normals.push_back(n);

		indices.normals.push_back(index + 1);
		indices.normals.push_back(index + 1);
		indices.normals.push_back(index + 1);
	}
}

// depends on CLOCKWISE winding
vec3 genNormal(const vec3 a, const vec3 b, const vec3 c) {
	const auto e1 = b - a;
	const auto e2 = c - a;
	return glm::normalize(glm::cross(e1, e2));
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