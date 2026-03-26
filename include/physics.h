#ifndef PHYSICS_H
#define PHYSICS_H

#include <iostream>
#include <mutex>

#include <vector>
#include <unordered_map>

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

class ObjContactListener : public JPH::ContactListener {
private:
    float kinetic_energy_to_amplitude = 0.0001f;
public:
    struct ContactEvent {
        glm::vec3 position;
        JPH::BodyID body_1;
        JPH::BodyID body_2;
        float amplitude;
    };

    std::mutex mutex;
    std::vector<ContactEvent> new_sound_waves;

    void OnContactAdded(
        const JPH::Body& body_1, const JPH::Body& body_2,
        const JPH::ContactManifold& manifold,
        JPH::ContactSettings&
    ) override {
        JPH::Vec3 rel_vel = body_1.GetLinearVelocity() - body_2.GetLinearVelocity();
        float impact_speed = abs(rel_vel.Dot(manifold.mWorldSpaceNormal));
        if (impact_speed < 1.0f) return;

        JPH::Vec3 pos = manifold.GetWorldSpaceContactPointOn1(0);
        JPH::Vec3 normal = manifold.mWorldSpaceNormal;
        glm::vec3 new_origin = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
        
        if (!body_1.IsStatic()) new_origin += 0.1f * glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());
        else if (!body_2.IsStatic()) new_origin -= 0.1f * glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());

        float speed_1 = body_1.GetLinearVelocity().Length();
        float speed_2 = body_2.GetLinearVelocity().Length();
        float kinetic_energy_1 = body_1.IsStatic() ? 0.0f : 0.5f / body_1.GetMotionProperties()->GetInverseMass() * speed_1 * speed_1;
        float kinetic_energy_2 = body_2.IsStatic() ? 0.0f : 0.5f / body_2.GetMotionProperties()->GetInverseMass() * speed_2 * speed_2;
        float restitution = (body_1.GetRestitution() + body_2.GetRestitution()) / 2.0f;
        float energy_lost = (kinetic_energy_1 + kinetic_energy_2) * (1 - restitution * restitution);
        energy_lost = energy_lost * kinetic_energy_to_amplitude;

        std::lock_guard<std::mutex> lock(mutex);
        new_sound_waves.push_back({new_origin, body_1.GetID(), body_2.GetID(), energy_lost});
    }
};

class IgnoreBodyFilter : public JPH::BodyFilter {
public:
    JPH::BodyID ignored_id_1;
    JPH::BodyID ignored_id_2;
    IgnoreBodyFilter(JPH::BodyID body_id_1, JPH::BodyID body_id_2 = JPH::BodyID()) : ignored_id_1(body_id_1), ignored_id_2(body_id_2) {}
    bool ShouldCollide(const JPH::BodyID& body_id) const override { return body_id != ignored_id_1 && body_id != ignored_id_2; }
    bool ShouldCollideLocked(const JPH::Body&) const override { return true; }
};

struct CollisionProperties {
    glm::vec3 position;
    int ignore_index_1 = -1;
    int ignore_index_2 = -1;
    float amplitude = 0.0f;
};

class PhysicsHandle {
private:
    const int step_size = 2;
    const JPH::uint physics_max_bodies = 1024;
    const JPH::uint physics_num_mutexes = 0;
    const JPH::uint physics_max_body_pairs = 1024;
    const JPH::uint physics_max_contact_constraints = 1024;
    BPLayerInterface bp_layer_interface;
    ObjVsBPLayerFilter obj_vs_bp_layer_filter;
    ObjVsObjFilter obj_vs_obj_filter;
    ObjContactListener contact_listener;
    JPH::TempAllocatorImpl* physics_temp_allocator = nullptr;
    JPH::JobSystemThreadPool* physics_job_system = nullptr;
    JPH::PhysicsSystem* physics_system = nullptr;
    std::unordered_map<JPH::uint32, int> body_id_to_index;
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

    std::vector<CollisionProperties> update(float delta_time, const std::vector<VulkanObject*> objects);
    void load_object_physics(const std::vector<VulkanObject*> objects);
    void fire_bullet(glm::vec3 position, glm::vec3 direction, const std::vector<VulkanObject*>& objects);
    std::vector<glm::vec3> find_reflection_points(glm::vec3 origin, int num_rays, float max_dist, int ignore_index_1 = -1, int ignore_index_2 = -1);
};

#endif