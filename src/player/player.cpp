
#include "ofApp.h"
#include "player.h"

const ofVec3f SCALE = {1.6,1.2,4};

Player::Player()
{
	for(int i = 0; i < sizeof(magnitude)-1; i++) {
		magnitude[i] = 0;
	}
	lunge_cooldown = 0;
	chop_cooldown = 0;
	direction = 0;
}



void Player::setup()
{
	body = dBodyCreate(app->getWorld());
	box.setScale(SCALE.x * 0.01, SCALE.y * 0.01, SCALE.z * 0.01);

	reset();

	dMassSetBox(&mass, 1, SCALE.x, SCALE.y, SCALE.z);
	dBodySetMass(body, &mass);

	//hbox = dCreateBox(app->getSpace(), SCALE.x, SCALE.y, SCALE.z);
	hbox = dCreateCapsule(app->getSpace(), SCALE.y/2, SCALE.z/2);
	dGeomSetBody(hbox, body);
	dGeomSetCategoryBits(hbox, GROUP_COLLIDE);

	htest = dCreateCylinder(app->getSpace(), 2, SCALE.z*2);
	dGeomSetData(htest, (void*)this);
	dGeomSetCategoryBits(htest, GROUP_HITTEST);
}

void Player::reset()
{
	dBodySetPosition(body, 0, 0, 2);
}

// Update the player
void Player::update()
{
	// If the player is moving, set the direction to be facing the directional magnitude
	if(isMoving()) direction = std::round(std::atan2(getMagnitude().x, getMagnitude().y) * 180 / PI);

	// Slowly reset the lunge cooldown if neccessary
	if(lunge_cooldown < 20) lunge_cooldown++;

	// Chop all selected trees upon cooldown
	if(selected_trees.size() > 0) {
		if(chop_cooldown <= 60 - (app->getSpeed() * 20)) {
			chop_cooldown++;
		} else {
			for(Tree* tree : selected_trees) {
				app->playSFX(CHOP);
				tree->chop();
			}
			chop_cooldown = 0;
		}
	} else {
		chop_cooldown = 0;
	}
	selected_trees.clear();

	// Set the ODE rigid body to be rotated acording to the player direction
	dMatrix3 matrix;
	dRFromAxisAndAngle(matrix, 0,0,1,-ofDegToRad(direction));
	dBodySetRotation(body, matrix);
	dMassRotate(&mass, matrix);
	dBodySetAngularVel(body, 0, 0, 0);

	// Add directional force to the ODE rigid body
	dBodyAddForce(body, getMagnitude().x * 160, getMagnitude().y * 160, 0);
	dBodySetLinearDamping(body, 0.18);

	// Update ofNode position for later
	const dReal* pos = dBodyGetPosition(body);
	const dReal* rot = dBodyGetQuaternion(body);
	setGlobalPosition(glm::vec3(pos[0],pos[1],pos[2]));
	setGlobalOrientation(glm::quat(rot[0], rot[1], rot[2], rot[3]));

	// Set the hittest cylinder to be at the feet of the player
	dGeomSetPosition(htest, pos[0], pos[1], SCALE.z/2);

	// Delegate selection to the selected fixture
	if(selected) selected->select();
	selected = nullptr;

	// Update where the wood joins with the player, incase of rotation
	if(wood) {
		dBodySetPosition(wood->getBody(),
			getPosition().x + cos(ofDegToRad(-direction+90)),
			getPosition().y + sin(ofDegToRad(-direction+90)),
			getPosition().z);
		dJointSetFixed(grabJoint);
	}
}

