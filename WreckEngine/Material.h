#pragma once

#include <vector>

#include "gl_structs.h"

namespace Render {

    struct Info {

        struct ShaderBinding {
            std::vector<GLres*> resources;
            GLprogram program;

            void update() const;
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

        shared<ShaderBinding>  shaders;
        shared<TextureBinding> textures;

        template<typename... GLResourcePtrs>
        void setShaders(GLprogram prog, GLResourcePtrs... resources) {
            shaders.reset(new ShaderBinding);
            shaders->program = prog;
            shaders->resources = { resources... };
        }

        void addResource(GLres* resource) { shaders->resources.push_back(resource); }
        void removeResource(GLres* resource) {
            auto& resources = shaders->resources;
            resources.erase(std::find(resources.begin(), resources.end(), resource));
        }

        template<typename... GLTextures>
        void setTextures(GLTextures... textures) {
            textures.reset(new TextureBinding);
            textures->bindings = { textures... };
        }

        void addTexture(const GLtexture& tex) { textures->bindings.push_back(tex); }
        void removeTexture(GLtexture& resource) {
            auto& bindings = textures->bindings;
            bindings.erase(std::find(bindings.begin(), bindings.end(), resource));
        }

        template<typename... Names>
        void setSamplers(int baseIndex, Names&&... _names) {
            textures->setSamplers(shaders->program, baseIndex, std::forward<Names>(_names)...);
        }

        void apply() const {
            assert(shaders && textures); // shaders and textures must be initialized
            shaders->update();
            textures->bind();
        }
    };

    struct DrawCall {
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

};