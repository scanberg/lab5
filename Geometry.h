#pragma once

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <iostream>
#include <algorithm>
#include <vector>
#include "Types.h"

typedef struct
{
    f32 x,y,z;
    f32 nx,ny,nz;
    f32 s,t;
}tVertex;

typedef struct
{
    f32 x,y,z;
}tNormal;

typedef struct
{
    f32 s,t;
}tTexCoord;

typedef struct
{
    u32 point[3];
}tFace;

class Geometry
{
public:
    std::vector<tVertex> vertex;
    std::vector<tFace> face;
    std::vector<vec3> faceNormal;

    void clear() { vertex.clear(); face.clear(); }

    void process()
    {
        assert(vertex.size() > 0);

        glm::vec3 a,b;

        faceNormal.resize(face.size(),vec3(0));

        std::vector<vec3> tempNormal;
        tempNormal.resize(vertex.size(),vec3(0));

        std::vector<i32> sharedFaces;
        sharedFaces.resize(vertex.size(),0);

        for (i32 i=0; i<(i32)face.size(); ++i)
        {
            a.x = vertex[face[i].point[2]].x - vertex[face[i].point[0]].x;
            a.y = vertex[face[i].point[2]].y - vertex[face[i].point[0]].y;
            a.z = vertex[face[i].point[2]].z - vertex[face[i].point[0]].z;

            b.x = vertex[face[i].point[1]].x - vertex[face[i].point[0]].x;
            b.y = vertex[face[i].point[1]].y - vertex[face[i].point[0]].y;
            b.z = vertex[face[i].point[1]].z - vertex[face[i].point[0]].z;

            faceNormal[i] = glm::normalize(glm::cross(b,a));

            for(u32 u=0; u<3; ++u)
            {
                tempNormal[face[i].point[u]] += faceNormal[i];
                sharedFaces[face[i].point[u]]++;
            }
        }
        for (i32 i=0; i<(i32)vertex.size(); ++i)
        {
            if(sharedFaces[i]>0)
            {
                tempNormal[i] /= (f32)sharedFaces[i];
                tempNormal[i] = glm::normalize(tempNormal[i]);
            }
            if(vertex[i].nx == 0.0f && vertex[i].ny == 0.0f && vertex[i].nz == 0.0f)
            {
                vertex[i].nx = tempNormal[i].x;
                vertex[i].ny = tempNormal[i].y;
                vertex[i].nz = tempNormal[i].z;
            }
        }
    }

    Geometry& addGeometry(Geometry &g)
    {
        u32 vertexOffset = vertex.size();
        tFace f;
        for(u32 i=0; i<g.vertex.size(); ++i)
        {
            vertex.push_back(g.vertex[i]);
        }
        for(u32 i=0; i<g.face.size(); ++i)
        {
            f = g.face[i];
            f.point[0] += vertexOffset;
            f.point[1] += vertexOffset;
            f.point[2] += vertexOffset;

            face.push_back(f);
        }
        return *this;
    }

    void translate(f32 x, f32 y, f32 z)
    {
        for(u32 i=0; i<vertex.size(); ++i)
        {
            vertex[i].x += x;
            vertex[i].y += y;
            vertex[i].z += z;
        }
    }

    void translate(vec3 vec)
    {
        for(u32 i=0; i<vertex.size(); ++i)
        {
            vertex[i].x += vec.x;
            vertex[i].y += vec.y;
            vertex[i].z += vec.z;
        }
    }
};

#endif
