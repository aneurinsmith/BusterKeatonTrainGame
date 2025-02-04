
#pragma once
#include "ofMain.h"
#include "ode/ode.h"
#include "ofxAssimpModelLoader.h"

class Train : public ofNode {

public:
	Train();

	void setup();
	void update();
	void customDraw();
	void exit();

	void setAlert(bool = true);
	bool is_alerted();

	float getHtestPos();

private:
	bool _is_alerted;

	ofSoundPlayer whistle_sfx;
	ofxAssimpModelLoader model;

	dGeomID hbox;
	dGeomID htest;

};
