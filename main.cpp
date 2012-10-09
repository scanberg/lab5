#include "sgct.h"

#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Types.h"
#include "DrawableObject.h"

sgct::Engine * gEngine;

DrawableObject * sphere = NULL;

u32 earthTexture = 0;
u32 marsTexture = 0;
u32 moonTexture = 0;

enum picked
{
    none = 0, moon, earth, mars
};

//variables to share across cluster
u8 chosen = none;
mat4 transform;
double curr_time = 0.0;


//callbacks
void myInitOGLFun();
void myPreSyncFun();
void myDrawFun();
void myEncodeFun();
void myDecodeFun();

//input callbacks
void keyCallback(int key, int action);
void mouseButtonCallback(int button, int action);

int main( int argc, char* argv[] )
{
	gEngine = new sgct::Engine( argc, argv );

	gEngine->setInitOGLFunction( myInitOGLFun );
	gEngine->setPreSyncFunction( myPreSyncFun );
	gEngine->setDrawFunction( myDrawFun );
	gEngine->setKeyboardCallbackFunction( keyCallback );
	gEngine->setMouseButtonCallbackFunction( mouseButtonCallback );

	if( !gEngine->init() )
	{
		delete gEngine;
		return EXIT_FAILURE;
	}

	sgct::SharedData::Instance()->setEncodeFunction( myEncodeFun );
	sgct::SharedData::Instance()->setDecodeFunction( myDecodeFun );

	// Main loop
	gEngine->render();

    // Clean up
    delete gEngine;

    if(sphere)
        delete sphere;

	// Exit program
	exit( EXIT_SUCCESS );
}

void myInitOGLFun()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    /** Initialize OpenGL memory here, textures, lists, etc. **/
    sphere = new DrawableObject("media/sphere.obj");

    sgct::TextureManager::Instance()->setAnisotropicFilterSize(4.0f);

    sgct::TextureManager::Instance()->loadTexure(marsTexture, "mars", "media/mars.png", true);
	sgct::TextureManager::Instance()->loadTexure(earthTexture, "earth", "media/earth.png", true);
	sgct::TextureManager::Instance()->loadTexure(moonTexture, "moon", "media/moon.png", true);

}

void myPreSyncFun()
{
	if( gEngine->isMaster() )
    {
        curr_time = sgct::Engine::getTime();
        /** Calculate variables here for navigation, interaction, etc. **/
    }
}

void myDrawFun()
{
    /** Apply transforms for navigation and draw the scene **/
    glEnable(GL_TEXTURE_2D);

    glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::Instance()->getTextureByIndex(earthTexture) );
    glPushMatrix();
        glRotatef(10.0f*curr_time, 0,1,0);
        if(chosen == earth)
            glColor3f(1.0f,1.0f,1.0f);
        else
            glColor3f(0.6f,0.6f,0.6f);
        sphere->draw();
    glPopMatrix();

    glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::Instance()->getTextureByIndex(moonTexture) );
    glPushMatrix();
        glTranslatef(-1.2,0,0);
        glRotatef(20.0f*curr_time, 0,1,0);
        glScalef(0.26,0.26,0.26);
        if(chosen == moon)
            glColor3f(1.0f,1.0f,1.0f);
        else
            glColor3f(0.6f,0.6f,0.6f);
        sphere->draw();
    glPopMatrix();

    glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::Instance()->getTextureByIndex(marsTexture) );
    glPushMatrix();
        glTranslatef(1.2,0,0);
        glRotatef(5.0f*curr_time, 0.5,1,0);
        glScalef(0.5,0.5,0.5);
        if(chosen == mars)
            glColor3f(1.0f,1.0f,1.0f);
        else
            glColor3f(0.6f,0.6f,0.6f);
        sphere->draw();
    glPopMatrix();

    glColor3f(1.0f,1.0f,1.0f);
}

void myEncodeFun()
{
    sgct::SharedData::Instance()->writeDouble( curr_time );
    sgct::SharedData::Instance()->writeUChar( chosen );
	for(int i=0; i<16; i++)
		sgct::SharedData::Instance()->writeFloat( glm::value_ptr(transform)[i] );
}

void myDecodeFun()
{
    curr_time = sgct::SharedData::Instance()->readDouble();
    chosen = sgct::SharedData::Instance()->readUChar();
	for(int i=0; i<16; i++)
		glm::value_ptr(transform)[i] = sgct::SharedData::Instance()->readFloat();
}

void keyCallback(int key, int action)
{
    if( gEngine->isMaster() )
    {
        switch( key )
        {
    		case 'Q':
    			if(action == GLFW_PRESS)
    				gEngine->terminate();
    			break;

            case '1':
                if(action == GLFW_PRESS)
                    chosen = moon;
                break;

    		case '2':
                if(action == GLFW_PRESS)
                    chosen = earth;
                break;

            case '3':
                if(action == GLFW_PRESS)
                    chosen = mars;
                break;

            case '0':
                if(action == GLFW_PRESS)
                    chosen = none;
                break;
        }
    }
}

void mouseButtonCallback(int button, int action)
{
	if( gEngine->isMaster() )
	{
		switch( button )
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			int tmpXPos, tmpYPos;
			sgct::Engine::getMousePos( &tmpXPos, &tmpYPos );
			break;
		}
	}
}

