#include <iostream>
#include <cmath>

#include <vector>

#include <vulkan_object.h>

#include <physics.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

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
    physics_system->SetContactListener(&bullet_contact_listener);
}

PhysicsHandle::~PhysicsHandle() {
    delete physics_system;
    delete physics_job_system;
    delete physics_temp_allocator;
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

// MODIFIES: this->body_interface
// EFFECTS: Adds the objects into the physics engine
//     For each object, use the properties to determine the settings of the physics body creation
//     If mass = 0.0, the object is static, otherwise dynamic, with the given mass
//     For static objects, and thus we can use a meshshape for accurate static geometry
//     For dynamic objects, use spheres for bullets, and convex hulls otherwise
//     Creates a ground as it doesnt exist in objects
void PhysicsHandle::load_object_physics(const std::vector<VulkanObject*> objects) {
    body_interface = &physics_system->GetBodyInterface();

    for (const VulkanObject* obj : objects) {
        const ObjectProperties& properties = obj->properties;

        bool is_dynamic = properties.physics_enabled && properties.mass > 0.0f;

        glm::vec3 rotation_rad = glm::radians(properties.rotation);
        JPH::Quat rotation = JPH::Quat::sEulerAngles(JPH::Vec3(rotation_rad.x, rotation_rad.y, rotation_rad.z));

        JPH::ShapeRefC shape;
        if (is_dynamic) {
            if (properties.bullet) {
                JPH::SphereShapeSettings sphere(properties.scale.x);
                shape = sphere.Create().Get();
            } else {
                JPH::ConvexHullShapeSettings hull;
                for (const auto& vertex : obj->get_vertices()) {
                    hull.mPoints.push_back(JPH::Vec3(
                        vertex.pos.x * properties.scale.x,
                        vertex.pos.y * properties.scale.y,
                        vertex.pos.z * properties.scale.z
                    ));
                }
                shape = hull.Create().Get();
            }
        } else {
            JPH::TriangleList triangles;
            const auto& vertices = obj->get_vertices();
            const auto& indices = obj->get_indices();
            for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                const auto& v0 = vertices[indices[i]];
                const auto& v1 = vertices[indices[i + 1]];
                const auto& v2 = vertices[indices[i + 2]];
                triangles.push_back(JPH::Triangle(
                    JPH::Float3(v0.pos.x * properties.scale.x, v0.pos.y * properties.scale.y, v0.pos.z * properties.scale.z),
                    JPH::Float3(v1.pos.x * properties.scale.x, v1.pos.y * properties.scale.y, v1.pos.z * properties.scale.z),
                    JPH::Float3(v2.pos.x * properties.scale.x, v2.pos.y * properties.scale.y, v2.pos.z * properties.scale.z)
                ));
            }
            JPH::MeshShapeSettings mesh(triangles);
            shape = mesh.Create().Get();
        }

        JPH::BodyCreationSettings settings(
            shape,
            JPH::RVec3(properties.position.x, properties.position.y, properties.position.z),
            rotation,
            is_dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
            is_dynamic ? ObjLayers::DYNAMIC : ObjLayers::STATIC
        );

        if (is_dynamic) settings.mMassPropertiesOverride.mMass = properties.mass;

        JPH::BodyID body_id = body_interface->CreateAndAddBody(settings, JPH::EActivation::Activate);

        body_interface->SetRestitution(body_id, 0.25f);

        physics_body_ids.push_back(body_id);
        if (properties.bullet) {
            bullet_body_ids.push_back(body_id);
            bullet_object_indices.push_back(physics_body_ids.size() - 1);
            bullet_contact_listener.bullet_ids.insert(body_id.GetIndexAndSequenceNumber());
            body_interface->SetPosition(body_id, JPH::RVec3(0.0f, -1000.0f, 0.0f), JPH::EActivation::DontActivate);
            body_interface->DeactivateBody(body_id);
        }
    }

    JPH::BoxShapeSettings ground_shape(JPH::Vec3(100.0f, 1.0f, 100.0f));
    JPH::BodyCreationSettings ground_settings(
        ground_shape.Create().Get(),
        JPH::RVec3(0.0f, -1.0f, 0.0f),
        JPH::Quat::sIdentity(),
        JPH::EMotionType::Static,
        ObjLayers::STATIC
    );
    body_interface->CreateAndAddBody(ground_settings, JPH::EActivation::DontActivate);
}

