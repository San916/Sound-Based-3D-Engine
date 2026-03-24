#include <iostream>

#include <vector>

#include <vulkan_object.h>

#include <physics.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

PhysicsHandle::PhysicsHandle() {
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    physics_temp_allocator = new JPH::TempAllocatorImpl({ 10 * 1024 * 1024 });
    physics_job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
    physics_system = new JPH::PhysicsSystem;
    physics_system->Init(
        physics_max_bodies, physics_num_mutexes, physics_max_body_pairs, physics_max_contact_constraints,
        bp_layer_interface, obj_vs_bp_layer_filter, obj_vs_obj_filter
    );

    physics_system->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
}

PhysicsHandle::~PhysicsHandle() {
    delete physics_system;
    delete physics_job_system;
    delete physics_temp_allocator;
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsHandle::load_object_physics(const std::vector<VulkanObject*> objects) {
    body_interface = &physics_system->GetBodyInterface();

    for (const VulkanObject* obj : objects) {
        const ObjectProperties& properties = obj->properties;

        JPH::BoxShapeSettings shape(JPH::Vec3(properties.scale.x, properties.scale.y, properties.scale.z));
        bool is_dynamic = properties.physics_enabled && properties.mass > 0.0f;

        glm::vec3 rotation_rad = glm::radians(properties.rotation);
        JPH::Quat rotation = JPH::Quat::sEulerAngles(JPH::Vec3(rotation_rad.x, rotation_rad.y, rotation_rad.z));

        JPH::BodyCreationSettings settings(
            shape.Create().Get(),
            JPH::RVec3(properties.position.x, properties.position.y, properties.position.z),
            rotation,
            is_dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
            is_dynamic ? ObjLayers::DYNAMIC : ObjLayers::STATIC
        );

        if (is_dynamic) settings.mMassPropertiesOverride.mMass = properties.mass;

        JPH::BodyID body_id = body_interface->CreateAndAddBody(settings, JPH::EActivation::Activate);

        body_interface->SetRestitution(body_id, 1.0f);

        physics_body_ids.push_back(body_id);
    }

    JPH::BoxShapeSettings ground_shape(JPH::Vec3(100.0f, 1.0f, 100.0f));
    JPH::BodyCreationSettings ground_settings(
        ground_shape.Create().Get(),
        JPH::RVec3(0.0f, -2.0f, 0.0f),
        JPH::Quat::sIdentity(),
        JPH::EMotionType::Static,
        ObjLayers::STATIC
    );
    body_interface->CreateAndAddBody(ground_settings, JPH::EActivation::DontActivate);
}

void PhysicsHandle::update(float delta_time, const std::vector<VulkanObject*> objects) {
    physics_system->Update(delta_time, 2, physics_temp_allocator, physics_job_system);

    for (size_t i = 0; i < objects.size(); i++) {
        if (!objects[i]->properties.physics_enabled) continue;
        JPH::RVec3 pos = body_interface->GetPosition(physics_body_ids[i]);

        JPH::Quat rotation = body_interface->GetRotation(physics_body_ids[i]);
        glm::quat glm_rotation(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());

        objects[i]->properties.rotation = glm::degrees(glm::eulerAngles(glm_rotation));
        objects[i]->properties.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
    }
}