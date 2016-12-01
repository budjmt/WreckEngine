#pragma once

#include <functional>

#include "Material.h"
#include "PostProcess.h"

namespace Render {

    class MaterialRenderer {
    public:
        MaterialRenderer(const size_t gBufferSize);

        static void init(const size_t max_gBufferSize);

        static std::vector<GLtexture> gBuffer;
        static GLtexture depth, stencil;
        static GLtexture prevOutput;

        void scheduleDraw(const size_t group, const DrawCall d, const DrawCall::Params p);
        void scheduleDrawArrays  (const size_t group, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const uint32_t instances = 1);
        void scheduleDrawElements(const size_t group, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const GLenum element_t, const uint32_t instances = 1);
        
        void clearOutput() const;
        void render();

        // returns the index of the added group
        uint32_t addGroup(std::function<void()> setup, std::function<void()> cleanup) { renderGroups.push_back({ setup, cleanup }); return renderGroups.size() - 1;
    }
    private:

        struct Group {
            std::function<void()> setup, cleanup;
            std::vector<DrawCall> drawCalls;
            std::vector<DrawCall::Params> params;

            static GLbuffer paramBuffer;

            // RAII helper
            struct Helper {
                Helper(Group& g) : group(g) { group.setup(); }
                ~Helper() { group.cleanup(); }
                void draw();
            private:
                Group& group;
            };
        };

        std::vector<Group> renderGroups = { { []() {}, []() {} } };
        GLframebuffer frameBuffer;
    };

    class PostProcessRenderer {
    public:
        PostProcessRenderer();
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

    class FullRenderer {
    public:
        MaterialRenderer objects;
        PostProcessRenderer postProcess;
        FullRenderer* next = nullptr;

        FullRenderer(const size_t gBufferSize) : objects(gBufferSize) {}

        // need to funnel output through
        void render() {
            objects.render();
            postProcess.apply();
            if (next) {
                // if there's no post process chain, the output is from the mat renderer
                MaterialRenderer::prevOutput = postProcess.entry.endsChain() ? MaterialRenderer::gBuffer[0] : postProcess.output;
                next->render();
            }
            else postProcess.render();
        }
    };
}