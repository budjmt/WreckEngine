#pragma once

#include "gl_structs.h"

#include "Render.h"

#define PADF1 float _pad1_
#define PADF2 PADF1, _pad2_
#define PADF3 PADF2, _pad3_

#include "Camera.h"

namespace Light {

    // defines a general light interface
    template<class T>
    struct Base {
        // uint32_t priority; // conceptual, cull lower priority lights on lower-end platforms

        static GLuint setupAttrs(GLattrarr& attrs, GLuint baseIndex = 0) {
            return T::setupAttrsImpl(attrs, baseIndex);
        }

        static void bindGeometry() {
            return T::bindGeometryImpl();
        }

        static void draw(Render::MaterialPass* r, const uint32_t group, const GLVAO* vao, const Render::Info* info, size_t instances) {
            T::drawImpl(r, group, vao, info, instances);
        }
    };

    // expands uniformly from a point
    struct Point : public Base<Point> {
        vec3 position;
        int isOff; // it's a bool, but an int for shader reasons
        vec3 color; // lights can't have opacity
        uint32_t tag;
        vec2 falloff; // light radius; inner = x, outer = y
        PADF2;

        static size_t count;

        static GLuint setupAttrsImpl(GLattrarr& attrs, GLuint baseIndex) {
            attrs.add<vec3>(1, 1);
            attrs.add<int>(1, 1);
            attrs.add<vec3>(1, 1);
            attrs.add<GLuint>(1, 1);
            attrs.add<vec2>(1, 1);
            return attrs.apply(baseIndex, sizeof(float) * 2);
        }

        static void bindGeometryImpl();

        static void drawImpl(Render::MaterialPass* r, const uint32_t group, const GLVAO* vao, const Render::Info* info, size_t instances) {
            r->scheduleDrawElements(group, vao, info, GL_TRIANGLES, count, GLtype<GLuint>(), instances);
        }

        bool isTransformed(const Point& update) const {
            return position != update.position;
        }

        mat4 getTransform() const {
            return glm::translate(position) * glm::scale(vec3(falloff.y));
        }
    };

    // uniformly lights all objects from a direction
    struct Directional : public Base<Directional> {
        vec3 direction;
        int isOff;
        vec3 color;
        uint32_t tag;

        static GLuint setupAttrsImpl(GLattrarr& attrs, GLuint baseIndex) {
            attrs.add<vec3>(1, 1);
            attrs.add<int>(1, 1);
            attrs.add<vec3>(1, 1);
            attrs.add<GLuint>(1, 1);
            return attrs.apply(baseIndex);
        }

        static void bindGeometryImpl();

        static void drawImpl(Render::MaterialPass* r, const uint32_t group, const GLVAO* vao, const Render::Info* info, size_t instances) {
            r->scheduleDrawArrays(group, vao, info, GL_TRIANGLES, 3, instances);
        }
    };

    // expands conically from a point and direction
    struct Spotlight : public Base<Spotlight> {
        vec3 position;
        int isOff;
        vec2 falloffRad;
        vec2 falloffLen;
        vec3 direction;
        uint32_t tag;
        vec3 color;
        PADF1;

        static size_t count;

        static GLuint setupAttrsImpl(GLattrarr& attrs, GLuint baseIndex) {
            attrs.add<vec3>(1, 1);
            attrs.add<int>(1, 1);
            attrs.add<vec2>(1, 1);
            attrs.add<vec2>(1, 1);
            attrs.add<vec3>(1, 1);
            attrs.add<GLuint>(1, 1);
            attrs.add<vec3>(1, 1);
            return attrs.apply(baseIndex, sizeof(float));
        }

        static void bindGeometryImpl();

        static void drawImpl(Render::MaterialPass* r, const uint32_t group, const GLVAO* vao, const Render::Info* info, size_t instances) {
            r->scheduleDrawElements(group, vao, info, GL_TRIANGLES, count, GLtype<GLuint>(), instances);
        }

        bool isTransformed(const Spotlight& update) const {
            return direction != update.direction || position != update.position;
        }

        mat4 getTransform() const {
            return glm::translate(position) * rotateBetween(vec3(0, 1, 0), direction) * glm::scale(vec3(falloffRad.y, falloffLen.y, falloffRad.y));
        }
    };

    // class Area : public Base
    // class Volume : public Base