void PhysicsHandle::fire_bullet(glm::vec3 position, glm::vec3 direction, const std::vector<VulkanObject*>& objects) {
    if (bullet_body_ids.empty()) return;

    size_t slot = next_bullet % bullet_body_ids.size();
    next_bullet++;

    JPH::BodyID id = bullet_body_ids[slot];
    objects[bullet_object_indices[slot]]->properties.visible = 1;

    const float bullet_speed = 50.0f;
    glm::vec3 velocity = glm::normalize(direction) * bullet_speed;

    body_interface->SetPosition(id, JPH::RVec3(position.x, position.y, position.z), JPH::EActivation::Activate);
    body_interface->SetLinearVelocity(id, JPH::Vec3(velocity.x, velocity.y, velocity.z));
    body_interface->SetAngularVelocity(id, JPH::Vec3(0.0f, 0.0f, 0.0f));
}

// EFFECTS: 
//     Creates num_rays uniformy distributed quasi-random rays using fibonacci sphere 
//     Returns up to num_rays hit positions within max_dist, which will be act as sound wave reflections.
std::vector<glm::vec3> PhysicsHandle::find_reflection_points(glm::vec3 origin, int num_rays, float max_dist, int obj_to_ignore = -1) {
    const JPH::NarrowPhaseQuery& query = physics_system->GetNarrowPhaseQuery();
    std::vector<glm::vec3> points;
    bool has_filter = obj_to_ignore >= 0 && obj_to_ignore < (int)physics_body_ids.size();
    IgnoreBodyFilter body_filter(has_filter ? physics_body_ids[obj_to_ignore] : JPH::BodyID());

    const float golden_ratio = 1.618f;
    for (int i = 0; i < num_rays; i++) {
        float theta = std::acos(1.0f - (2.0f * (i + 0.5f) / num_rays));
        float phi = 2.0f * 3.1415f * i / golden_ratio;

        glm::vec3 ray_dir(std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi));

        JPH::RRayCast ray(JPH::RVec3(origin.x, origin.y, origin.z), JPH::Vec3(ray_dir.x, ray_dir.y, ray_dir.z) * max_dist);
        JPH::RayCastResult hit;
        if (query.CastRay(ray, hit, JPH::BroadPhaseLayerFilter(), JPH::ObjectLayerFilter(), body_filter)) {
            JPH::RVec3 hit_pos = ray.GetPointOnRay(hit.mFraction);
            points.push_back(glm::vec3(hit_pos.GetX(), hit_pos.GetY(), hit_pos.GetZ()));
        }
    }
    return points;
}

// MODIFIES: this, objects
// EFFECTS: Updates the physics engine, then sets each objects properties using the updated values
//     Returns positions of any bullet collisions that occurred this frame
std::vector<glm::vec3> PhysicsHandle::update(float delta_time, const std::vector<VulkanObject*> objects) {
    physics_system->Update(delta_time, 5, physics_temp_allocator, physics_job_system);

    for (size_t i = 0; i < objects.size(); i++) {
        if (!objects[i]->properties.physics_enabled) continue;
        JPH::RVec3 pos = body_interface->GetPosition(physics_body_ids[i]);

        JPH::Quat rotation = body_interface->GetRotation(physics_body_ids[i]);
        glm::quat glm_rotation(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());

        objects[i]->properties.rotation = glm::degrees(glm::eulerAngles(glm_rotation));
        objects[i]->properties.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
    }

    std::vector<glm::vec3> collisions;
    {
        std::lock_guard<std::mutex> lock(bullet_contact_listener.mutex);
        collisions.swap(bullet_contact_listener.collision_positions);
    }
    return collisions;
}