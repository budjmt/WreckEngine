#pragma once

#include <fstream>

#include "gl_structs.h"
#include "Mesh.h"

#include "FreeImage.h"

inline std::vector<std::string> tokenize(std::string str, std::string delimiter) {
    std::vector<std::string> tokens;
    int curr, prev;
    for (curr = str.find(delimiter, 0), prev = 0; curr != std::string::npos; prev = curr + 1, curr = str.find(delimiter, prev)) {
        if (curr - prev > 0) {
            tokens.push_back(str.substr(prev, curr - prev));
        }
    }
    curr = str.length();
    if (curr - prev > 0)
        tokens.push_back(str.substr(prev, curr - prev));
    return tokens;
}

namespace File {

    enum class Extension {
        TXT,
        GLSL,
        PNG,
        OBJ
    };
    
    struct ImageData {
        unsigned char* bytes;
        shared<FIBITMAP> image{ nullptr, [](FIBITMAP* image) { if (image) FreeImage_Unload(image); } };
        size_t width, height;
    };

    template<Extension E> class resource {};
    template<> struct resource<Extension::TXT>  { typedef unique<const char[]> res_t; };
    template<> struct resource<Extension::GLSL> { typedef GLshader res_t; };
    template<> struct resource<Extension::PNG>  { typedef ImageData res_t; };
    template<> struct resource<Extension::OBJ>  { typedef shared<Mesh> res_t; };

    template<Extension E> using resource_t = typename resource<E>::res_t;

    template<Extension E>
    inline resource_t<E> load(const char* path, const uint32_t options = 0) {}

    template<Extension E> inline bool isValid(resource_t<E>&) { return false; }
    template<> inline bool isValid<Extension::TXT>(resource_t<Extension::TXT>& res) { return res != nullptr; }
    template<> inline bool isValid<Extension::GLSL>(resource_t<Extension::GLSL>& res) { return res.valid(); }
    template<> inline bool isValid<Extension::PNG>(resource_t<Extension::PNG>& res) { return res.image != nullptr; }
    template<> inline bool isValid<Extension::OBJ>(resource_t<Extension::OBJ>& res) { return res != nullptr; }

    template<>
    inline resource_t<Extension::TXT> load<Extension::TXT>(const char* path, const uint32_t) {
        using namespace std;
        ifstream infile;
        infile.open(path, ios::binary);
        if (infile.is_open()) {
            infile.seekg(0, ios::end);
            int length = (int)infile.tellg();
            infile.seekg(0, ios::beg);

            auto filecontents = new char[length + 1];
            infile.read(filecontents, length);
            filecontents[length] = 0;
            infile.close();
            return ::unique<const char[]>(filecontents);
        }
        return nullptr;
    }

    template<>
    inline resource_t<Extension::GLSL> load<Extension::GLSL>(const char* path, const GLenum shaderType) {
        auto fileContents = load<Extension::TXT>(path);
        if (!fileContents) {
            printf("Error! File %s could not be read.\n", path);
            return GLshader();
        }

        GLshader shader;
        shader.create(fileContents.get(), shaderType);

        if (shader.getVal(GL_COMPILE_STATUS) == GL_TRUE)
            return shader;

        auto logLength = shader.getVal(GL_INFO_LOG_LENGTH);
        auto log = std::vector<char>(logLength);
        glGetShaderInfoLog(shader(), logLength, 0, &log[0]);
        printf("Error in file %s: \n%s\n", path, &log[0]);
        return GLshader();
    }

    template<>
    inline resource_t<Extension::PNG> load<Extension::PNG>(const char* path, const uint32_t) {
        ImageData data{};
        auto bitmap = FreeImage_Load(FreeImage_GetFileType(path), path);
        if (!bitmap) {
            return data;
        }
        //we convert the 24bit bitmap to 32bits
        auto image = FreeImage_ConvertTo32Bits(bitmap);
        //delete the 24bit bitmap from memory
        FreeImage_Unload(bitmap);
        if (!image) {
            return data;
        }
        
        data.image.reset(image);
        data.width  = FreeImage_GetWidth(image);
        data.height = FreeImage_GetHeight(image);
        data.bytes  = FreeImage_GetBits(image);
        return data;
    }

    template<>
    inline resource_t<Extension::OBJ> load<Extension::OBJ>(const char* path, const uint32_t) {
        using namespace std;
        ifstream infile;
        infile.open(path, ios::in);
        if (!infile.is_open()) {
            printf("Error! File %s could not be read.\n", path);
            return shared<Mesh>(nullptr);
        }
        else
            printf("File %s Loading...\n", path);

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
                    auto v = (GLuint)stoi(faceTokens[0]) - 1
                       , u = (GLuint)stoi(faceTokens[1]) - 1
                       , n = (GLuint)stoi(faceTokens[2]) - 1;
                    indices.verts.push_back(v);
                    indices.uvs.push_back(u);
                    indices.normals.push_back(n);
                }
                break;
            }
        }

        printf("Complete!\n");
        return make_shared<Mesh>(data, indices);
    }
};