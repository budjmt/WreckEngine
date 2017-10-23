#pragma once

#include "Game.h"
#include "gl_structs.h"

struct PlanetCSphere {
    struct TerrainPlane {
        struct {
            vec3 center;
            float radius;
        } boundingSphere;
        vec3 boundingPoint; // for horizon culling
        Entity* entity;

        TerrainPlane(Entity* e, const Mesh* mesh, const vec3 dir, const float radius);
        int update(const vec3& pos, const Camera* cam, const float radius, const bool translucent);
    };
    std::vector<TerrainPlane> planes;

    PlanetCSphere(const float _radius, GLprogram _prog
                , State& state, Render::MaterialPass* renderer, const size_t group, std::function<void(DrawMesh&)> drawSetup);
    int update(const vec3& pos, const Camera* cam);

    void setActive(bool a) {
        active = a;
        for (auto& tp : planes)
            tp.entity->active = active;
    }

    bool active = true;
    bool translucent = false;
    const float radius;
    GLprogram prog;

private:
    shared<Entity> genPlane(const vec3 dir, Render::MaterialPass* renderer, const size_t group, std::function<void(DrawMesh&)>& drawSetup);
};

class TessellatorTest : public Game {
public:
    TessellatorTest();
    void update(double dt) override;
    void postUpdate() override;
    void draw() override;

    void setupPostProcess();

private:
    unique<PlanetCSphere> surface, atmosphere, water;
};