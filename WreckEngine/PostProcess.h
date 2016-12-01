#pragma once

#include "Material.h"

namespace Render {

    class Composite;
    class PostProcessRenderer;

    class PostProcess {
    public:
        Info data;
        GLframebuffer fbo;
        PostProcessRenderer* renderer;

        virtual ~PostProcess() = default;

        static void init();

        static GLprogram make_program(const GLshader& fragment) {
            assert(fragment.type == GL_FRAGMENT_SHADER);
            GLprogram p;
            p.vertex = defaultVertex;
            p.fragment = fragment;
            p.create();
            p.link();
            return p;
        }

        virtual void reset() { for (auto p : chain) p->reset(); }
        virtual void apply(PostProcess* prev);

        // might want parameter pack version
        void chainsTo(PostProcess* p);
        void chainsTo(Composite* p);

        // Given some set of output textures T ([io])
        // a *continuation* consists of two PP, A ([this]) then B ([p]) where:
        //   - A's frame buffer outputs to all textures in T
        //   - B's texture binding input is a super set of T
        // This convenience function assumes:
        //   - Any additional inputs in B will come before or after the frame buffer outputs, i.e. not interleaved
        //   - B takes ALL of the outputs provided; any outputs that need to be added/removed must be bound/unbound elsewhere
        //   - All textures provided are color buffers for A's frame buffer
        template<typename... Textures>
        void continuesWith(PostProcess* p, Textures... io) {
            GLtexture textures[sizeof...(io)] = { io... };

            for (int i = 0; i < sizeof...(io); ++i)
                fbo.attachTexture(tex, GLframebuffer::Attachment::Color);

            const auto& inputs = p->textures->bindings;
            inputs.insert(inputs.end(), textures.begin(), textures.end());

            this->chainsTo(p);
        }

        bool endsChain() const { return chain.empty(); }

    protected:
        std::vector<PostProcess*> chain;
        friend class PostProcessRenderer;
        
        static GLshader defaultVertex;
    };

    class Composite : public PostProcess {
    public:
        void reset() override { PostProcess::reset(); numReady = 0; }

        bool ready() const { return numReady == dependencies.size(); }
        void apply(PostProcess* prev) override {
            ++numReady; // don't need validity check; only dependencies should show up as prev ptrs
            if (ready())
                PostProcess::apply(this);
        }

    private:
        std::vector<PostProcess*> dependencies;
        size_t numReady = 0;
        friend class PostProcess;
    };

};