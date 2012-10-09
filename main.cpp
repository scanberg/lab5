#include "sgct.h"

#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Types.h"
#include "DrawableObject.h"

#define MOUSE

sgct::Engine * gEngine;

DrawableObject * sphere = NULL;

enum navigationmode { GAZE=0, CROSSHAIR};

f32 moveSpeed = 0.0f;

u8 navigation = GAZE;

vec3 headDirection(0,0,-1);
vec3 headPosition(0,0,4);

vec3 wandDirection(0,0,-1);
vec3 wandPosition(1,0,3);
bool wandButtons[6] = {false,false,false,false,false,false};

u32 milkywayTexture = 0;
u32 earthTexture = 0;
u32 marsTexture = 0;
u32 moonTexture = 0;

enum modifiers { TRANSLATE=0, ROTATE, SCALE };
u8 modifier = TRANSLATE;

bool mouseDown[3];
enum mousebuttons { MOUSELEFT = 0, MOUSEMIDDLE, MOUSERIGHT };
ivec2 mousePosition;

enum picked{ NONE = 0, MOON, EARTH, MARS };

//variables to share across cluster
u8 chosen = NONE;
double curr_time = 0.0;

mat4 transform;
mat4 earthTransform;
mat4 moonTransform;
mat4 marsTransform;

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

    sgct::TextureManager::Instance()->loadTexure(milkywayTexture, "milkyway", "media/milkyway.png", true);
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

        vec3 translateMod(0.0f);
        vec3 rotateMod(0.0f);
        f32 scaleMod = 0.0f;

        #ifdef MOUSE
            sgct::Engine::getMousePos( &mousePosition.x, &mousePosition.y );

            f32 deadzone = 0.009f;
            static ivec2 startPosition;
            vec2 amount;

            if(mouseDown[MOUSELEFT] || mouseDown[MOUSEMIDDLE] || mouseDown[MOUSERIGHT])
                amount = (vec2)(startPosition-mousePosition)*0.0005f;
            else
                startPosition = mousePosition;

            if(mouseDown[MOUSELEFT])
                moveSpeed = glm::abs(amount.y) > deadzone ? amount.y-glm::sign(amount.y)*deadzone : 0.0f;
            else
                moveSpeed = 0.0f;

            if(mouseDown[MOUSERIGHT])
            {
                switch(modifier)
                {
                    /*case TRANSLATE:
                        vec3 planeNormal = -headDirection;
                        vec3 planeUp(0,1,0);
                        vec3 planeRight = glm::cross(planeUp,planeNormal);
                        translateMod = amount.x * planeRight + amount.y * planeUp;
                        break;
                    */
                    case ROTATE:
                        rotateMod = vec3(amount.y,amount.x,0);
                        break;

                    case SCALE:
                        scaleMod = glm::abs(amount.y) > deadzone ? amount.y-glm::sign(amount.y)*deadzone : 0.0f;
                        break;
                }
            }
        #else
            /** Wand! **/
        #endif

        vec3 translation;
        if(navigation == GAZE)
            translation = glm::normalize(headDirection);
        else
            translation = glm::normalize(wandPosition - headPosition);

        transform = glm::translate( transform, -translation*moveSpeed );

        earthTransform = glm::translate(mat4(1.0f), vec3(0.0,0.0,0.0)+translateMod*(float)(chosen==EARTH));
        earthTransform = glm::scale(earthTransform, vec3(1.0f)+vec3(scaleMod)*(float)(chosen==EARTH));
        earthTransform = glm::rotate(earthTransform, 10.0f*(float)curr_time, vec3(0,1,0));

        moonTransform = glm::translate(mat4(1.0f), vec3(-1.2,0.0,0.0));
        moonTransform = glm::scale(moonTransform, vec3(0.26f));

        marsTransform = glm::translate(mat4(1.0f), vec3(1.2,0.0,0.0));
        marsTransform = glm::scale(marsTransform, vec3(0.5f));
    }
}

void drawText()
{
    Freetype::print(sgct::FontManager::Instance()->GetFont( "SGCTFont", 10 ), 10.0f, 10.0f,
        "Navigation-mode: %s", navigation==GAZE ? "gaze" : "crosshair");

    Freetype::print(sgct::FontManager::Instance()->GetFont( "SGCTFont", 10 ), 10.0f, 22.0f,
        "Modifier: %s", (modifier==0) ? "translate" : (modifier==1) ? "rotate" : "scale");

    Freetype::print(sgct::FontManager::Instance()->GetFont( "SGCTFont", 10 ), 10.0f, 34.0f,
        "MoveSpeed: %f", moveSpeed);

    Freetype::print(sgct::FontManager::Instance()->GetFont( "SGCTFont", 10 ), 10.0f, 46.0f,
        "MousePos: %i, %i", mousePosition.x, mousePosition.y);
}

