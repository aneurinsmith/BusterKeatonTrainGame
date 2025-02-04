
#pragma once
#include "fixture.h"

class Wood : public fixture {

public:
	Wood(float = 0, float = 0);
	Wood(glm::vec2 pos);
	Wood(ofVec2f);
	~Wood();

	void customUpdate();
	void customDraw();

	dBodyID getBody();

private:
	ofBoxPrimitive box;

	dBodyID body;
	dMass mass;
	ofColor col;

};
