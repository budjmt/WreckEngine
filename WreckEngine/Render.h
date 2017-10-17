#pragma once

#include <functional>

#include "Material.h"
#include "PostProcess.h"
#include "Event.h"

class Camera;

namespace Render {

    struct CameraData {
        mat4 viewProjection;
        vec3 position, forward;
        Camera* cam = nullptr;

        CameraData() = default;
        CameraData(Camera* c);
        CameraData(CameraData&&) = default;
        CameraData& operator=(CameraData&&) = default;
    };

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
        MaterialPass(size_t gBufferSize);
        explicit MaterialPass(const std::vector<GLuint>& targets);
        MaterialPass(const std::vector<GLtexture>& colorTargets, const GLtexture& depthstencilTarget);

        void scheduleDraw(size_t group, DrawCall d, DrawCall::Params p);
        void scheduleDrawArrays  (size_t group, const Entity* entity, const GLVAO* vao, const Info* mat, GLenum tesselPrim, uint32_t count, uint32_t instances = 1);
        void scheduleDrawElements(size_t group, const Entity* entity, const GLVAO* vao, const Info* mat, GLenum tesselPrim, uint32_t count, GLenum element_t, uint32_t instances = 1);
        
        void render();

        // returns the index of the added group
        uint32_t addGroup(std::function<void()> setup, std::function<void()> cleanup) { 
            renderGroups.push_back(Group(setup, cleanup)); 
            return renderGroups.size() - 1;
        }


        void preprocess(std::function<void(std::vector<DrawCallInfo>&)> func) {
            for (auto& group : renderGroups)
                func(group.drawCalls);
        }
    private:

        struct Group {
            std::function<void()> setup, cleanup;
            GLbuffer paramBuffer;

            std::vector<DrawCallInfo> drawCalls;
            std::vector<DrawCall::Params> rawDrawParams;

            Group(std::function<void()> s, std::function<void()> c) : setup(s), cleanup(c) {
                paramBuffer.create(GL_DRAW_INDIRECT_BUFFER, GL_STREAM_DRAW);
                paramBuffer.bind();
                paramBuffer.data(0, nullptr);
            };

            // RAII helper
            struct Helper {
                explicit Helper(Group& g) : group(g) { group.setup(); }
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
        Camera* renderCam = nullptr;
        static const CameraData& getCamData();

        MaterialPass objects;
        PostProcessChain postProcess;
        Renderer* next = nullptr;
        std::function<void()> setup = [](){};

        static void init(size_t max_gBufferSize);

        Renderer(size_t gBufferSize) : objects(gBufferSize) {}
        explicit Renderer(const std::vector<GLuint>& targets) : objects(targets) {}

        void render();
        void renderChildren();
    };
}

template<>
class GLresource<GLcamera::matrix> : public GLres {
public:
    using uniform_t = mat4;

    GLresource() = default;
    GLresource(GLuniform<mat4> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : GLresource(p.getUniform<mat4>(name)) {}

    void update() const override { location.update(Render::Renderer::getCamData().viewProjection); }

private:
    GLuniform<mat4> location;
};

template<>
class GLresource<GLcamera::position> : public GLres {
public:
    using uniform_t = vec3;

    GLresource() = default;
    GLresource(GLuniform<vec3> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : GLresource(p.getUniform<vec3>(name)) {}

    void update() const override { location.update(Render::Renderer::getCamData().position); }

private:
    GLuniform<vec3> location;
};

template<>
class GLresource<GLcamera::direction> : public GLres {
public:
    using uniform_t = vec3;

    GLresource() = default;
    GLresource(GLuniform<vec3> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : GLresource(p.getUniform<vec3>(name)) {}

    void update() const override { location.update(Render::Renderer::getCamData().forward); }

private:
    GLuniform<vec3> location;
};
