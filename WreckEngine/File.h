#pragma once

#include <fstream>

#include "gl_structs.h"
#include "Mesh.h"

#include <stb_image.h>
#include <stb_image_write.h>

// tokenizes [str] on any character present in [delimiter]
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
#if WR_USE_FREEIMAGE
        unsigned char* bytes;
        FIBITMAP* image;
        void unload() { if (image) FreeImage_Unload(image); }
#else
        shared<unsigned char> bytes;
#endif
        size_t width, height;
        inline operator bool() const { return static_cast<bool>(bytes); }
    };

    template<Extension E> struct resource {};
    template<> struct resource<Extension::TXT>  { typedef std::string type; };
    template<> struct resource<Extension::GLSL> { typedef GLshader type; };
    template<> struct resource<Extension::PNG>  { typedef ImageData type; };
    template<> struct resource<Extension::OBJ>  { typedef shared<Mesh> type; };

    template<Extension E> using resource_t = typename resource<E>::type;

    template<Extension E> struct blob {};
    template<> struct blob<Extension::TXT>  { typedef unique<unsigned char[]> type; };
    template<> struct blob<Extension::PNG>  { typedef GLtexture type; };
    template<> struct blob<Extension::OBJ>  { typedef blob<Extension::TXT>::type type; };

    template<Extension E> using blob_t = typename blob<E>::type;

    template<Extension E> struct options {};
    template<> struct options<Extension::TXT> { size_t size; uint32_t flags; };
    template<> struct options<Extension::PNG> {
        int width, height;
        GLenum target, format = GL_RGBA, level = 0;
        uint32_t flags;
        std::function<void(bool)> callback;
    };
    template<> struct options<Extension::OBJ> : public options<Extension::TXT> {};

    template<Extension E>
    inline resource_t<E> load(const char* path, const uint32_t options = 0) {}

    template<Extension E>
    inline void save(const char* path, blob_t<E> data, const options<E> options) {}

    template<Extension E> inline bool isValid(const resource_t<E>&) { return false; }
    template<> inline bool isValid<Extension::TXT>  (const resource_t<Extension::TXT>&  res) { return res.length() > 0; }
    template<> inline bool isValid<Extension::GLSL> (const resource_t<Extension::GLSL>& res) { return res.valid(); }
    template<> inline bool isValid<Extension::PNG>  (const resource_t<Extension::PNG>&  res) { return res; }
    template<> inline bool isValid<Extension::OBJ>  (const resource_t<Extension::OBJ>&  res) { return res != nullptr; }

    template<>
    inline resource_t<Extension::TXT> load<Extension::TXT>(const char* path, const uint32_t) {
        using namespace std;

        std::string fileContents;
        ifstream infile(path, ios::binary);
        if (infile.is_open()) {
            infile.seekg(0, ios::end);
            auto length = (size_t) infile.tellg();
            infile.seekg(0, ios::beg);

            fileContents.resize(length);
            infile.read(&fileContents[0], length);
            //filecontents[length] = 0;
        }
        return std::move(fileContents);
    }

    void processShaderSource(const char* path, resource_t<Extension::TXT>& resource);

    template<>
    inline resource_t<Extension::GLSL> load<Extension::GLSL>(const char* path, const GLenum shaderType) {
        GLshader shader;
        auto fileContents = load<Extension::TXT>(path);
        if (!isValid<Extension::TXT>(fileContents)) {
            printf("Error! File %s could not be read.\n", path);
            return shader;
        }

        processShaderSource(path, fileContents);

        shader.create(fileContents.c_str(), (GLshader::Type) shaderType);

        if (shader.getVal(GL_COMPILE_STATUS) == GL_TRUE)
            return shader;

        auto logLength = shader.getVal(GL_INFO_LOG_LENGTH);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader(), logLength, 0, &log[0]);
        printf("Error in file %s: \n%s\n", path, &log[0]);
        return GLshader();
    }

    template<>
    inline resource_t<Extension::PNG> load<Extension::PNG>(const char* path, const uint32_t) {
        ImageData data{};

#if WR_USE_FREEIMAGE
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

        data.image  = image;
        data.width  = FreeImage_GetWidth(image);
        data.height = FreeImage_GetHeight(image);
        data.bytes  = FreeImage_GetBits(image);
#else
        constexpr auto requiredFormat = STBI_rgb_alpha;
        int w, h, channels;
        auto bytes = stbi_load(path, &w, &h, &channels, requiredFormat);

        data.bytes = shared<unsigned char>(std::move(bytes), [](void* mem) { stbi_image_free(mem); });
        data.width = static_cast<size_t>(w);
        data.height = static_cast<size_t>(h);
#endif

        return data;
    }

    template<>
    inline resource_t<Extension::OBJ> load<Extension::OBJ>(const char* path, const uint32_t) {
        using namespace std;

        ifstream infile(path);
        if (!infile.is_open()) {
            printf("Error! File %s could not be read.\n", path);
            return shared<Mesh>();
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
                    data.verts.push_back({ stof(tokens[1]), stof(tokens[2]), stof(tokens[3]) });
                    break;
                case 'n': // normals
                    data.normals.push_back({ stof(tokens[1]), stof(tokens[2]), stof(tokens[3]) });
                    break;
                case 't': // UVs
                    data.uvs.push_back({ stof(tokens[1]), stof(tokens[2]), 0 });
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
        return make_shared<Mesh>(std::move(data), std::move(indices));
    }

    template<>
    inline void save<Extension::TXT>(const char* path, blob_t<Extension::TXT> data, const options<Extension::TXT> options) {
        std::ofstream os(path, options.flags);
        os.write((char*) data.get(), options.size);
    }

    template<>
    inline void save<Extension::OBJ>(const char* path, blob_t<Extension::OBJ> data, const options<Extension::OBJ> options) {
        static_assert(std::is_same<blob_t<Extension::OBJ>, blob_t<Extension::TXT>>::value , "This method is now invalid because txt and obj are not using the same type");
        save<Extension::TXT>(path, std::move(data), options);
    }

    template<>
    inline void save<Extension::PNG>(const char* path, const blob_t<Extension::PNG> data, const options<Extension::PNG> options) {
        Thread::Render::runNextFrame([=] {
            data.bind();

            GLint width, height;
            GLenum format;
            GL_CHECK(glGetTexParameteriv(data.target, GL_TEXTURE_WIDTH, &width));
            GL_CHECK(glGetTexParameteriv(data.target, GL_TEXTURE_HEIGHT, &height));
            GL_CHECK(glGetTexParameteriv(data.target, GL_TEXTURE_INTERNAL_FORMAT, (GLint*) &format));

            auto pitch = GLtexture::getFormatPitch(format);
            auto pixels = make_unique<unsigned char[]>(width * height * pitch);

            GL_CHECK(glGetTexImage(options.target, options.level, options.format, GL_UNSIGNED_BYTE, pixels.get()));

            // TODO move to jobs thread when it exists
#if WR_USE_FREEIMAGE
            auto bitmap = FreeImage_ConvertFromRawBits(pixels.get(), options.width, options.height, pitch, pitch * 8, 0, 0, 0);
            bool success = FreeImage_Save(FIF_PNG, bitmap, path, options.flags) != 0;
#else
            bool success = 0 != stbi_write_png(path, width, height, pitch, pixels.get(), width * pitch);
#endif

            if (options.callback) options.callback(success);
        });
    }
};