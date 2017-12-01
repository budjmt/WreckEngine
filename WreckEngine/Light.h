#pragma once

#include "slot_map.h"

#include "gl_structs.h"

#include "Render.h"

// all lights try to align to vec4 while also being aware of an implicit padding uint32 from slot_map
#define PADF1 float _pad1_
#define PADF2 PADF1, _pad2_
#define PADF3 PADF2, _pad3_

#include "Camera.h"

namespace Light {

    // defines a general light interface
    template<class T>
    struct Base {
        // uint32_t priority; // conceptual, cull lower priority lights on lower-end platforms

        static constexpr auto elemSize() { return slot_map<T>::dataSize; }

        static GLuint setupAttrs(GLattrarr& attrs, size_t offset = 0, GLuint baseIndex = 0) {
            return T::setupAttrsImpl(attrs, offset, baseIndex);
        }

        static GLuint setupDeferredAttrs(GLattrarr& attrs, GLuint baseIndex = 0) {
            return T::setupDeferredAttrsImpl(attrs, baseIndex);
        }

        static GLuint setupGeometry(GLattrarr& attrs) {
            return T::setupGeometryImpl(attrs);
        }

        static void draw(Render::MaterialPass* r, const uint32_t group, const GLVAO* vao, const Render::Info* info, size_t instances) {
            T::drawImpl(r, group, vao, info, instances);
        }
    };

    // expands uniformly from a point
    struct Point : public Base<Point> {
        vec3 position;
        int isOff = 0; // it's a bool, but an int for shader reasons
        vec3 color; // lights can't have opacity
        GLuint tag;
        vec2 falloff; // light radius; inner = x, outer = y
        int castsShadow = 0;

        struct DeferredData {
            mat4 world;
        };

        static GLuint setupAttrsImpl(GLattrarr& attrs, size_t offset, GLuint baseIndex) {
            attrs.add<vec3>(1, 1);
            attrs.add<int>(1, 1);
            attrs.add<vec3>(1, 1);
            attrs.add<GLuint>(1, 1);
            attrs.add<vec2>(1, 1);
            attrs.add<int>(1, 1);
            return attrs.apply(baseIndex, sizeof(float) * 0 + sizeof(uint32_t), offset);
        }

        static GLuint setupDeferredAttrsImpl(GLattrarr& attrs, GLuint baseIndex) {
            attrs.add<mat4>(1, 1);
            return attrs.apply(baseIndex, sizeof(float) * 0);
        }

        static GLuint setupGeometryImpl(GLattrarr& attrs);

        static void drawImpl(Render::MaterialPass* r, const uint32_t group, const GLVAO* vao, const Render::Info* info, size_t instances) {
            r->scheduleDrawElements(0, nullptr, vao, info, GL_TRIANGLES, count, GLtype<GLuint>(), instances);
        }

        bool isTransformed(const Point& update) const {
            return position != update.position;
        }

        DeferredData getDeferredData() const {
            DeferredData data;
            data.world = glm::translate(position) * glm::scale(vec3(falloff.y * 2 + 0.1f));
            return data;
        }

    private:
        static size_t count;
    };

    // uniformly lights all objects from a direction
    struct Directional : public Base<Directional> {
        vec3 direction;
        int isOff = 0;
        vec3 color;
        GLuint tag;
        int castsShadow = 0;
        PADF2;

        struct DeferredData {
            mat4 lightWorldViewProj;
        };

        static GLuint setupAttrsImpl(GLattrarr& attrs, size_t offset, GLuint baseIndex) {
            attrs.add<vec3>(1, 1);
            attrs.add<int>(1, 1);
            attrs.add<vec3>(1, 1);
            attrs.add<GLuint>(1, 1);
            attrs.add<int>(1, 1);
            return attrs.apply(baseIndex, sizeof(float) * 2 + sizeof(uint32_t), offset);
        }

        static GLuint setupDeferredAttrsImpl(GLattrarr& attrs, GLuint baseIndex) {
            attrs.add<mat4>(1, 1);
            return attrs.apply(baseIndex, sizeof(float) * 0);
        }

        static GLuint setupGeometryImpl(GLattrarr& attrs);

        static void drawImpl(Render::MaterialPass* r, const uint32_t group, const GLVAO* vao, const Render::Info* info, size_t instances) {
            r->scheduleDrawArrays(group, nullptr, vao, info, GL_TRIANGLES, 3, instances);
        }

        bool isTransformed(const Directional& update) const;
        DeferredData getDeferredData() const;
    };