// ofNode draw method
void Player::customDraw()
{
	ofSetColor(ofColor::fromHex(0x5f9ea0));
	box.draw();

#ifdef DEBUG
	// Draw various debugging info if using DEBUG build
	ofPushStyle();
	ofSetColor(ofColor::red);
	ofDrawBitmapString(
		to_string(direction) + '\n' +
		to_string(getPosition().x)+", "+to_string(getPosition().y) + '\n' +
		to_string(chop_cooldown) + '\n' +
		to_string(getMagnitude().x)+", "+to_string(getMagnitude().y),
	0, 0, 6);

	// Draw the wireframe of the hitbox
	ofSetColor(ofColor::green);
	ofCylinderPrimitive c;
	c.rotateDeg(90, ofVec3f(1,0,0));
	c.setResolution(10,1);
	c.setScale(.04, SCALE.z * .02, .04);
	c.drawWireframe();

	ofPopStyle();
#endif
}

// Safely destroy all ODE objects
void Player::exit()
{
	dGeomDestroy(hbox);
	dGeomDestroy(htest);
	dBodyDestroy(body);
}



// Check all the WASD magnitudes to see if the player should be moving
bool Player::isMoving()
{
	return magnitude[0] != 0.0f
		|| magnitude[1] != 0.0f
		|| magnitude[2] != 0.0f
		|| magnitude[3] != 0.0f;
}

// Set the WASD key magnitude (either 0 or 1 in this implementation)
void Player::setMagnitude(int i, bool v)
{
	magnitude[i] = v;
}

// Convert the WASD input to a directional magnitude
ofVec2f Player::getMagnitude()
{
	float xAxis = magnitude[3] - magnitude[1];
	float yAxis = magnitude[0] - magnitude[2];

	// Diagnonal input on a discrete grid is slightly faster. This counters that effect
	if((magnitude[0] || magnitude[2]) && (magnitude[1] || magnitude[3])) {
		xAxis *= .8;
		yAxis *= .8;
	}

	return { xAxis, yAxis };
}

// Lunge the player forward in the direction they are going
void Player::lunge()
{
	if(lunge_cooldown >= 20) {
		dBodyAddForce(body, (getMagnitude().x) * 2400, (getMagnitude().y) * 2400, 0);
		lunge_cooldown = 0;
	}
}



// Attempt to grab the selected wood
void Player::grab()
{
	Wood* w = dynamic_cast<Wood*>(selected);
	if(w) {
		wood = w;

		grabJoint = dJointCreateFixed(app->getWorld(), 0);
		dJointAttach(grabJoint, body, wood->getBody());
		dBodySetPosition(wood->getBody(),
			getPosition().x + cos(ofDegToRad(-direction+90)),
			getPosition().y + sin(ofDegToRad(-direction+90)),
			getPosition().z);
		dJointSetFixed(grabJoint);

		dSpaceRemove(app->getSpace(), wood->getHbox());
		selected = nullptr;

		app->playSFX(GRAB);
	}
}

// Attempt to drop the held wood
void Player::drop()
{
	if(wood) {
		dJointDisable(grabJoint);
		dSpaceAdd(app->getSpace(), wood->getHbox());
		wood = nullptr;
	}
}

// Attempt to place the grabbed wood into the selected track
void Player::place()
{
	Track* t = dynamic_cast<Track*>(selected);
	if(t) {
		t->addWood();
		dJointDestroy(grabJoint);
		app->dropFixture(wood);
		wood = nullptr;
		app->playSFX(PLACE);
	}
}



// Player gets
fixture* Player::getSelected() {return selected; }
Wood* Player::getGrabbed() {return wood;}
int Player::getChopCooldown() {return chop_cooldown;}
int Player::getLungeCooldown() {return lunge_cooldown;}



// Logic for what object should be selected
void Player::select(fixture* f)
{
	if(dynamic_cast<Tree*>(f)) {
		selected_trees.push_back((Tree*)f);
	} else {
		Track* t = dynamic_cast<Track*>(f);
		if((getGrabbed() && t && !t->hasWood()) || (getGrabbed() == nullptr && dynamic_cast<Wood*>(f))) {
			if(!selected) selected = f;
			else {
				ofVec3f sel_pos = selected->getPosition();
				ofVec3f new_pos = f->getPosition();
				ofVec3f player_pos = getPosition();

				if(player_pos.distance(new_pos) < player_pos.distance(sel_pos)) {
					selected = f;
				}
			}
		}
	}
}
