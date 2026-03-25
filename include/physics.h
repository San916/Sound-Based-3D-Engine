#ifndef PHYSICS_H
#define PHYSICS_H

#include <iostream>
#include <mutex>

#include <unordered_set>
#include <vector>

#include <glm/glm.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/PhysicsSystem.h>

typedef struct VulkanObject VulkanObject;

// Can add more object layers
namespace ObjLayers {
    static constexpr JPH::ObjectLayer STATIC = 0;
    static constexpr JPH::ObjectLayer DYNAMIC = 1;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
}

namespace BPLayers {
    static constexpr JPH::BroadPhaseLayer STATIC (0);
    static constexpr JPH::BroadPhaseLayer DYNAMIC (1);
    static constexpr JPH::uint NUM_LAYERS(2);
}

class BPLayerInterface : public JPH::BroadPhaseLayerInterface {
private:
    JPH::BroadPhaseLayer obj_to_bp_layer[2];
public:
    BPLayerInterface() {
        obj_to_bp_layer[ObjLayers::STATIC] = BPLayers::STATIC;
        obj_to_bp_layer[ObjLayers::DYNAMIC] = BPLayers::DYNAMIC;
    }
    JPH::uint GetNumBroadPhaseLayers () const { return BPLayers::NUM_LAYERS; };
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer obj_layer) const override { return obj_to_bp_layer[obj_layer]; }
    const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer bp_layer) const {
		switch ((JPH::BroadPhaseLayer::Type)bp_layer) {
            case (JPH::BroadPhaseLayer::Type)BPLayers::STATIC:
                return "STATIC";
            case (JPH::BroadPhaseLayer::Type)BPLayers::DYNAMIC:
                return "DYNAMIC";
            default:
                return "INVALID";
		}
	}
};

class ObjVsBPLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer obj_layer, JPH::BroadPhaseLayer bl_layer) const override {
        return obj_layer == ObjLayers::DYNAMIC || bl_layer == BPLayers::DYNAMIC;
    }
};

class ObjVsObjFilter : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer obj_layer_1, JPH::ObjectLayer obj_layer_2) const override {
        return obj_layer_1 == ObjLayers::DYNAMIC || obj_layer_2 == ObjLayers::DYNAMIC;
    }
};

class BulletContactListener : public JPH::ContactListener {
public:
    std::unordered_set<JPH::uint32> bullet_ids;
    std::mutex mutex;
    std::vector<glm::vec3> collision_positions;

    void OnContactAdded(
        const JPH::Body& body1, const JPH::Body& body2,
        const JPH::ContactManifold& manifold,
        JPH::ContactSettings&
    ) override {
        if (!bullet_ids.count(body1.GetID().GetIndexAndSequenceNumber()) && !bullet_ids.count(body2.GetID().GetIndexAndSequenceNumber())) return;

        JPH::Vec3 rel_vel = body1.GetLinearVelocity() - body2.GetLinearVelocity();
        float impact_speed = abs(rel_vel.Dot(manifold.mWorldSpaceNormal));
        if (impact_speed < 2.0f) return;
        
        bool bullet_is_body1 = bullet_ids.count(body1.GetID().GetIndexAndSequenceNumber());
        JPH::Vec3 pos = bullet_is_body1 ? manifold.GetWorldSpaceContactPointOn1(0) : manifold.GetWorldSpaceContactPointOn2(0);

        JPH::Vec3 normal = bullet_is_body1 ? manifold.mWorldSpaceNormal : -manifold.mWorldSpaceNormal;
        glm::vec3 collision_pos = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()) + 0.01f * glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());

        std::lock_guard<std::mutex> lock(mutex);
        collision_positions.push_back(collision_pos);
    }
};

class IgnoreBodyFilter : public JPH::BodyFilter {
public:
    JPH::BodyID ignored_id;
    IgnoreBodyFilter(JPH::BodyID id) : ignored_id(id) {}
    bool ShouldCollide(const JPH::BodyID& body_id) const override { return body_id != ignored_id; }
    bool ShouldCollideLocked(const JPH::Body&) const override { return true; }
};

class PhysicsHandle {
private:
    const JPH::uint physics_max_bodies = 1024;
    const JPH::uint physics_num_mutexes = 0;
    const JPH::uint physics_max_body_pairs = 1024;
    const JPH::uint physics_max_contact_constraints = 1024;
    BPLayerInterface bp_layer_interface;
    ObjVsBPLayerFilter obj_vs_bp_layer_filter;
    ObjVsObjFilter obj_vs_obj_filter;
    BulletContactListener bullet_contact_listener;
    JPH::TempAllocatorImpl* physics_temp_allocator = nullptr;
    JPH::JobSystemThreadPool* physics_job_system = nullptr;
    JPH::PhysicsSystem* physics_system = nullptr;
    std::vector<JPH::BodyID> physics_body_ids;
    std::vector<JPH::BodyID> bullet_body_ids;
    std::vector<size_t> bullet_object_indices;
    size_t next_bullet = 0;
    JPH::BodyInterface* body_interface = nullptr;
public:
    PhysicsHandle();
    ~PhysicsHandle();

    PhysicsHandle(const PhysicsHandle&) = delete;
    PhysicsHandle& operator=(const PhysicsHandle&) = delete;

    std::vector<glm::vec3> update(float delta_time, const std::vector<VulkanObject*> objects);
    void load_object_physics(const std::vector<VulkanObject*> objects);
    void fire_bullet(glm::vec3 position, glm::vec3 direction, const std::vector<VulkanObject*>& objects);
    std::vector<glm::vec3> find_reflection_points(glm::vec3 origin, int num_rays, float max_dist, int obj_to_ignore);
};

#endif