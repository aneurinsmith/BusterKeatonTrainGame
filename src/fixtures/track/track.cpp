
#include "ofApp.h"
#include "track.h"

// Track constructor. I've set the initial track variables here
Track::Track(int posX)
{
	alpha = 255;
	setPosition(posX, 0, 0);
	hbox = dCreateBox(app->getSpace(), 2, 2, 2);
	dGeomSetPosition(hbox, getPosition().x, getPosition().y, getPosition().z);
	dGeomSetData(hbox, (void*)this);
	dGeomSetCategoryBits(hbox, GROUP_RECIEVE);
	_hasWood = true;
}



// fixture update method
bool prevHasWood = false;
void Track::customUpdate()
{
	// If its too far left, move it all the way to the right, and...
	if(getPosition().x < -36+app->getDistance()/10) {
		setPosition(getPosition().x + 72, 0, 0);
		dGeomSetPosition(hbox, getPosition().x, getPosition().y, getPosition().z);
		// ...add some score, and...
		app->addScore(10,1, false);
		// ...randomly assign wood. Programmed to avoid too many consecutive damaged tracks
		if(prevHasWood) {
			if(rand() % 3 == 0) {
				_hasWood = false;
				prevHasWood = false;
			} else {
				_hasWood = true;
				prevHasWood = true;
			}
		} else {
			if(rand() % 24 == 0) {
				_hasWood = false;
				prevHasWood = false;
			} else {
				_hasWood = true;
				prevHasWood = true;
			}
		}
	}

	// If its selected, the alpha should be slightly more opaque
	if(isSelected) alpha = 80;
	else alpha = 20;
}

// ofNode draw method
void Track::customDraw()
{
	// Set the color/alpha based on params
	if(hasWood()) ofSetColor(ofColor::fromHex(0x964B00), 255);
	else ofSetColor(ofColor::fromHex(0x964B00), alpha);

//	// Draw the wood part
    ofTranslate(-0.5,0,0.1);
    ofDrawBox(0.4,2,0.2);
    ofTranslate(1,0,0.1);
    ofDrawBox(0.4,2,0.2);

	// Draw the metal part
	ofSetColor(ofColor::fromHex(0xD0D0D0));
	ofTranslate(-0.5,-0.7,0.1);
	ofDrawBox(2,0.2,0.4);
	ofTranslate(0,1.4,0);
	ofDrawBox(2,0.2,0.4);
}

bool Track::hasWood() {return _hasWood;}
void Track::addWood()
{
	// If the train is about to derail, give extra points for the clutch move
	int multiplyer = 1 + (app->getDistance()*0.005);
	if(getPosition().x < app->getTrain().getHtestPos()+1 && app->getTrain().is_alerted()) multiplyer *= 2;
	app->addScore(100, multiplyer, true, ofVec2f(getPosition().x, getPosition().y));
	_hasWood = true;
}