    enum UpdateFreq : uint32_t { NEVER = 0, RARELY, SOMETIMES, OFTEN };

    template<typename T> class Manager;

    template<typename T>
    class Group {
    public:

        GLbuffer subBuffer, subTransformBuffer;
        size_t bufferIndex = 0;

        // sets the buffer used and updates the local internal data; returns the number of bytes used
        size_t setBuffers(GLbuffer buffer, GLbuffer transform) {
            subBuffer = buffer;
            subBuffer.set(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
            subTransformBuffer = transform;
            subTransformBuffer.size = sizeof(mat4) * lights.size();
            return subBuffer.size = sizeof(T) * lights.size();
        }

        // updates the group's sub-buffer contents as required; assumes the buffer is bound
        void update() {
            if (neededUpdates > OFTEN) return;
            auto offset = freqData[neededUpdates].offset;
            subBuffer.subdata(&lights[offset], (lights.size() - offset) * sizeof(T), (bufferIndex + offset) * sizeof(T));
            neededUpdates = (UpdateFreq) (OFTEN + 1);
        }

        void addLight(T light, const UpdateFreq freq) {
            auto data = freqData[freq];
            size_t offset = data.offset + data.size;
            
            lights.insert(lights.begin() + offset, light);
            ++freqData[freq].size;

            switch (freq) {
            case RARELY:
                ++freqData[RARELY].offset;
            case SOMETIMES:
                ++freqData[SOMETIMES].offset;
            case OFTEN:
                ++freqData[OFTEN].offset;
            }
        }

        // get a light index after all lights have been inserted into the group
        int getLightIndexByTag(const uint32_t tag, const UpdateFreq frequency) const { 
            auto data = freqData[frequency];
            for (auto i = data.offset, end = data.offset + data.size; i < end; ++i) {
                if (lights[i].tag == tag)
                    return i;
            }
            return -1;
        }

        T getLight(const uint32_t index) const { return lights[index]; }
        
        void updateLight(const uint32_t index, const UpdateFreq frequency, const T& light) { 
            assert(frequency != NEVER);
            assert(index >= freqData[frequency].offset && index < freqData[frequency].offset + freqData[frequency].size);
            
            if (lights[index].isTransformed(light)) {
                auto transform = light.getTransform();
                subTransformBuffer.subdata(&transform, sizeof(mat4), (bufferIndex + index) * sizeof(mat4));
            }
            
            lights[index] = light; 
            if (frequency < neededUpdates) 
                neededUpdates = frequency;
        }

    private:
        std::vector<T> lights;
        struct { size_t size, offset; } freqData[4]{};
        UpdateFreq neededUpdates = (UpdateFreq) (OFTEN + 1);

        void setupTransformBuffer() const {
            std::vector<mat4> mats;
            mats.reserve(lights.size());
            for (auto& light : lights)
                mats.push_back(light.getTransform());
            subTransformBuffer.subdata(&mats[0], sizeof(mat4) * mats.size(), bufferIndex * sizeof(mat4));
        }

        friend class Manager<T>;
    };

    template<> void Group<Directional>::updateLight(const uint32_t index, const UpdateFreq frequency, const Directional& light) {
        assert(frequency != NEVER);
        assert(index >= freqData[frequency].offset && index < freqData[frequency].offset + freqData[frequency].size);

        lights[index] = light;
        if (frequency < neededUpdates)
            neededUpdates = frequency;
    }

    template<> void Group<Directional>::setupTransformBuffer() const {}

    template<typename T>
    class Manager {
    public:
        Manager() { 
            deferredVao.create();
            forwardBlock.create();
            transformBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW); 
        }

        GLuniformblock<T> forwardBlock;
        Render::Info renderInfo;
        GLuniform<mat4> camMat;
        GLuniform<vec3> camPos;

        // equivalent to using resetGroups, addGroup (for each lightGroup), and then finishGroups
        void setGroups(std::vector<Group<T>> lightGroups) {
            groups = lightGroups;
            forwardBlock.block.size = 0;
            for (auto& group : groups) {
                addGroupImpl(group);
            }
            finishGroups();
        }

        // resets the internal group set; call before making changes
        void resetGroups() {
            groups.clear();
            forwardBlock.block.size = 0;
        }

        // adds a group to the set
        void addGroup(Group<T> lightGroup) {
            addGroupImpl(lightGroup);
            groups.push_back(lightGroup);
        }

