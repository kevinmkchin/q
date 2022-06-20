#pragma once

#include <gmath.h>

#include "Collision.h"
#include "Shader.h"

struct MeshGroup;

/** 2022-06-20 (Kevin): I've found an entity system design that I like (I don't like ECS, I don't like OOP).
    Even with a composition over inheritance design, there are multiple architectures one could implement - 
    I've seen people use multiple inheritance of pure virtual interfaces to "compose" an entity. 
    
    The approach that I like is this:

    All entities are inherited from an Entity class (nice for dispatch and having one pointer type), but they
    are at most 1 level deep in heritance i.e. never inherit a child of an Entity class.

    Have various components for various things (e.g. graphics, physics, chase player, health), and compose
    an entity type by combining these components and initializing them in different ways - one entity might
    start with 100 health while another entity starts with 30 health.

    Inevitably, these components will want to communicate with one another (although each component being 
    completely isolated is ideal, it is almost impossible to achieve and will require workarounds that will
    needlessly complicate your code). For example, the graphics/render component will want to communicate with
    the transform component, or the graphics/render component may want to communicate with the physics component
    so that it may render differently based on whether the entity is colliding with something. The simplest and
    most intuitive solution is just to allow these components to refer directly to each other. To enforce that
    every component has a reference to every other component it needs to communicate with, create a non-default
    constructor which takes a pointer to these other components as its arguments. This forces any instance of a
    given component to receive addresses for the components it needs to communicate with whenever this given
    component is created. Therefore, it is impossible for an entity to have component A without constructing A
    with the addresses of the components that A needs to communicate with.
    */

class TransformComponent
{
public:
    vec3 position;
    quat orientation;
    vec3 scale = {1.f,1.f,1.f};

public:
    mat4 GetModelMatrix();
};

class GameModelComponent
{
public:
    GameModelComponent(TransformComponent* transformArg);

    void Render(const Shader& shader);

public:
    MeshGroup* sharedMesh;
private:
    TransformComponent* transform;
};


//class HealthComponent
//{
//public:
//    virtual float health() = 0;
//protected:
//    float m_Health;
//};



