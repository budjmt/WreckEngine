#pragma once

#include <experimental/filesystem>
#include "File.h"

namespace HotSwap {

    struct SwapResource {
        virtual void update() = 0;
    };

    extern std::vector<shared<SwapResource>> resources;
    void main();

    namespace fs = std::experimental::filesystem;

    template<File::Extension E>
    class Resource {
    public:
        Resource() = default;
        Resource(const char* path, const uint32_t _options = 0) : rawPath(path), filePath(path), options(_options), resource(File::load<E>(path, _options)) {}
        
        File::resource_t<E> get() { return resource; }

        // sets the resource without attempting to load the file path specified
        void set(File::resource_t<E> res, const char* path, const uint32_t _options = 0) {
            filePath = rawPath = path;
            options = _options;
            resource = res;
        }

        bool checkForUpdate() {
            if (!File::isValid<E>(resource)) return false;
            auto write = fs::last_write_time(filePath);
            if (write > lastModified) {
                lastModified = write;
                return true;
            }
            return false;
        }

        bool getUpdate(File::resource_t<E>& res) {
            auto swapRes = File::load<E>(rawPath, options);
            if (File::isValid<E>(swapRes)) {
                res = std::move(swapRes);
                return true;
            }
            return false;
        }

        bool tryUpdate(File::resource_t<E>& res) {
            return checkForUpdate() && getUpdate(res);
        }
    private:
        const char* rawPath;
        fs::path filePath;
        fs::file_time_type lastModified = Time::system_now();
        File::resource_t<E> resource;
        uint32_t options;
    };

    // For a shader to be hot-swappable, it must specify all uniform locations + initial values and any immutable bindings for samplers
    // Any additions/removals of uniforms require, at minimum, a re-run of the program; sampler binding changes do not
    class Shader : public SwapResource {
    public:
        static shared<Shader> create();

        Resource<File::Extension::GLSL> vertex, tessControl, tessEval, geometry, fragment;
        inline GLprogram getProgram() const { return program; }

        void setupProgram();
        void update() override;
    private:
        Shader() = default;
        GLprogram program;
    };
};