void myDrawFun()
{
    /** Apply transforms for navigation and draw the scene **/
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
        glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::Instance()->getTextureByIndex(milkywayTexture) );
        glColor3f(0.5f,0.5f,0.5f);
        glScalef(10.0f,10.0f,10.0f);
        sphere->draw();
    glPopMatrix();

    glClear(GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

        glMultMatrixf(glm::value_ptr(transform));

        /** EARTH **/
        glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::Instance()->getTextureByIndex(earthTexture) );
        glPushMatrix();

            glMultMatrixf(glm::value_ptr(earthTransform));
            if(chosen == EARTH)
                glColor3f(1.0f,1.0f,1.0f);
            else
                glColor3f(0.7f,0.7f,0.7f);
            sphere->draw();
        glPopMatrix();

        /** MOON **/
        glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::Instance()->getTextureByIndex(moonTexture) );
        glPushMatrix();
            glMultMatrixf(glm::value_ptr(moonTransform));
            if(chosen == MOON)
                glColor3f(1.0f,1.0f,1.0f);
            else
                glColor3f(0.7f,0.7f,0.7f);
            sphere->draw();
        glPopMatrix();

        /** MARS **/
        glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::Instance()->getTextureByIndex(marsTexture) );
        glPushMatrix();
            glMultMatrixf(glm::value_ptr(marsTransform));
            if(chosen == MARS)
                glColor3f(1.0f,1.0f,1.0f);
            else
                glColor3f(0.7f,0.7f,0.7f);
            sphere->draw();
        glPopMatrix();

    glPopMatrix();

    glColor3f(1.0f,1.0f,1.0f);

    drawText();
}

void myEncodeFun()
{
    sgct::SharedData::Instance()->writeDouble( curr_time );
    sgct::SharedData::Instance()->writeUChar( chosen );
    sgct::SharedData::Instance()->writeUChar( navigation );

	for(int i=0; i<16; i++)
		sgct::SharedData::Instance()->writeFloat( glm::value_ptr(transform)[i] );

    for(int i=0; i<16; i++)
        sgct::SharedData::Instance()->writeFloat( glm::value_ptr(earthTransform)[i] );

    for(int i=0; i<16; i++)
        sgct::SharedData::Instance()->writeFloat( glm::value_ptr(moonTransform)[i] );

    for(int i=0; i<16; i++)
        sgct::SharedData::Instance()->writeFloat( glm::value_ptr(marsTransform)[i] );
}

void myDecodeFun()
{
    curr_time = sgct::SharedData::Instance()->readDouble();
    chosen = sgct::SharedData::Instance()->readUChar();
    navigation = sgct::SharedData::Instance()->readUChar();

	for(int i=0; i<16; i++)
		glm::value_ptr(transform)[i] = sgct::SharedData::Instance()->readFloat();

	for(int i=0; i<16; i++)
		glm::value_ptr(earthTransform)[i] = sgct::SharedData::Instance()->readFloat();

	for(int i=0; i<16; i++)
		glm::value_ptr(moonTransform)[i] = sgct::SharedData::Instance()->readFloat();

	for(int i=0; i<16; i++)
		glm::value_ptr(marsTransform)[i] = sgct::SharedData::Instance()->readFloat();
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
                    chosen = MOON;
                break;

    		case '2':
                if(action == GLFW_PRESS)
                    chosen = EARTH;
                break;

            case '3':
                if(action == GLFW_PRESS)
                    chosen = MARS;
                break;

            case '0':
                if(action == GLFW_PRESS)
                    chosen = NONE;
                break;

            case GLFW_KEY_TAB:
                if(action == GLFW_PRESS)
                    navigation = !navigation;
                break;

            case 'W':
                if(action == GLFW_PRESS)
                    modifier = TRANSLATE;
                break;

            case 'E':
                if(action == GLFW_PRESS)
                    modifier = ROTATE;
                break;

            case 'R':
                if(action == GLFW_PRESS)
                    modifier = SCALE;
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
            mouseDown[MOUSELEFT] = (action == GLFW_PRESS ? true : false);
			break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            mouseDown[MOUSEMIDDLE] = (action == GLFW_PRESS ? true : false);
			break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            mouseDown[MOUSERIGHT] = (action == GLFW_PRESS ? true : false);
			break;
		}
	}
}

