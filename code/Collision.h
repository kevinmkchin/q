#pragma once

#include "Common.h"

#include <functional>

#include <gmath.h>

struct CollisionEventArgs
{
    vec3 normal;
    float depth = 0.f;
    EntityIndex other = 0;
};

enum class ColliderType
{
    Invalid,
    Mesh,
    Sphere,
    Line
};

struct EntityTransform;

/** 2022-06-22 (Kevin): Design of a collider is as follows:
    An entity class can contain a collider. A collider knows the EntityIndex of its owning entity, and
    the address to the owning entity's transform component. A collider can also have an onCollisionCallback
    std::function defined - if this callback is defined then it will be called after checking collisions. */
struct Collider
{
    /* A preparation function which transforms the collider to World Space from Model Space */
    virtual void TransformCollider() = 0;

    /* Returns the furthest point on the collider along a given direction */
    virtual vec3 FindFurthestPoint(vec3 direction) const = 0;

    EntityIndex owner = EntityIndexInvalid;
    EntityTransform* ownerTransform = nullptr;
    ColliderType colliderType = ColliderType::Invalid;
    std::function<void(CollisionEventArgs)> onCollisionCallback = nullptr;
};

struct MeshCollider : Collider
{
    std::vector<vec3> verticesModelSpace; // TODO(Kevin): make this a pointer to so e.g. all cars in a racing game can share same model space collider

    MeshCollider(EntityIndex ownerArg, EntityTransform* transformArg, std::function<void(CollisionEventArgs)> onCollisionCallbackArg);

    void TransformCollider() override;

    vec3 FindFurthestPoint(vec3 direction) const override;

private:
    std::vector<vec3> verticesWorldSpace; // changes based on transform of owning game object
};

struct SphereCollider : Collider
{
    float radius; // radius in model frame. before being affected by scale.

    SphereCollider(EntityIndex ownerArg, EntityTransform* transformArg, std::function<void(CollisionEventArgs)> onCollisionCallbackArg);

    void TransformCollider() override;

    vec3 FindFurthestPoint(vec3 direction) const override;

    vec3 GetPosition() const;

    float GetScaledRadius() const;

private:
    vec3 position; // position in world frame. should be position of owningGameObject
    float scaledRadius; // radius actually used for FindFurthestPoint
};

struct LineCollider : Collider
{
    vec3 a;
    vec3 b;

    LineCollider(EntityIndex ownerArg, EntityTransform* transformArg, std::function<void(CollisionEventArgs)> onCollisionCallbackArg);

    void TransformCollider() override;

    vec3 FindFurthestPoint(vec3 direction) const override;
};


void CheckCollisionAndNotify(Collider* colliderA, Collider* colliderB);
