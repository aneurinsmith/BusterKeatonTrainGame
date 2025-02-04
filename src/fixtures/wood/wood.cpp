
#include "ofApp.h"
#include "wood.h"

// Wood constructor. I've set the initial wood variables here
Wood::Wood(float x, float y)
{
	setPosition(x,y,2);
	box.setScale(.01,.01,.01);

	body = dBodyCreate(app->getWorld());
	dBodySetPosition(body, x, y, 4);

	dMassSetBox(&mass, .1, 1, 1, 1);
	dBodySetMass(body, &mass);

	hbox = dCreateBox(app->getSpace(), 1, 1, 1);
	dGeomSetBody(hbox, body);
	dGeomSetData(hbox, (void*)this);
	dGeomSetCategoryBits(hbox, GROUP_COLLIDE | GROUP_RECIEVE);
}

// Wood constructor overloads
Wood::Wood(glm::vec2 pos) : Wood(pos.x, pos.y) {};
Wood::Wood(ofVec2f pos) : Wood(pos.x, pos.y) {};

// Automatically destroy body with the fixture
Wood::~Wood()
{
	dBodyDestroy(body);
}



// fixture update method
void Wood::customUpdate()
{
	// Simply update the position of the ofNode to the position of the rigid body
	const dReal* pos = dBodyGetPosition(body);
	const dReal* rot = dBodyGetQuaternion(body);
	setGlobalPosition(glm::vec3(pos[0],pos[1],pos[2]));
	setGlobalOrientation(glm::quat(rot[0],rot[1],rot[2],rot[3]));

	// Update color if its selected
	if(isSelected) col = ofColor::fromHex(0xd19261);
	else col = ofColor::fromHex(0x985B2C);
}

// ofNode draw method
void Wood::customDraw()
{
	ofSetColor(col);
	box.draw();

#ifdef DEBUG
	// Draw various debugging info if using DEBUG build
	ofPushStyle();
	ofSetColor(ofColor::fromHex(0xff0000));
	ofDrawBitmapString(to_string(getPosition().x)+", "+to_string(getPosition().y), 0, 0, 0);
	ofPopStyle();

	// Draw the wireframe of the hitbox
	ofNoFill();
	ofSetColor(ofColor::green);
	ofDrawBox(0, 0, 0, 1, 1, 1);
	ofFill();
#endif
}

// Wood get
dBodyID Wood::getBody() {return body;}
