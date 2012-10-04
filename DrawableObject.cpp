#include "DrawableObject.h"
#include "ObjLoader.h"
#include "sgct.h"

DrawableObject::DrawableObject(Geometry *geom)
{
    init(geom);
}

DrawableObject::DrawableObject( const std::string &filename )
{
    Geometry g;
    loadObj(g,filename,1.0f);
    init(&g);
}

void DrawableObject::init(Geometry *geom)
{
    if(!geom)
        return;

    if(!geom->vertex.size() || !geom->face.size())
        return;

    vertexData = std::vector<tVertex>(geom->vertex.begin(),geom->vertex.end());
    triangleData = std::vector<tFace>(geom->face.begin(),geom->face.end());
}



void DrawableObject::load()
{
    unload();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(2, vbo);

    // bind buffer for vertices and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 8*vertexData.size()*sizeof(f32), &vertexData[0].x, GL_STATIC_DRAW);

    // Enable specific pointer for Vertex, for compability-mode and attributepointer for shader
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 8*sizeof(f32), (char*)NULL);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (char*)NULL);
    glEnableVertexAttribArray(0);

    // Enable specific pointer for Normal, for compability-mode and attributepointer for shader
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 8*sizeof(f32), (char*)NULL+3*sizeof(f32));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (char*)NULL+3*sizeof(f32));
    glEnableVertexAttribArray(1);

    // Enable specific pointer for TextureCoord, for compability-mode and attributepointer for shader
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 8*sizeof(f32), (char*)NULL+6*sizeof(f32));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (char*)NULL+6*sizeof(f32));
    glEnableVertexAttribArray(2);

    // Enable specific pointer for Color, for compability-mode and attributepointer for shader,
    // not really color, just the normal color coded.
    /*glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, 8*sizeof(f32), (char*)NULL+3*sizeof(f32));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (char*)NULL+3*sizeof(f32));
    glEnableVertexAttribArray(3);*/

    // Create and bind a BO for index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

    // copy data into the buffer object
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * triangleData.size()*sizeof(u32), &triangleData[0].point[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    existOnGpu = true;
}

void DrawableObject::unload()
{
    if(existOnGpu)
    {
        glDeleteBuffers(2, vbo);
        glDeleteVertexArrays(1, &vao);
    }

    existOnGpu = false;
}

void DrawableObject::draw()
{
    if(!existOnGpu)
        load();

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 3 * triangleData.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
