#include "Entity3D.h"
#include "sgct.h"

void Entity3D::writeData()
{
    for(int i=0; i<3; i++)
        sgct::SharedData::Instance()->writeFloat( glm::value_ptr(position)[i] );

    for(int i=0; i<3; i++)
        sgct::SharedData::Instance()->writeFloat( glm::value_ptr(orientation)[i] );

    sgct::SharedData::Instance()->writeFloat( scale );

    for(int i=0; i<3; i++)
        sgct::SharedData::Instance()->writeFloat( glm::value_ptr(color)[i] );
}

void Entity3D::getData()
{
	for(int i=0; i<3; i++)
		glm::value_ptr(position)[i] = sgct::SharedData::Instance()->readFloat();

	for(int i=0; i<3; i++)
		glm::value_ptr(orientation)[i] = sgct::SharedData::Instance()->readFloat();

    scale = sgct::SharedData::Instance()->readFloat();

	for(int i=0; i<3; i++)
		glm::value_ptr(color)[i] = sgct::SharedData::Instance()->readFloat();
}

mat4 Entity3D::getLocalMatrix()
{
    mat4 m = glm::translate(mat4(1.0f),position);
    m = glm::scale(m,vec3(scale,scale,scale));
    m = m*glm::yawPitchRoll(orientation.y,orientation.x,orientation.z);

    return m;
}

mat4 Entity3D::getWorldMatrix()
{
    mat4 m = getLocalMatrix();
    Entity3D* p = parent;

    while(p != NULL)
    {
        m = p->getLocalMatrix()*m;
    }

    return m;
}

void Entity3D::draw()
{
    if(!drawObj)
        return;

    glBindTexture( GL_TEXTURE_2D, textureID );
    glPushMatrix();
        mat4 m = getLocalMatrix();
        glMultMatrixf(glm::value_ptr(m));
        glColor3f(color.x,color.y,color.z);
        drawObj->draw();
    glPopMatrix();
}
