#pragma once

#include "Render.h"
#include "Light.h"

namespace Render {
    
    struct LitRenderer {
        Renderer deferred, forward;
        Light::System lights;
        Info::res_proxy<vec3> ambientColor;
        bool lightingOn = true;

        explicit LitRenderer(size_t gBufferSize) : deferred({ 0, 1, 2, 3 }), forward(gBufferSize), lightR({ 4, 5 }) {
            deferred.setup = [this]() {
                GLstate<GL_BLEND, GL_ENABLE_BIT>{ false }.apply();

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
                GLstate<GL_DEPTH_TEST, GL_DEPTH_WRITEMASK>{ false }.apply();
                GLstate<GL_DEPTH_TEST, GL_DEPTH_FUNC>{ GL_GREATER }.apply();

                // additive blending for accumulation
                GLstate<GL_BLEND, GL_ENABLE_BIT>{ true }.apply();
                GLstate<GL_BLEND, GL_BLEND_FUNC>{ GLstate_comp{ GL_ONE }, GLstate_comp{ GL_ONE } }.apply();
                
                // clockwise winding for back-lit faces
                GLstate<GL_CULL_FACE, GL_FRONT_FACE>{ GL_CW }.apply();
            };
            lightGroup = lightR.objects.addGroup([]() {
                GLstate<GL_DEPTH_TEST, GL_ENABLE_BIT>{ false }.apply();
            }, []() {
                GLstate<GL_DEPTH_TEST, GL_ENABLE_BIT>{ true }.apply();
                GLstate<GL_BLEND, GL_ENABLE_BIT>{ false }.apply();
            });

            forward.setup = [this]() {
                // undoing the light-specific settings
                GLstate<GL_CULL_FACE, GL_FRONT_FACE>{ GL_CCW }.apply();
                GLstate<GL_DEPTH_TEST, GL_DEPTH_WRITEMASK>{ true }.apply();
                GLstate<GL_DEPTH_TEST, GL_DEPTH_FUNC>{ GL_LEQUAL }.apply();

                // re-enable blend after the accumulation PP and change blend func
                GLstate<GL_BLEND, GL_ENABLE_BIT>{ true }.apply();
                GLstate<GL_BLEND, GL_BLEND_FUNC>{ GLstate_comp{ GL_SRC_ALPHA }, GLstate_comp{ GL_ONE_MINUS_SRC_ALPHA } }.apply();

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
            deferred.clearColorIndex = 2;
            lightR.postProcess.output = gBuffer[0];

            auto accumulate = make_shared<PostProcess>();
            auto accProg = PostProcess::make_program("Shaders/light/accumulate.glsl");
            accProg.use();
            accumulate->data.setTextures(gBuffer[2], gBuffer[3], gBuffer[4], gBuffer[5]);
            accumulate->data.setShaders(accProg);
            ambientColor = accumulate->data.addResource<vec3>("ambient");
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