#pragma once

#include "Material.h"

namespace Render {

    class Composite;
    class PostProcessChain;

    class PostProcess {
    public:
        Info data;
        GLframebuffer fbo;
        PostProcessChain* renderer;

        PostProcess();
        virtual ~PostProcess() = default;

        static void init();

        static GLprogram make_program(const char* shaderFile);
        static GLprogram make_program(const GLshader& fragment);

        template<typename... GLTextures>
        void renderToTextures(GLTextures... tex) {
            GLtexture textures[] = { tex... };
            fbo.bindPartial();
            for (size_t i = 0; i < sizeof...(tex); ++i)
                fbo.attachTexture(textures[i], GLframebuffer::Attachment::Color);
            fbo.unbind();
        }

        virtual void refresh() { for (auto& p : chain) p->refresh(); }
        virtual void apply();

        // might want parameter pack version
        PostProcess* chainsTo(shared<PostProcess> p);
        PostProcess* chainsTo(shared<Composite> p);

        // returns the end of the cycle (a reference to a copy of b)
        // this pointer MUST be used to continue the chain, or it will be malformed
        PostProcess* cyclesWith(size_t numCycles, shared<PostProcess> p);

        // Chains from the caller to p and binds a set of textures to the caller's frame buffer as well as p's texture bindings
        // Assumes:
        //   - Any additional inputs in B will come before or after the frame buffer outputs, i.e. not interleaved
        //   - B takes ALL of the outputs provided; any outputs that need to be added/removed must be bound/unbound elsewhere
        //   - All textures provided are color buffers for A's frame buffer
        template<typename... Textures>
        PostProcess* continuesWith(shared<PostProcess> p, Textures... io) {
            GLtexture textures[sizeof...(io)] = { io... };

            for (int i = 0; i < sizeof...(io); ++i)
                fbo.attachTexture(textures[i], GLframebuffer::Attachment::Color);

            auto& data = p->data;
            if (data.textures) {
                auto& inputs = data.textures->bindings;
                inputs.insert(inputs.end(), textures.begin(), textures.end());
            }
            else {
                data.setTextures(std::forward<Textures>(io)...);
            }

            return this->chainsTo(p);
        }

        bool endsChain() const { return chain.empty(); }

    protected:
        std::vector<shared<PostProcess>> chain;
        friend class PostProcessChain;
        
        static GLshader defaultVertex;
    };

    class Composite : public PostProcess {
    public:
        void refresh() override { PostProcess::refresh(); numReady = 0; }

        bool ready() const { return numReady == dependencies.size(); }
        void apply() override {
            ++numReady; // don't need validity check; only dependencies should show up as prev ptrs
            if (ready())
                PostProcess::apply();
        }

    private:
        std::vector<PostProcess*> dependencies;
        size_t numReady = 0;
        friend class PostProcess;
    };

};