    // expands conically from a point and direction
    struct Spotlight : public Base<Spotlight> {
        vec3 position;
        int isOff = 0;
        vec3 direction;
        GLuint tag;
        vec2 falloffRad;
        vec2 falloffLen;
        vec3 color;
        int castsShadow = 0;
        PADF3;

        struct DeferredData {
            mat4 world;
            mat4 lightViewProj;
        };

        static GLuint setupAttrsImpl(GLattrarr& attrs, size_t offset, GLuint baseIndex) {
            attrs.add<vec3>(1, 1);
            attrs.add<int>(1, 1);
            attrs.add<vec3>(1, 1);
            attrs.add<GLuint>(1, 1);
            attrs.add<vec4>(1, 1); // merge the two fields into one
            attrs.add<vec3>(1, 1);
            attrs.add<int>(1, 1);
            return attrs.apply(baseIndex, sizeof(float) * 3 + sizeof(uint32_t), offset);
        }

        static GLuint setupDeferredAttrsImpl(GLattrarr& attrs, GLuint baseIndex) {
            attrs.add<mat4>(2, 1);
            return attrs.apply(baseIndex, sizeof(float) * 0);
        }

        static GLuint setupGeometryImpl(GLattrarr& attrs);

        static void drawImpl(Render::MaterialPass* r, const uint32_t group, const GLVAO* vao, const Render::Info* info, size_t instances) {
            r->scheduleDrawElements(0, nullptr, vao, info, GL_TRIANGLES, count, GLtype<GLuint>(), instances);
        }

        bool isTransformed(const Spotlight& update) const {
            return direction != update.direction || position != update.position;
        }

        DeferredData getDeferredData() const {
            DeferredData data;
            auto radScale = falloffRad.y * 2;
            data.world = glm::translate(position) * rotateBetween(vec3(0,-1,0), direction) * glm::scale(vec3(radScale, falloffLen.y, radScale) + vec3(0.1f));
            data.lightViewProj = glm::lookAt(position, position + direction, { 0, 1, 0 });
            return data;
        }

    private:
        static size_t count;
    };

    // class Area : public Base
    // class Volume : public Base

    enum UpdateFreq : uint32_t { NEVER = 0, RARELY, SOMETIMES, OFTEN };

    template<typename T> using light_key = typename slot_map<T>::key;

    template<typename T> class Manager;

    template<typename T>
    class Group {
    public:
        GLbuffer subBuffer, subDeferredInstanceBuffer;
        uint32_t id{};

        using light_key = light_key<T>;

        std::pair<size_t, size_t> getBufferSizes() const {
            auto numLights = lights.size();
            return { Base<T>::elemSize() * numLights, sizeof(T::DeferredData) * numLights };
        }

