#pragma once

#include "Render.h"
#include "Light.h"

namespace Render {
    
    struct LitRenderer {
        Renderer opaque, alpha;
        Light::System lights;

        LitRenderer(const size_t gBufferSize) : opaque({ 1, 2, 3 }), alpha(gBufferSize), lightR({ 0, 4, 5 }) {
            opaque.setup = []() {
                GL_CHECK(glDisable(GL_BLEND));
                GL_CHECK(glFrontFace(GL_CCW));
                GL_CHECK(glDepthMask(GL_TRUE));
            };
            lightR.setup = []() {
                // prevents lights from culling each other
                GL_CHECK(glDepthMask(GL_FALSE));

                // additive blending for accumulation
                GL_CHECK(glEnable(GL_BLEND));
                GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
                
                // clockwise winding for back-lit faces
                GL_CHECK(glFrontFace(GL_CW));
            };
            alpha.setup = [this]() {
                // undoing the light-specific settings
                GL_CHECK(glFrontFace(GL_CCW));
                GL_CHECK(glDepthMask(GL_TRUE));

                GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            };

            // GBuffer layout:
            // 0: regular color
            // 1: position
            // 2: normals
            // 3: deferred color
            // 4: diffuse light accumulation
            // 5: specular light accumulation
            lightR.postProcess.output = gBuffer[0];

            auto accumulate = make_shared<PostProcess>();
            accumulate->data.setShaders(PostProcess::make_program("Shaders/light/accumulate.glsl"));
            accumulate->data.setTextures(gBuffer[3], gBuffer[4], gBuffer[5]);
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
            if(Camera::main) lights.updateCamera(Camera::main);
            lights.forward();
            lights.defer(&lightR.objects, lightGroup);
            opaque.render(); 
        }

    private:
        Renderer lightR;
        uint32_t lightGroup;
    };

};