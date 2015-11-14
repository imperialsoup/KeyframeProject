#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "Skeleton.h"
#include "Skin.h"
#include "Animation.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace gl;
using namespace ci::app;
using namespace std;

class KeyframeProjectApp : public App {
public:

	void setup();
	void mouseDown(MouseEvent event);
	void update();
	void draw();
	void drawAxis();

	Animation animation;
	Skeleton skeleton;
	Skin skin;

private:
	params::InterfaceGlRef	mParams;
	CameraPersp	mCam;
	vec3 mEye;
	vec3 mCenter;
	vec3 mUp;

	vec3  * mpose;
	quat	camRotation;
	float	camDistance;

};

void KeyframeProjectApp::setup()
{
	//setup the camera
	camDistance = 10.0f;
	mEye = vec3(camDistance, camDistance, camDistance);
	mCenter = vec3(0.0);
	mUp = vec3(0.0, 1.0, 0.0);;
	mCam.setPerspective(90.0f, getWindowAspectRatio(), 0.5f, 2000.0f);
	mCam.lookAt(mEye, mCenter, mUp);

	//setup the camera tweak bar
	mParams = params::InterfaceGl::create(getWindow(), "Cam Params", toPixels(vec2(200, 200)));
	mParams->addParam("Scene Rotation", &camRotation, "opened=1");
	mParams->addParam("Zoom", &camDistance, "min=1.0");

	//setup skeleton
	skeleton.init("C:/Users/Marie/Documents/Fall_2015/CS_4392_Animation/assignments/HW3/tube.skel");

	//setup skin
	skin.init("C:/Users/Marie/Documents/Fall_2015/CS_4392_Animation/assignments/HW3/tube.skin", &skeleton);

	//setup animation 
	animation.init("C:/Users/Marie/Documents/Fall_2015/CS_4392_Animation/assignments/HW4/wasp_walk.anim");

	skeleton.depthSearch(&skeleton.root);
	//skeleton.doThing();
	console() << "Finished setting up!" << endl;
}

void KeyframeProjectApp::mouseDown(MouseEvent event)
{
}

void KeyframeProjectApp::update(){
	mEye = vec3(0.0f, 0.0f, camDistance);
	mCam.lookAt(mEye, vec3(0.0));
	skeleton.update();
	skin.update();
	console() << ci::app::getElapsedSeconds() * 1000;
	}

void KeyframeProjectApp::drawAxis()
{
	double length = 200;

	/*X : blue*/
	gl::color(0, 0, 255);
	gl::drawLine(vec3(0.0), vec3(length, 0, 0));

	/*Y : green*/
	gl::color(0, 255, 0);
	gl::drawLine(vec3(0.0), vec3(0, length, 0));

	/*Z : red*/
	gl::color(255, 0, 0);
	gl::drawLine(vec3(0.0), vec3(0, 0, length));

}

void KeyframeProjectApp::draw()
{
	clear(Color(0, 0, 0));
	pushMatrices();
	setMatrices(mCam);
	rotate(camRotation);

	//ScopedModelMatrix scopedMatrix;

	drawAxis();


	skeleton.draw();
	skin.draw();

	popMatrices();
	mParams->draw();
}

CINDER_APP(KeyframeProjectApp, RendererGl)

