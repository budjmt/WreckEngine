#pragma once

#include "Render.h"
#include "Light.h"

namespace Render {
    
    struct LitRenderer {
        Renderer deferred, forward;
        Light::System lights;
        bool lightingOn = true;

        LitRenderer(const size_t gBufferSize) : deferred({ 0, 1, 2 }), forward(gBufferSize), lightR({ 3, 4 }) {
            deferred.setup = []() {
                GL_CHECK(glDisable(GL_BLEND));
            };
            
            lightR.setup = []() {
                // prevents lights from culling each other
                GL_CHECK(glDepthMask(GL_FALSE));
                GL_CHECK(glDepthFunc(GL_GREATER));

                // additive blending for accumulation
                GL_CHECK(glEnable(GL_BLEND));
                GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
                
                // clockwise winding for back-lit faces
                GL_CHECK(glFrontFace(GL_CW));
            };
            lightGroup = lightR.objects.addGroup([]() {
                GL_CHECK(glDisable(GL_DEPTH_TEST));
            }, []() {
                GL_CHECK(glEnable(GL_DEPTH_TEST));
                GL_CHECK(glDisable(GL_BLEND));
            });

            forward.setup = [this]() {
                // undoing the light-specific settings
                GL_CHECK(glFrontFace(GL_CCW));
                GL_CHECK(glDepthMask(GL_TRUE));
                GL_CHECK(glDepthFunc(GL_LEQUAL));

                // re-enable blend after the accumulation PP and change blend func
                GL_CHECK(glEnable(GL_BLEND));
                GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            };

            // GBuffer layout:
            // 0: position -> lit color
            // 1: normals
            // 2: deferred color
            // 3: diffuse  light accumulation
            // 4: specular light accumulation
            lightR.postProcess.output = gBuffer[0];

            auto accumulate = make_shared<PostProcess>();
            accumulate->data.setShaders(PostProcess::make_program("Shaders/light/accumulate.glsl"));
            accumulate->data.setTextures(gBuffer[2], gBuffer[3], gBuffer[4]);
            accumulate->renderToTextures(lightR.postProcess.output);
            accumulate->data.shaders->program.use();
            accumulate->data.setSamplers(0, "color", "diffuse", "specular");

            lightR.postProcess.entry.chainsTo(accumulate);

            deferred.next = &lightR;
            lightR.next = &forward;
        }

        void render() { 
            lights.update();
            if (lightingOn && Camera::main) {
                lights.updateCamera(Camera::main);
                lights.forward();
                lights.defer(&lightR.objects, lightGroup);
            }
            deferred.render(); 
        }

    private:
        Renderer lightR;
        uint32_t lightGroup;
    };

};