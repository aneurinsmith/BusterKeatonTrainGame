
#pragma once
#include "fixture.h"

class Tree : public fixture {

public:
	Tree();

	void customUpdate();
	void customDraw();

	void chop();

	int getHealth();

private:
	ofBoxPrimitive box;

	int health;
	ofColor col;

};
