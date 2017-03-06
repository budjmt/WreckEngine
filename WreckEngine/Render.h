#pragma once

#include <functional>

#include "Material.h"
#include "PostProcess.h"
#include "Event.h"

namespace Render {

    struct Target {
        GLtexture texture;
        struct {
            GLenum to, from, type;
        } formatInfo;

        // creates a render target. If this is for depth and/or stencil, [from] must be GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, or GL_DEPTH_STENCIL
        template<typename value_t>
        static GLtexture create(GLenum to = GL_RGBA, GLenum from = GL_RGBA) {
            Target rt;
            rt.texture.create();
            rt.texture.bind();
            rt.texture.param(GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            rt.texture.set2D<value_t>(nullptr, Window::frameWidth, Window::frameHeight, rt.formatInfo.from = from, rt.formatInfo.to = to);
            rt.formatInfo.type = GLtype<value_t>();
            
            targets.push_back(rt);
            return rt.texture;
        }

    private:
        Target() = default;
        static void resizeTargets(Event::Handler::param_t e);

        static Event::Handler resizeHandler;
        static std::vector<Target> targets;
    };

    extern std::vector<GLtexture> gBuffer;
    extern GLtexture depthstencil;

    extern GLbuffer fs_triangle;

    class MaterialPass {
    public:
        MaterialPass(const size_t gBufferSize);
        MaterialPass(const std::vector<GLuint>& targets);

        void scheduleDraw(const size_t group, const DrawCall d, const DrawCall::Params p);
        void scheduleDrawArrays  (const size_t group, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const uint32_t instances = 1);
        void scheduleDrawElements(const size_t group, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const GLenum element_t, const uint32_t instances = 1);
        
        void render();

        // returns the index of the added group
        uint32_t addGroup(std::function<void()> setup, std::function<void()> cleanup) { 
            renderGroups.push_back(Group(setup, cleanup)); 
            return renderGroups.size() - 1;
        }
    private:

        struct Group {
            std::function<void()> setup, cleanup;
            GLbuffer paramBuffer;

            std::vector<DrawCall> drawCalls;
            std::vector<DrawCall::Params> params;

            Group(std::function<void()> s, std::function<void()> c) : setup(s), cleanup(c) {
                paramBuffer.create(GL_DRAW_INDIRECT_BUFFER, GL_STREAM_DRAW);
                paramBuffer.bind();
                paramBuffer.data(0, nullptr);
            };

            // RAII helper
            struct Helper {
                Helper(Group& g) : group(g) { group.setup(); }
                ~Helper() { group.cleanup(); }
                void draw();
            private:
                Group& group;
            };
        };

        std::vector<Group> renderGroups = { Group([]() {}, []() {}) };
        GLframebuffer frameBuffer;

        void prepareFrameBuffer();
    };

    class PostProcessChain {
    public:
        PostProcess entry; // chain post processes to this
        GLtexture output;  // bind a color buffer of the final post process to this

        static void init();

        void apply();
        void finish(PostProcess* curr);
        void render() const;

    private:
        static GLVAO triangle;
        static GLprogram finalize;
    };

    class Renderer {
    public:
        MaterialPass objects;
        PostProcessChain postProcess;
        Renderer* next = nullptr;
        std::function<void()> setup = [](){};

        static void init(const size_t max_gBufferSize);

        Renderer(const size_t gBufferSize) : objects(gBufferSize) {}
        Renderer(const std::vector<GLuint>& targets) : objects(targets) {}

        void render();
        void renderChildren();
    };
}