        // finishes the group set by allocating the VBO and populating it with the groups' data; call after adding all groups
        void finishGroups() {
            forwardBlock.block.bind();
            transformBuffer.bind();
            forwardBlock.block.data(forwardBlock.block.size, nullptr);
            transformBuffer.data(forwardBlock.block.size / sizeof(T) * sizeof(mat4), nullptr);
            for (auto& group : groups) {
                group.update();
                group.setupTransformBuffer();
            }
        }

        void update() {
            for (auto& group : groups)
                group.update();
        }

        void updateCamera(Camera* camera) const {
            renderInfo.shaders->program.use();
            camMat.update(camera->getCamMat());
            camPos.update(camera->transform.getComputed()->position);
        }

        void defer(Render::MaterialPass* renderer, const size_t renderGroup) const {
            Base<T>::draw(renderer, renderGroup, &deferredVao, &renderInfo, forwardBlock.block.size / sizeof(T));
        }

    private:
        std::vector<Group<T>> groups;
        
        GLVAO deferredVao;
        GLbuffer transformBuffer;

        void addGroupImpl(Group<T>& g) {
            g.neededUpdates = UpdateFreq::NEVER;
            g.bufferIndex = forwardBlock.block.size / sizeof(T);
            forwardBlock.block.size += g.setBuffers(forwardBlock.block, transformBuffer);
        }

        void setupDeferred(GLattrarr& attrs) const {
            deferredVao.bind();

            Base<T>::bindGeometry(); // vertices + elements
            attrs.add<vec3>(1);
            auto index = attrs.apply();

            transformBuffer.bind();
            attrs.add<mat4>(1, 1);
            index = attrs.apply(index);

            forwardBlock.block.bind(); // instances
            Base<T>::setupAttrs(attrs, index);
        }

        friend class System;
    };

    template<> 
    void Manager<Directional>::setupDeferred(GLattrarr& attrs) const {
        deferredVao.bind();

        Base<Directional>::bindGeometry(); // vertices
        attrs.add<vec2>(2);
        auto index = attrs.apply();

        forwardBlock.block.bind(); // instances
        Base<Directional>::setupAttrs(attrs, index);
    }

    template<> void Manager<Directional>::finishGroups() {
        forwardBlock.block.data(forwardBlock.block.size, nullptr);
        for (auto& group : groups)
            group.update();
    }

    Manager<Point>       make_manager_point();
    Manager<Directional> make_manager_directional();
    Manager<Spotlight>   make_manager_spotlight();

    class Block {
    public:
        template<typename T>

        Block() = default;
        Block(GLprogram& program, const char* point, const char* directional, const char* spot) {
            pointLights = program.getUniformBlock<Point>(point, 0);
            directionalLights = program.getUniformBlock<Directional>(directional, 1);
            spotLights = program.getUniformBlock<Spotlight>(spot, 2);
        }

        GLuniformblock<Point> pointLights;
        GLuniformblock<Directional> directionalLights;
        GLuniformblock<Spotlight> spotLights;
    };

    class System {
    public:

        System() {
            GLattrarr attrs;
            pointLights.setupDeferred(attrs);
            directionalLights.setupDeferred(attrs);
            spotLights.setupDeferred(attrs);
        }

        Manager<Point> pointLights = make_manager_point();
        Manager<Directional> directionalLights = make_manager_directional();
        Manager<Spotlight> spotLights = make_manager_spotlight();

        Block getForwardBlock(GLprogram& program, const char* point, const char* directional, const char* spot) {
            Block b(program, point, directional, spot);
            b.pointLights.block = pointLights.forwardBlock.block;
            b.directionalLights.block = directionalLights.forwardBlock.block;
            b.spotLights.block = spotLights.forwardBlock.block;
            return b;
        }

        void update() {
            pointLights.update();
            directionalLights.update();
            spotLights.update();
        }

        void updateCamera(Camera* camera) const {
            pointLights.updateCamera(camera);
            directionalLights.updateCamera(camera);
            spotLights.updateCamera(camera);
        }

        void defer(Render::MaterialPass* renderer, const uint32_t group) {
            pointLights.defer(renderer, group);
            directionalLights.defer(renderer, group);
            spotLights.defer(renderer, group);
        }
    };

};