#include "sgct.h"

#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Types.h"
#include "DrawableObject.h"
#include "Entity3D.h"

//#define MOUSE
#define WAND

#define WANDBUTTONS 6

sgct::Engine * gEngine;

DrawableObject * sphere = NULL;

Entity3D earth, moon, mars, milkyway, wand;

enum navigationmode { GAZE=0, CROSSHAIR};

f32 moveSpeed = 0.0f;

u8 navigation = GAZE;

bool accelerate = false;

vec3 headDirection(0,0,-1);
vec3 headOrientation(0,0,0);
vec3 headPosition(0,0,4);

vec3 wandDirection(0,0,-1);
bool wandButtons[WANDBUTTONS] = {false,false,false,false,false,false};

mat4 wandMatrix;
/**
0,1,2,3 front,
4 analogue
5 back/shooot?
**/

#define BUTTON_ROTATE 0
#define BUTTON_TRANSLATE 1
#define BUTTON_SCALE 2
#define BUTTON_MOVE 4
#define BUTTON_SELECT 5


u32 milkywayTexture = 0;
u32 earthTexture = 0;
u32 marsTexture = 0;
u32 moonTexture = 0;

enum modifiers { TRANSLATE=0, ROTATE, SCALE };
u8 modifier = TRANSLATE;

bool mouseDown[3];
enum mousebuttons { MOUSELEFT = 0, MOUSEMIDDLE, MOUSERIGHT };
ivec2 mousePosition;

Entity3D *chosenEntity = NULL;

//variables to share across cluster
double curr_time = 0.0;
mat4 transform;

//callbacks
void myInitOGLFun();
void myPreSyncFun();
void myDrawFun();
void myEncodeFun();
void myDecodeFun();

//input callbacks
void keyCallback(int key, int action);
void mouseButtonCallback(int button, int action);

//helper function
void getSelectedEntity();

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

f32 intersectSphere(vec3 sp, f32 radius)
{
    vec3 wd = glm::normalize(wandDirection);
    vec3 wp = wand.getPosition();

    vec3 v = mat3(transform)*sp - wp;

    f32 t = glm::dot(v,wd);

    vec3 li = wp + wd*t;

    vec3 dv = sp - li;

    if(glm::dot(dv,dv) < radius*radius)
        return t;
    else
        return MAXFLOAT;
}

