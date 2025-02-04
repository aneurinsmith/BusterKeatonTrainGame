
#include "ofApp.h"
#include "train.h"

// Train constructor
Train::Train()
{
	whistle_sfx.load("sfx/whistle.wav");
}



// Initial train setup
void Train::setup()
{
	model.loadModel("train.obj", true);
	model.setRotation(0, 90, 1, 0, 0);
	model.setScale(-.018,.018,-.018);

	hbox = dCreateBox(app->getSpace(), 10, 2, 2);
	dGeomSetCategoryBits(hbox, GROUP_COLLIDE);

	htest = dCreateBox(app->getSpace(), 60, 1, 2);
	dGeomSetData(htest, (void*)this);
	dGeomSetCategoryBits(htest, GROUP_HITTEST);
}

// Update train position, based on distance travelled
void Train::update()
{
	dGeomSetPosition(hbox, getPosition().x, getPosition().y, 1);
	dGeomSetPosition(htest, -25+getPosition().x+(app->getSpeed()*30), 0, 0);
	setGlobalPosition(-30+app->getDistance()/10,0,.6);
}

// ofNode draw method
void Train::customDraw()
{
	model.drawFaces();

	ofQuaternion rot;
	rot = getOrientationQuat();
	rot.makeRotate((int)app->getDistance()*10 % 360, ofVec3f(0,1,0));

	ofSetColor(ofColor::lightGray);
	ofCylinderPrimitive c;
	c.setResolution(8,1);
	c.setScale(.01,.002,.01);
	c.setGlobalOrientation(rot);

	c.setPosition(3.2,-.75,0.3); c.draw();
	c.setPosition(2,-.75,0.3); c.draw();
	c.setPosition(0.4,-.75,0.3); c.draw();
	c.setPosition(-2.2,-.75,0.3); c.draw();
	c.setPosition(-4,-.75,0.3); c.draw();

	if(is_alerted()) {
		ofPushMatrix();
		ofSetColor(ofColor::red);
		ofTranslate(4,0,6);
		ofDrawBox(1);
		if(!whistle_sfx.isPlaying()) {
			whistle_sfx.play();
		}
		ofPopMatrix();
	} else {
		whistle_sfx.stop();
	}
	//setAlert(false);

#ifdef DEBUG
	// Draw hitboxes if using DEBUG build
	ofNoFill();
	ofSetColor(ofColor::blue);
	ofDrawBox(-25+(app->getSpeed()*30), 0, 0, 60, 1, 2);
	ofSetColor(ofColor::green);
	ofDrawBox(0, 0, 1, 10, 2, 2);
	ofFill();
#endif
}

// Safely destroy ODE objects
void Train::exit()
{
	dGeomDestroy(hbox);
	dGeomDestroy(htest);
}



void Train::setAlert(bool alerted){_is_alerted = alerted;}
bool Train::is_alerted() {return _is_alerted;}

float Train::getHtestPos()
{
	return 5+getPosition().x+(app->getSpeed()*30);
}
