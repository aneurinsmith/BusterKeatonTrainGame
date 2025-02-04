
#pragma once
#include "ofMain.h"
#include "ode/ode.h"

#include "fixture.h"
#include "wood.h"
#include "tree.h"

class Player : public ofNode
{
public:
	Player();

	void setup();
	void reset();
	void update();
	void customDraw();
	void exit();

	bool isMoving();
	void setMagnitude(int, bool);
	ofVec2f getMagnitude();
	void lunge();

	void grab();
	void drop();
	void place();

	fixture* getSelected();
	Wood* getGrabbed();
	int getChopCooldown();
	int getLungeCooldown();

	void select(fixture*);

private:
	ofBoxPrimitive box;

	dBodyID body;
	dGeomID hbox;
	dGeomID htest;
	dMass mass;

	dJointID grabJoint;

	float magnitude[4];
	float direction;

	int chop_cooldown;
	int lunge_cooldown;

	Wood* wood = nullptr;
	vector<Tree*> selected_trees;
	fixture* selected = nullptr;
};