        // sets the buffer used and updates the local internal data; returns the number of bytes used for the light sub-buffer
        // group size is fixed once this function is called
        size_t setBuffers(GLbuffer buffer, GLbuffer deferredInstances) {
            subBuffer = buffer;
            subBuffer.set(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
            subDeferredInstanceBuffer = deferredInstances;

            auto sizes = getBufferSizes();
            subDeferredInstanceBuffer.size = sizes.second;
            return subBuffer.size = sizes.first;
        }

        // updates the group's light-value sub-buffer contents as required; assumes the buffer is bound
        // returns lights.size()
        size_t updateLights(uint32_t bufferIndex, size_t initOffset) {
            auto numLights = lights.size();
            if (neededUpdates <= OFTEN) {
                auto offset = freqData[neededUpdates].offset;
                subBuffer.subdata(lights.data() + offset, (numLights - offset) * Base<T>::elemSize(), (bufferIndex + offset) * Base<T>::elemSize() + initOffset);
                neededUpdates = (UpdateFreq)(OFTEN + 1);
            }
            return numLights;
        }

        // updates the group's deferred data sub buffer contents as required; assumes the buffer is bound
        // returns lights.size()
        size_t updateDeferred(uint32_t bufferIndex) {
            while (transformedLights.size()) {
                auto index = transformedLights.back();
                auto instanceData = lights.valueAt(index).getDeferredData();
                subDeferredInstanceBuffer.subdata(&instanceData, sizeof(instanceData), (bufferIndex + index) * sizeof(instanceData));
                transformedLights.pop_back();
            }
            return lights.size();
        }

        light_key addLight(T light, const UpdateFreq freq) {
            // cannot add lights after group is finalized
            if (subBuffer.size > 0)
                return { lights.size(), 0 };

            auto data = freqData[freq];
            size_t offset = data.offset + data.size;

            ++freqData[freq].size;

            switch (freq) {
            case NEVER:
                ++freqData[RARELY].offset;
            case RARELY:
                ++freqData[SOMETIMES].offset;
            case SOMETIMES:
                ++freqData[OFTEN].offset;
            }

            return lights.insert(begin(lights) + offset, light);
        }

        // get a light index after all lights have been inserted into the group
        light_key getLightKeyByTag(uint32_t tag, UpdateFreq frequency) const { 
            auto data = freqData[frequency];
            for (auto i = data.offset, end = i + data.size; i < end; ++i) {
                if (lights.valueAt(i).tag == tag)
                    return *lights.key_of_index(i);
            }
            return { lights.size(), 0 };
        }

        T getLight(light_key key) const { return *lights[key]; }

        // FIX : this has a race condition when executed from the update thread
        // updates the given light with the data passed; if the light was moved, then its deferred data is updated immediately. 
        // the rest of its data is updated at the end of the render frame
        void updateLight(light_key key, UpdateFreq frequency, const T& light) {
            assert(frequency != NEVER);

            auto index = lights.index_of_key(key);
            assert(index >= freqData[frequency].offset && index < freqData[frequency].offset + freqData[frequency].size); // the index falls outside the allocated range

            auto& lightData = lights.valueAt(index);

            if (lightData.isTransformed(light))
                transformedLights.push_back(index);

            lightData = light; 
            if (frequency < neededUpdates) 
                neededUpdates = frequency;
        }

    private:
        slot_map<T> lights;
        std::vector<size_t> transformedLights;
        struct { size_t size, offset; } freqData[4]{};
        UpdateFreq neededUpdates = (UpdateFreq)(OFTEN + 1);

        void flagForRefresh() {
            neededUpdates = NEVER; // this should ONLY occur for a refresh
            // deferred data is manually refreshed
        }

        uint32_t refreshDeferredInstanceBuffer(uint32_t bufferIndex) {
            transformedLights.clear();
            auto numLights = lights.size();

            std::vector<T::DeferredData> deferredData;
            deferredData.reserve(numLights);
            for (auto& light : lights.values())
                deferredData.emplace_back(light.getDeferredData());
            subDeferredInstanceBuffer.subdata(deferredData.data(), sizeof(T::DeferredData) * numLights, bufferIndex * sizeof(T::DeferredData));

            return numLights;
        }

        friend class Manager<T>;
    };

    template<typename T>
    class Manager {
        slot_map<Group<T>> groups; // probably the wrong tool for the job but works for now
        bool groupChange = false;

        GLVAO deferredVao;
        GLbuffer deferredInstanceBuffer;
    public:
        Manager() { 
            deferredVao.create();
            forwardBlock.create();
            deferredInstanceBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW); 
        }

        GLuniformblock<T> forwardBlock;
        Render::Info renderInfo;

        using group_proxy = proxy<decltype(groups)>;
        using group_key = typename slot_map<Group<T>>::key;

        // gets a proxy to the group associated with the given key
        group_proxy getGroup(group_key k) { return make_proxy(groups, k); }

        // gets a proxy to the group associated with [index] in the slot_map
        group_proxy getGroupWithIndex(uint32_t index) { return getGroup(*groups.key_of_index(index)); }

        // tries to get a proxy to a group with [id], returns an empty one on fail
        group_proxy getGroupWithId(uint32_t id) {
            auto groupBeg = begin(groups.values()), groupEnd = end(groups.values());
            auto g = std::find(groupBeg, groupEnd, [id](auto& val) { return val.id == id; });
            return (g == groupEnd) ? group_proxy() : getGroupWithIndex(g - groupBeg);
        }

        // equivalent to using resetGroups and then adding each light in the range
        void setGroups(std::initializer_list<Group<T>> groups_init) {
            resetGroups();
            groups = groups_init;
            refreshGroups();
        }

        // equivalent to using resetGroups and then adding each light in the range
        // moves out of the given range into the groups
        template<typename Iter>
        void setGroups(Iter inGroupsBegin, Iter inGroupsEnd) {
            resetGroups();
            groups.reserve(inGroupsEnd - inGroupsBegin);
            groups.insert(begin(groups), inGroupsBegin, inGroupsEnd);
            refreshGroups();
        }

        // resets the internal group set; call before making changes
        void resetGroups() {
            groups.clear();
            groupChange = true;
        }

        // adds a group to the set
        group_key addGroup(Group<T>&& lightGroup) {
            groupChange = true;
            return groups.emplace_back(lightGroup);
        }

