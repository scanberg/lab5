#pragma once
#ifndef ENTITY3D_H
#define ENTITY3D_H

#include "Types.h"
#include "DrawableObject.h"

#define TEXTUREID_UNUSED 0

class Entity3D
{
protected:
    vec3 position;
    vec3 orientation;
    f32 scale;

    vec3 color;

    Entity3D *parent;
    DrawableObject *drawObj;
    u32 textureID;
public:
    Entity3D() : scale(1.0f), color(1.0f), parent(NULL), drawObj(NULL), textureID(TEXTUREID_UNUSED) {}
    virtual ~Entity3D() {}

    void setPosition(f32 x, f32 y, f32 z) { position = vec3(x,y,z); }
    void setPosition(const vec3 &pos) { position = pos; }
    void setOrientationXYZ(f32 x, f32 y, f32 z) { orientation = vec3(x,y,z); }
    void setOrientationXYZ(const vec3 &ori) { orientation = ori; }
    void setScale(f32 s) { scale = s; }

    void setColor(f32 r, f32 g, f32 b) { color = vec3(r,g,b); }

    void setDrawable(DrawableObject *_drawObj){ drawObj = _drawObj; }
    void setTexture(u32 _tex) { textureID=_tex; }

    vec3 getPosition() { return position; }
    vec3 getOrientationXYZ() { return orientation; }
    f32 getScale() { return scale; }

    void rotate(f32 x, f32 y, f32 z) { orientation += vec3(x,y,z); }
    void rotate(const vec3 &v) { orientation += v; }
    void translate(f32 x, f32 y, f32 z) { position += vec3(x,y,z); }
    void translate(const vec3 &v) { position += v; }

    mat4 getLocalMatrix();
    mat4 getWorldMatrix();

    virtual void draw();

    void writeData();
    void getData();
};

#endif
