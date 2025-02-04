
#pragma once
#include "ofMain.h"
#include "ode/ode.h"

// Parent class for all fixtures.
class fixture : public ofNode {

public:

	~fixture() {
		dGeomDestroy(hbox);
	}

	void select() {
		isSelected = true;
	}
	void update() {
		customUpdate();
		isSelected = false;
	}

	virtual void customUpdate() = 0;

	dGeomID getHbox() {return hbox;}

protected:
	bool isSelected;
	dGeomID hbox;

};
