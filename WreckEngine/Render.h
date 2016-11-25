#pragma once

#include "Material.h"
#include "PostProcess.h"

namespace Render {

    class MaterialRenderer {
    public:
        MaterialRenderer(const size_t gBufferSize);

        static void init(const size_t max_gBufferSize);

        static std::vector<GLtexture> gBuffer;
        static GLtexture depthstencil;

        void scheduleDraw(const DrawCall d);
        void scheduleDrawArrays  (const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const uint32_t instances = 1);
        void scheduleDrawElements(const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const GLenum element_t, const uint32_t instances = 1);
        void render();
    private:
        std::vector<DrawCall> drawCalls;
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

    private:
        GLVAO triangle;
        static GLprogram finalize;
    };

    class FullRenderer {
        MaterialRenderer objects;
        PostProcessRenderer postProcess;

        FullRenderer(const size_t gBufferSize) : objects(gBufferSize) {}

        void render() {
            objects.render();
            postProcess.apply();
        }
    };
}