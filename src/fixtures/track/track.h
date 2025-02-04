
#pragma once
#include "fixture.h"

class Track : public fixture {

public:
	Track(int);

	void customUpdate();
	void customDraw();

	bool hasWood();
	void addWood();

private:
	bool _hasWood;
	int alpha;

	ofColor col;

};
