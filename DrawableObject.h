#pragma once
#ifndef DRAWABLEOBJECT_H
#define DRAWABLEOBJECT_H

#include <vector>

#include "Types.h"
#include "Geometry.h"

class DrawableObject
{
private:
    std::vector<tVertex> vertexData;
    std::vector<tFace> triangleData;

    u32 vao;
    u32 vbo[2];

    bool existOnGpu;
public:
    DrawableObject() : existOnGpu(false) {}
    DrawableObject( Geometry *geom );
    DrawableObject( const std::string &filename );
    ~DrawableObject() { unload(); }

    void init(Geometry *geom);
    void load();
    void unload();
    void draw();
};

#endif
