#ifndef PHYSICS_H
#define PHYSICS_H

#include <iostream>

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
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

class PhysicsHandle {
private:
    const JPH::uint physics_max_bodies = 1024;
    const JPH::uint physics_num_mutexes = 0;
    const JPH::uint physics_max_body_pairs = 1024;
    const JPH::uint physics_max_contact_constraints = 1024;
    BPLayerInterface bp_layer_interface;
    ObjVsBPLayerFilter obj_vs_bp_layer_filter;
    ObjVsObjFilter obj_vs_obj_filter;
    JPH::TempAllocatorImpl* physics_temp_allocator = nullptr;
    JPH::JobSystemThreadPool* physics_job_system = nullptr;
    JPH::PhysicsSystem* physics_system = nullptr;
    std::vector<JPH::BodyID> physics_body_ids;
    JPH::BodyInterface* body_interface = nullptr;
public:
    PhysicsHandle();
    ~PhysicsHandle();

    PhysicsHandle(const PhysicsHandle&) = delete;
    PhysicsHandle& operator=(const PhysicsHandle&) = delete;

    void update(float delta_time, const std::vector<VulkanObject*> objects);
    void load_object_physics(const std::vector<VulkanObject*> objects);
};

#endif