#pragma once

#include "alias_vector.h"

#include "gl_structs.h"

class Entity;

namespace Render {

    class Info {
        alias_vector<uint8_t> uniqueVars;
    public:
        struct ShaderBinding {
            struct Variable {
                uint32_t offset;
                bool isShared;

                template<typename T> struct is_shared_t            { static constexpr bool value = false; };
                template<> struct is_shared_t<GLresolution>        { static constexpr bool value = true; };
                template<> struct is_shared_t<GLtime>              { static constexpr bool value = true; };
                template<> struct is_shared_t<GLcamera::matrix>    { static constexpr bool value = true; };
                template<> struct is_shared_t<GLcamera::position>  { static constexpr bool value = true; };
                template<> struct is_shared_t<GLcamera::direction> { static constexpr bool value = true; };
            };
            GLprogram program;

        private:
            std::map<std::string, Variable> varLookup;
            alias_vector<uint8_t> sharedVars;
            friend class Info;
        };
    
        struct TextureBinding {
            std::vector<GLtexture> bindings; // push textures in the order they should be bound

            void bind() const;

            template<typename... Names>
            void setSamplers(GLprogram& prog, int baseIndex, Names... _names) {
                assert(bindings.size() >= sizeof...(_names));
                const char* names[] = { std::forward<Names>(_names)... };
                for (int i = 0; i < sizeof...(_names); ++i)
                    prog.setOnce<GLsampler>(names[i], baseIndex + i);
            }
        };

        template<typename T>
        using res_proxy = proxy_ptr<alias_vector<GLresource<T>>>;

        shared<ShaderBinding>  shaders;
        shared<TextureBinding> textures;

        void setShaders(GLprogram prog) {
            shaders = make_shared<ShaderBinding>();
            shaders->program = prog;
        }

        template<typename T>
        res_proxy<T> addResource(const std::string& varName, bool shared = ShaderBinding::Variable::is_shared_t<T>::value) {
            return addResource<T>(varName, shaders->program.getUniform<typename GLresource<T>::uniform_t>(varName.c_str()), shared);
        }

        template<typename T>
        res_proxy<T> addResource(const std::string& varName, GLuniform<typename GLresource<T>::uniform_t> location, bool shared = ShaderBinding::Variable::is_shared_t<T>::value) {
            if (shaders->varLookup.count(varName))
                return getResource<T>(varName);

            using resource_t = GLresource<T>;
            auto& resources = shared ? shaders->sharedVars : uniqueVars;
            auto bytesUsed = resources.size();

            shaders->varLookup[varName] = { bytesUsed, shared };
            resources.resize(bytesUsed + sizeof(resource_t));

            resource_t resource(location);
            std::memcpy(resources.data() + bytesUsed, &resource, sizeof(resource_t));
            return reinterpret_cast<res_proxy<T>&>(make_proxy(resources, bytesUsed));
        }

        template<typename T>
        res_proxy<T> getResource(const std::string& varName) {
            using resource_t = GLresource<T>;
            auto& lookup = shaders->varLookup;
            assert(lookup.count(varName));
            auto varData = lookup.at(varName);

            auto& resources = varData.isShared ? shaders->sharedVars : uniqueVars;
            assert(dynamic_cast<resource_t*>((GLres*)(resources.data() + varData.offset)) != nullptr); // asserts the correct type based on the vptr
            return reinterpret_cast<res_proxy<T>&>(make_proxy(resources, varData.offset));
        }

        template<typename... GLTextures>
        void setTextures(GLTextures... textures) {
            textures = make_shared<TextureBinding>();
            textures->bindings = { textures... };
        }

        void addTexture(const GLtexture& tex) { textures->bindings.push_back(tex); }
        void removeTexture(GLtexture& resource) {
            auto& bindings = textures->bindings;
            bindings.erase(std::find(begin(bindings), end(bindings), resource));
        }

        template<typename... Names>
        void setSamplers(int baseIndex, Names&&... _names) {
            textures->setSamplers(shaders->program, baseIndex, std::forward<Names>(_names)...);
        }

        void apply() const {
            assert(shaders && textures); // shaders and textures must be initialized
            shaders->program.use();
            auto& sharedVars = shaders->sharedVars;
            for (auto& [name, data] : shaders->varLookup) {
                auto& resources = data.isShared ? sharedVars : uniqueVars;
                reinterpret_cast<const GLres&>(resources[data.offset]).update();
            }
            textures->bind();
        }
    };

    struct DrawCall {
        const Entity* entity;
        const Info* material;
        const GLVAO* vao;

        enum Type { Arrays, Elements } call;
        GLenum tesselPrim, element_t;

        struct Params {
            uint32_t count; // number of vertices to be rendered
            uint32_t instances; // should be set to at least 1
            union {
                // baseInstanceArr and baseInstanceElem represent the same field, 
                // but they're at different locations depending on the [indirect] draw call
                // also don't use it because it's 4.2+
                struct { uint32_t first, baseInstanceArr; }; // For DrawArrays
                struct { uint32_t firstIndex, baseVertex, baseInstanceElem; }; // For DrawElements
            };
        };

    private:
        void render(const void* offset) const;
        friend class MaterialPass;
    };

    struct DrawCallInfo {
        DrawCall data;
        DrawCall::Params params;
    };

};