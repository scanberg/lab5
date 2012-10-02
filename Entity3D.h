#pragma once
#ifndef ENTITY3D_H
#define ENTITY3D_H

class Entity3D
{
protected:
    Entity3D *parent;
    vec3 position;
    vec3 orientation;
    f32 scale;

    DrawableObject *drawObj;
public:
    Entity3D() : parent(NULL), drawObj(NULL) {}
    virtual ~Entity3D() {}

    void setPosition(f32 x, f32 y, f32 z) { position = vec3(x,y,z); }
    void setOrientation(f32 x, f32 y, f32 z) { orientation = vec3(x,y,z); }
    void setScale(f32 s) { scale = s; }

    void setRotation(f32 x, f32 y, f32 z);

    void setDrawable(DrawableObject *_drawObj){ drawObj = _drawObj; }

    mat4 getLocalMatrix();
    void setLocalMatrix( const mat4 &mat )

    virtual void update() {}
    virtual void draw() {}

};

#endif
