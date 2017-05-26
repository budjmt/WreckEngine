#pragma once

#include "Render.h"
#include "Light.h"

namespace Render {
    
    struct LitRenderer {
        Renderer deferred, forward;
        Light::System lights;
        GLresource<vec3> ambientColor;
        bool lightingOn = true;

        explicit LitRenderer(size_t gBufferSize) : deferred({ 0, 1, 2, 3 }), forward(gBufferSize), lightR({ 4, 5 }) {
            deferred.setup = [this]() {
                GL_CHECK(glDisable(GL_BLEND));

                const auto camDir = Renderer::getCamData().forward;
                deferred.objects.preprocess([&](auto& drawCalls) {
                    std::sort(begin(drawCalls), end(drawCalls), [camDir](DrawCallInfo& drawA, DrawCallInfo& drawB) {
                        if (!(drawA.data.entity && drawB.data.entity)) return true;
                        float aProj = glm::dot(camDir, drawA.data.entity->transform.getComputed()->position()); 
                        float bProj = glm::dot(camDir, drawA.data.entity->transform.getComputed()->position());
                        return aProj < bProj; // closer objects come first
                    });
                });
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

                const auto camDir = Renderer::getCamData().forward;
                deferred.objects.preprocess([&](auto& drawCalls) {
                    std::sort(begin(drawCalls), end(drawCalls), [camDir](DrawCallInfo& drawA, DrawCallInfo& drawB) {
                        if (!(drawA.data.entity && drawB.data.entity)) return true;
                        float aProj = glm::dot(camDir, drawA.data.entity->transform.getComputed()->position());
                        float bProj = glm::dot(camDir, drawA.data.entity->transform.getComputed()->position());
                        return aProj > bProj; // further objects come first
                    });
                });
            };

            // GBuffer layout:
            // 0: position -> lit color
            // 1: normals
            // 2: deferred diffuse color
            // 3: deferred specular color
            // 4: diffuse  light accumulation
            // 5: specular light accumulation
            lightR.postProcess.output = gBuffer[0];

            auto accumulate = make_shared<PostProcess>();
            auto accProg = PostProcess::make_program("Shaders/light/accumulate.glsl");
            accProg.use();
            ambientColor = accProg.getUniform<vec3>("ambient");
            accumulate->data.setShaders(accProg, &ambientColor);
            accumulate->data.setTextures(gBuffer[2], gBuffer[3], gBuffer[4], gBuffer[5]);
            accumulate->renderToTextures(lightR.postProcess.output);
            //accumulate->data.shaders->program.use();
            //accumulate->data.setSamplers(0, "diffuseColor", "specularColor", "diffuseLight", "specularLight");

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