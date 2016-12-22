#pragma once

#include "Render.h"
#include "Light.h"

namespace Render {
    
    struct LitRenderer {
        Renderer opaque, alpha;
        Light::System lights;

        LitRenderer(const size_t opaqueG, const size_t alphaG, const size_t lightG) : opaque(opaqueG), alpha(alphaG), lightR(lightG) {
            opaque.setup = []() {
                GL_CHECK(glDisable(GL_BLEND));
                GL_CHECK(glFrontFace(GL_CCW));
                //GL_CHECK(glEnable(GL_DEPTH_TEST));
                //GL_CHECK(glDepthFunc(GL_LEQUAL));
            };
            lightR.setup = []() {
                //GL_CHECK(glDisable(GL_DEPTH_TEST));
                //GL_CHECK(glDepthFunc(GL_GREATER));

                // additive blending for accumulation
                GL_CHECK(glEnable(GL_BLEND));
                GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
                
                // clockwise winding for back-lit faces
                GL_CHECK(glFrontFace(GL_CW));
            };
            alpha.setup = [this]() {
                // undoing the light-specific settings
                GL_CHECK(glFrontFace(GL_CCW));

                GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            };

            // GBuffer layout:
            // 0: color
            // 1: position
            // 2: normals
            // 3: diffuse light accumulation
            // 4: specular light accumulation
            lightR.postProcess.output = GLframebuffer::createRenderTarget<GLubyte>();

            auto accumulate = make_shared<PostProcess>();
            accumulate->data.setShaders(PostProcess::make_program("Shaders/light/accumulate.glsl"));
            accumulate->data.setTextures(gBuffer[0], gBuffer[3], gBuffer[4]);
            accumulate->renderToTextures(lightR.postProcess.output);
            accumulate->data.shaders->program.use();
            accumulate->data.setSamplers(0, "color", "diffuse", "specular");

            lightR.postProcess.entry.chainsTo(accumulate);

            opaque.next = &lightR;
            //lightR.next = &alpha;
            lightGroup = 0;
        }

        void render() { 
            lights.update();
            lights.updateCamera(Camera::main);
            lights.defer(&lightR.objects, lightGroup);
            opaque.render(); 
        }

    private:
        Renderer lightR;
        uint32_t lightGroup;
    };

};