void getSelectedEntity()
{
    f32 nearestHit = MAXFLOAT;

    f32 hit = intersectSphere(moon.getPosition(),moon.getScale());
    if(hit < nearestHit)
    {
        chosenEntity = &moon;
        nearestHit = hit;
    }

    hit = intersectSphere(earth.getPosition(),earth.getScale());
    if(hit < nearestHit)
    {
        chosenEntity = &earth;
        nearestHit = hit;
    }

    hit = intersectSphere(mars.getPosition(),mars.getScale());
    if(hit < nearestHit)
    {
        chosenEntity = &mars;
        nearestHit = hit;
    }
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

    earth.setDrawable(sphere);
    earth.setPosition(0,0,0);
    earth.setScale(0.3);
	earth.setTexture( sgct::TextureManager::Instance()->getTextureByIndex(earthTexture) );

    moon.setDrawable(sphere);
    moon.setPosition(-0.5,0,0);
    moon.setScale(0.075);
	moon.setTexture( sgct::TextureManager::Instance()->getTextureByIndex(moonTexture) );

    mars.setDrawable(sphere);
    mars.setPosition(0.5,0,0);
    mars.setScale(0.15);
	mars.setTexture( sgct::TextureManager::Instance()->getTextureByIndex(marsTexture) );

    milkyway.setDrawable(sphere);
    milkyway.setPosition(0,0,0);
    milkyway.setScale(10.0);
	milkyway.setTexture( sgct::TextureManager::Instance()->getTextureByIndex(milkywayTexture) );

	wand.setDrawable(sphere);
	wand.setPosition(0.2,0,3);
	wand.setOrientationXYZ(0,90,0);
	wand.setScale(0.025);
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
        {
            sgct::Engine::getMousePos( &mousePosition.x, &mousePosition.y );

            f32 deadzone = 0.009f;
            static ivec2 startPosition;
            vec2 amount;

            if(mouseDown[MOUSELEFT] || mouseDown[MOUSEMIDDLE] || mouseDown[MOUSERIGHT])
                amount = (vec2)(startPosition-mousePosition)*0.0009f;
            else
            {
                startPosition = mousePosition;
            }

            if(mouseDown[MOUSELEFT])
            {
                if(accelerate)
                {
                    static f32 acc = 0;
                    acc += glm::abs(amount.y) > deadzone ? amount.y-glm::sign(amount.y)*deadzone : 0.0f;
                    acc *= 0.1f;
                    moveSpeed += acc;
                }
                else
                    moveSpeed = glm::abs(amount.y) > deadzone ? amount.y-glm::sign(amount.y)*deadzone : 0.0f;
            }
            else
                moveSpeed = 0.0f;

            if(mouseDown[MOUSERIGHT])
            {
                switch(modifier)
                {
                    case TRANSLATE:
                    {
                        vec3 planeNormal = -glm::normalize(headDirection);
                        vec3 planeUp(0,1,0);
                        vec3 planeRight = glm::cross(planeUp,planeNormal);
                        translateMod = -amount.x * planeRight + amount.y * planeUp;
                    }
                        break;

                    case ROTATE:
                        rotateMod = vec3(amount.y,-amount.x,0)*0.1f;
                        break;

                    case SCALE:
                        scaleMod = (glm::abs(amount.y) > deadzone ? amount.y-glm::sign(amount.y)*deadzone : 0.0f)*3.0f;
                        break;
                }
            }
        }
        #endif
        #ifdef WAND
        {
            for(size_t i = 0; i < sgct::Engine::getTrackingManager()->getNumberOfTrackers(); i++)
            {
                sgct::SGCTTracker *trackerPtr = sgct::Engine::getTrackingManager()->getTrackerPtr(i);
                for(size_t j = 0; j < trackerPtr->getNumberOfDevices(); j++)
                {
                    sgct::SGCTTrackingDevice *devicePtr = trackerPtr->getDevicePtr(j);

                    if(devicePtr == sgct::Engine::getTrackingManager()->getHeadDevicePtr())
                    {
                        headPosition = vec3(devicePtr->getPosition());
                        headOrientation = vec3(devicePtr->getEulerAngles()) * PI/180.0f;
                        headDirection = mat3(glm::yawPitchRoll(headOrientation.y,headOrientation.x,headOrientation.z))*vec3(0,0,-1);
                    }
                    else
                    {
                        if( devicePtr->hasSensor() )
                        {
                            wand.setPosition(vec3(devicePtr->getPosition()));
                            vec3 angles = vec3(devicePtr->getEulerAngles()) * PI/180.0f;
                            wand.setOrientationXYZ(angles.x,angles.y,angles.z);
                            wandMatrix = mat4(devicePtr->getTransformMat());

                            wandDirection = mat3(wandMatrix) * vec3(0,0,-1);
                        }
                        if( devicePtr->hasButtons() )
                        {
                            for(size_t k=0; k < WANDBUTTONS; k++)
                            {
                                wandButtons[k] = devicePtr->getButton(k);
                            }
                        }
                    }
                }
            }

            f32 deadzone = 0.01f;
            static vec3 startPosition;
            vec3 amount;

            if(wandButtons[BUTTON_TRANSLATE] || wandButtons[BUTTON_ROTATE] || wandButtons[BUTTON_SCALE] || wandButtons[BUTTON_MOVE])
                amount = wand.getPosition() - startPosition;
            else
                startPosition = wand.getPosition();

            if(wandButtons[BUTTON_TRANSLATE])
            {
                translateMod = amount;
            }
            if(wandButtons[BUTTON_ROTATE])
            {
                rotateMod = amount;
            }
            if(wandButtons[BUTTON_SCALE])
            {
                scaleMod = (glm::abs(amount.y) > deadzone ? amount.y - glm::sign(amount.y)*deadzone : 0.0f)*0.5f;
            }

            static f32 acc = 0;

            if(wandButtons[BUTTON_MOVE])
            {
                if(accelerate)
                {
                    acc += (glm::abs(amount.z) > deadzone ? amount.z - glm::sign(amount.z)*deadzone : 0.0f)*0.001f;
                    //acc *= 0.1f;
                    moveSpeed += acc;
                }
                else
                {
                    moveSpeed = amount.z * .5f;
                }
            }
            else
            {
                moveSpeed = 0.0f;
                acc = 0;
            }

            if(wandButtons[BUTTON_SELECT])
            {
                getSelectedEntity();
            }
        }
        #endif

        /** modify chosenEntity with modifiers **/
        if(chosenEntity)
        {
            chosenEntity->translate(translateMod);
            chosenEntity->rotate(rotateMod);
            chosenEntity->setScale(chosenEntity->getScale()+scaleMod);
        }

        /** setup transform matrix **/
        vec3 translation;
        if(navigation == GAZE)
            translation = glm::normalize(headDirection);
        else
            translation = glm::normalize(wand.getPosition() - headPosition);

        transform = glm::translate( transform, -translation*moveSpeed );

        /** setup entities **/

        /** earth **/
        if(chosenEntity==&earth)
            earth.setColor(1,1,1);
        else
            earth.setColor(0.7,0.7,0.7);
        earth.rotate(0.0,0.001,0.0);

        /** moon **/
        if(chosenEntity==&moon)
            moon.setColor(1,1,1);
        else
            moon.setColor(0.7,0.7,0.7);
        moon.rotate(0.001,0.001,0.0);

        /** mars **/
        if(chosenEntity==&mars)
            mars.setColor(1,1,1);
        else
            mars.setColor(0.7,0.7,0.7);
        mars.rotate(0.0,-0.001,0.001);
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

    Freetype::print(sgct::FontManager::Instance()->GetFont( "SGCTFont", 10 ), 10.0f, 58.0f,
        "TranslateMode: %s", (accelerate == true) ? "Acceleration" : "Velocity");

    for(int i=0; i<6; ++i)
    {
        Freetype::print(sgct::FontManager::Instance()->GetFont( "SGCTFont", 10 ), 10.0f, 70.0f+12.0f*i,
            "Button%i: %i", i, wandButtons[i]);
    }
}

