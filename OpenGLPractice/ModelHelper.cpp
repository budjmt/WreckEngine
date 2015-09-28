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