        // removes a group from the set
        Group<T> removeGroup(group_key key) {
            groupChange = true;
            Group<T> group = std::move(groups[key]);
            groups.removeAtBack(key);
            return group;
        }

        void update() {
            if (groupChange) {
                refreshGroups();
            }
            else {
                uint32_t processedLights = 0;
                deferredInstanceBuffer.bind();
                for (auto& group : groups.values())
                    processedLights += group.updateDeferred(processedLights);
            }

            uint32_t processedLights = 0;
            forwardBlock.block.bindAs(GL_ARRAY_BUFFER);
            for (auto& group : groups.values())
                processedLights += group.updateLights(processedLights, sizeof(vec4));
        }

        void forward(const GLuint index) {
            if (groups.size() == 0) return;
            forwardBlock.index = index;
            forwardBlock.bind();
        }

        void defer(Render::MaterialPass* renderer, const size_t renderGroup) const {
            // if a light value is ever <= sizeof(vec4), there will be issues calculating the correct number of instances
            Base<T>::draw(renderer, renderGroup, &deferredVao, &renderInfo, getNumLights());
        }

    private:
        size_t getNumLights() const { return forwardBlock.block.size / Base<T>::elemSize(); }

        void refreshGroups() {
            forwardBlock.block.size = sizeof(vec4); // space for the light count
            deferredInstanceBuffer.size = 0;
            for (auto& group : groups.values()) {
                group.flagForRefresh();
                forwardBlock.block.size += group.setBuffers(forwardBlock.block, deferredInstanceBuffer);
            }
            refreshBuffers();
            groupChange = false; // group changes have now been processed
        }

        // completes a group set update by allocating the VBO and populating any data that must be immediately addressed
        void refreshBuffers() {
            uint32_t numLights = getNumLights();
            refreshMainBuffer(numLights);

            if constexpr(std::is_same_v<Directional, T>) return; // Directional doesn't have deferred data

            deferredInstanceBuffer.bind();
            deferredInstanceBuffer.data(numLights * sizeof(T::DeferredData), nullptr);

            uint32_t processedLights = 0;
            for (auto& group : groups.values())
                processedLights += group.refreshDeferredInstanceBuffer(processedLights);
        }

        void refreshMainBuffer(uint32_t numLights) {
            // the additional numLights data tells the shader how many lights there are for forward rendering
            forwardBlock.block.bind();
            forwardBlock.block.data(forwardBlock.block.size, nullptr);
            forwardBlock.block.subdata(&numLights, sizeof(numLights));
        }

        void setupDeferred(GLattrarr& attrs) const {
            deferredVao.bind();

            auto index = Base<T>::setupGeometry(attrs); // vertices + elements (if applicable)

            deferredInstanceBuffer.bind();
            index = Base<T>::setupDeferredAttrs(attrs, index); // per-instance data that isn't in the light struct

            forwardBlock.block.bindAs(GL_ARRAY_BUFFER); // instances
            Base<T>::setupAttrs(attrs, sizeof(vec4), index); // remember to pass the offset from the numLights vec in
        }

        friend class System;
    };

    Manager<Point>       make_manager_point();
    Manager<Spotlight>   make_manager_spotlight();
    Manager<Directional> make_manager_directional();

    template<typename T> using group_proxy = typename Manager<T>::group_proxy;

    class System {
    public:
        System() {
            GLattrarr attrs;
            pointLights.setupDeferred(attrs);
            spotLights.setupDeferred(attrs);
            directionalLights.setupDeferred(attrs);
        }

        Manager<Point> pointLights = make_manager_point();
        Manager<Spotlight> spotLights = make_manager_spotlight();
        Manager<Directional> directionalLights = make_manager_directional();

        void connectLightBlocks(GLprogram& program, const char* point, const char* spot, const char* directional) {
            program.getUniformBlock<Point>(point, 0);
            program.getUniformBlock<Spotlight>(spot, 1);
            program.getUniformBlock<Directional>(directional, 2);
        }

        void update() {
            pointLights.update();
            spotLights.update();
            directionalLights.update();
        }

        void forward() {
            pointLights.forward(0);
            spotLights.forward(1);
            directionalLights.forward(2);
        }

        void defer(Render::MaterialPass* renderer, const uint32_t group) {
            pointLights.defer(renderer, group);
            spotLights.defer(renderer, group);
            directionalLights.defer(renderer, group); // must be rendered last to prevent clipping with depth test enabled
        }
    };

};