void myDrawFun()
{
    /** Apply transforms for navigation and draw the scene **/
    glEnable(GL_TEXTURE_2D);

    milkyway.draw();

    glClear(GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

        glMultMatrixf(glm::value_ptr(transform));

        /** EARTH **/
        earth.draw();

        /** MOON **/
        moon.draw();

        /** MARS **/
        mars.draw();

    glPopMatrix();

    /** WAND BODY **/

    wand.draw();

    /** WAND DIRECTION **/
    glColor3f(1.0f,1.0f,0.0f);
    glPushMatrix();
        glLineWidth(2.0);
        glMultMatrixf(glm::value_ptr(wandMatrix));
        glBegin(GL_LINES);
            glVertex3f(0,0,0);
            glVertex3f(0,0,-20);
        glEnd();
    glPopMatrix();

    glColor3f(1.0f,1.0f,1.0f);

    drawText();
}

void myEncodeFun()
{
    sgct::SharedData::Instance()->writeDouble( curr_time );
    sgct::SharedData::Instance()->writeUChar( navigation );

	for(int i=0; i<16; i++)
		sgct::SharedData::Instance()->writeFloat( glm::value_ptr(transform)[i] );

    earth.writeData();
    moon.writeData();
    mars.writeData();
    wand.writeData();
}

void myDecodeFun()
{
    curr_time = sgct::SharedData::Instance()->readDouble();
    navigation = sgct::SharedData::Instance()->readUChar();

	for(int i=0; i<16; i++)
		glm::value_ptr(transform)[i] = sgct::SharedData::Instance()->readFloat();

    earth.getData();
    moon.getData();
    mars.getData();
    wand.getData();
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
                    chosenEntity = &moon;
                break;

    		case '2':
                if(action == GLFW_PRESS)
                    chosenEntity = &earth;
                break;

            case '3':
                if(action == GLFW_PRESS)
                    chosenEntity = &mars;
                break;

            case '0':
                if(action == GLFW_PRESS)
                    chosenEntity = NULL;
                break;

            case GLFW_KEY_LCTRL:
                if(action == GLFW_PRESS)
                    accelerate = !accelerate;
                break;

            case GLFW_KEY_TAB:
                if(action == GLFW_PRESS)
                    navigation = !navigation;
                break;

            case GLFW_KEY_ENTER:
                if(action == GLFW_PRESS)
                    getSelectedEntity();
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

