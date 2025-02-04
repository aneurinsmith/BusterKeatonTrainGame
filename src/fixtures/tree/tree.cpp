
#include "ofApp.h"
#include "tree.h"

// Tree constructor. I've set the initial tree variables here
Tree::Tree()
{
	health = 3;
	setPosition(glm::vec3(36+((int)(app->getDistance()/10)/2)*2, ((rand()%17)-8) * 2, 1));

	hbox = dCreateBox(app->getSpace(), 2, 2, 2);
	dGeomSetPosition(hbox, getPosition().x, getPosition().y, getPosition().z);
	dGeomSetData(hbox, (void*)this);
	dGeomSetCategoryBits(hbox, GROUP_COLLIDE | GROUP_RECIEVE);
}



// fixture update method
void Tree::customUpdate()
{
	// Update color if its selected
	if(isSelected) col = ofColor::fromHex(0x3eda3e);
	else col = ofColor::fromHex(0x20AA20);
}

// ofNode draw method
void Tree::customDraw()
{
	// Update 'box' scale according to health
	box.setScale(.02 * (health/3.0f),.02 * (health/3.0f),.02);

	ofSetColor(col);
	box.draw();

#ifdef DEBUG
	// Draw various debugging info if using DEBUG build
	ofPushStyle();
	ofSetColor(ofColor::fromHex(0xff0000));
	ofDrawBitmapString(to_string(getPosition().x)+", "+to_string(getPosition().y), 0, 0, 0);

	// Draw the wireframe of the hitbox
	ofNoFill();
	ofSetColor(ofColor::green);
	ofDrawBox(0, 0, 0, 2, 2, 2);
	ofFill();

	ofPopStyle();
#endif
}



// Reduce health of the tree, or destroy if health is 0
void Tree::chop()
{
	health--;

	if(health < 1) {
		// If the train is about to derail, give extra points for the clutch move
		int multiplyer = 1 + (app->getDistance()*0.005);
		if(getPosition().y == 0 && app->getTrain().is_alerted()) multiplyer *= 2;
		app->addScore(10, multiplyer, true, ofVec2f(getPosition().x, getPosition().y));
		app->pushFixture(new Wood(getPosition().x, getPosition().y));
		app->dropFixture(this);
		app->playSFX(FELL);
	}
}

// Tree get
int Tree::getHealth(){return health;}
