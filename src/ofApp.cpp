#include "ofApp.h"

// Constructor method. All class variables should be assigned here
ofApp::ofApp()
{
	// Initilise class variables
	game_state = STARTUP;
	marker_pos = 0;
	speed = 0.2f;
	distance = 0.0f;
	score = 0;
	high_score = 0;

	// Load sound effects
	fell_sfx = new ofSoundPlayer[4];
	grab_sfx = new ofSoundPlayer[4];
	chop_sfx = new ofSoundPlayer[4];
	place_sfx = new ofSoundPlayer[4];
	for(int i = 1; i <= 4; i++) {
		fell_sfx[i-1].load(string("sfx/fell" + to_string(i) + ".wav"));
		grab_sfx[i-1].load(string("sfx/grab" + to_string(i) + ".wav"));
		chop_sfx[i-1].load(string("sfx/chop" + to_string(i) + ".wav"));
		place_sfx[i-1].load(string("sfx/place" + to_string(i) + ".wav"));
	}
}

// Destructor method. Destroy all sound effect pointer arrays
ofApp::~ofApp()
{
	delete[] fell_sfx;
	delete[] grab_sfx;
	delete[] chop_sfx;
	delete[] place_sfx;
}



// Initial setup
void ofApp::setup()
{
	// Initilise ODE objects
	dInitODE2(0);
	world = dWorldCreate();
	space = dHashSpaceCreate(0);
	ground = dCreatePlane(space, 0, 0, 1, 0);
	dGeomSetCategoryBits(ground, GROUP_COLLIDE);
	contact_group = dJointGroupCreate(0);
	dWorldSetGravity(world, 0, 0, -9.8);
	dAllocateODEDataForThread(dAllocateMaskAll);

	// Initilise OF camera object
	cam.setAutoDistance(false);
	cam.setNearClip(0.1);
	cam.setPosition(0,-16,40);
	cam.lookAt({0,-1,0},ofVec3f(0,0,1));
	cam.setUpAxis(ofVec3f(0,0,1));
#ifndef DEBUG
	cam.disableMouseInput();
#endif

	// Initilise OF light object
	light.setDirectional();
	light.setPosition(0,0,0);
	light.setOrientation(ofVec3f(-150,-45,0));
	light.enable();

	// Initiate everything else
	text.load("Consolas.ttf", 16);
	player.setup();
	train.setup();
	reset();
	ofEnableAlphaBlending();
}

// Reset all parameters to the start
void ofApp::reset()
{
	// Remove all tracks, and replace them
	for(Track* track : tracks) delete track;
	tracks.clear();
	for(int i=-36, j = 0; i < 36; i+=2, j++) {
		tracks.push_back(new Track(i));
	}

	// Player must not be holding anything when fixtures are reset
	player.drop();

	// Remove all fixtures, and replace the trees
	for(fixture* f : fixtures) delete f;
	fixtures.clear();
	for(distance = -700; distance < 0; distance+=7) {
		Tree* tree = new Tree();
		if(tree->getPosition().y != 0) {
			pushFixture(tree);
		} else {
			delete tree;
		}
	}

	// Reset the positions and speed
	player.reset();
	marker_pos = 0;
	new_high_score = false;
	score = 0;
	distance = 0.0f;
	speed = 0.2f;
}

// Update the simulation
void ofApp::update()
{
	// Delegate update to all objects in the scene
	player.update();
	train.update();
	for(Track* track : tracks) {
		track->update();

		// Check if a track is broken infront of the train
		if(track->getPosition().x < -24+(distance/10)) {
			if(!track->hasWood()) gameover();
		}
	}

	for(fixture* fixture : fixtures) {

		// Check if a tree is on the tracks infront of the train
		if(fixture->getPosition().x < -24+(distance/10)) {
			if(Tree* t = dynamic_cast<Tree*>(fixture)) {
				if(t->getPosition().y == 0) gameover();
			}
		}

		// If the fixture is too off the left side... delete it
		if(fixture->getPosition().x < -36+(distance/10)) {
			dropFixture(fixture);
		} else {
			fixture->update();
		}
	}

	// Update the position of the score objects
	for(int i = 0; i < scores.size(); i++) {
		scores[i].pos.z+=0.08;
		if(scores[i].pos.z > 6) {
			scores.erase(scores.begin()+i);
		}
	}

	train.setAlert(false);
	// Only update the simulation if the game is running
	if(game_state == RUNNING)
	{
		cam.setPosition(distance/10, -16, 40);
		distance += speed;
		if(speed < 2) speed += 0.00004;

		if(rand() % (int)(40 - (speed*18)) == 0) {
			pushFixture(new Tree());
		}

		dSpaceCollide(space, 0, &nearCallback);
		dWorldStep(world, 0.05);
		dJointGroupEmpty(contact_group);
	}
}

// Render the game
void ofApp::draw()
{
	// Change OpenFrameworks settings
	ofBackground(20);
	ofEnableLighting();
	ofEnableDepthTest();
	ofPushMatrix();

	// Begin drawing 3D scene
	cam.begin();

#ifdef DEBUG
	// Draw discrete grid if using DEBUG build
	ofDisableLighting(); ofPushStyle(); ofPushMatrix();
	ofTranslate(-1 + (round((distance+10)/20)*2),1);
	ofDrawGrid(2,19,false,false,false,true);
	ofSetLineWidth(4);
	ofSetColor(0,0,255); ofDrawArrow(ofVec3f(0,0,0),ofVec3f(0,0,2), .2);
	ofSetColor(0,255,0); ofDrawArrow(ofVec3f(0,0,0),ofVec3f(0,2,0), .2);
	ofSetColor(255,0,0); ofDrawArrow(ofVec3f(0,0,0),ofVec3f(2,0,0), .2);
	ofPopMatrix(); ofPopStyle(); ofEnableLighting();
#endif

	// Draw the ground plane
	ofPushMatrix();
	ofTranslate(distance/10,0,0);
	ofSetColor(ofColor::fromHex(0x85DD43),255);
	ofDrawPlane(74,34);
	ofPopMatrix();

	// Delegate draw to all objects in the scene
	player.draw();
	train.draw();
	for(Track* track : tracks) track->draw();
	for(fixture* fixture : fixtures) fixture->draw();

	// Draw mile marker line
	ofPushStyle();
	ofSetLineWidth(2);
	ofSetColor(255);
	ofDrawLine(
		ofVec3f(marker_pos-29,-17,.02),
		ofVec3f(marker_pos-29,17,.02)
	);
	ofPopStyle();

	// Draw the 2D UI
	cam.end();
	ofPopMatrix();
	ofDisableLighting();
	ofDisableDepthTest();
	if(game_state == RUNNING) {
		ofSetColor(255);
		drawText("Score: "+to_string(score), 400, 30);
		drawText("High Score: "+to_string(high_score), 400, 60);
		drawText("Distance: "+to_string((int)distance/20), 400, 90);
		drawText("Speed: "+to_string(round(speed*1000)/1000), 400, 120);

		// Draw the chop cooldown timer
		ofVec3f sp;
		float percent = (float)(player.getChopCooldown()) / (60-getSpeed()*20);
		sp = cam.worldToScreen(player.getPosition());
		sp.x -= 30 * percent; sp.y -= 30;
		ofDrawRectangle(sp, 60 * percent,10);

		// Draw mile marker text
		if(marker_pos+25 < (distance/10)) marker_pos+=100;
		sp = cam.worldToScreen(ofVec3f(marker_pos-29,-18,0));
		drawText(to_string((marker_pos/2)),sp.x, sp.y, true);

		// Draw the floating scores
		for(Score score : scores) {
			ofSetColor(score.col);
			sp = cam.worldToScreen(score.pos);
			drawText(to_string(score.value * score.multipler),sp.x, sp.y);
		}
	} else {
		if(game_state == STARTUP) {
			// Draw the startup display
			ofSetColor(0); ofDrawRectangle((ofGetWidth()/2)-300,(ofGetHeight()/2)-60,600,120);
			ofSetColor(255); drawText("Buster Keaton Train Game!", (ofGetWidth()/2),(ofGetHeight()/2)-20, true);
			ofSetColor(80); drawText("press any key to start...", (ofGetWidth()/2),(ofGetHeight()/2)+20, true);
		} else if(game_state == GAMEOVER) {
			// Draw the gameover display
			ofSetColor(0); ofDrawRectangle((ofGetWidth()/2)-300,(ofGetHeight()/2)-105,600,210);
			ofSetColor(255); drawText(new_high_score ? "New High Score!" : "Game Over!", (ofGetWidth()/2),(ofGetHeight()/2)-70, true);
			ofSetColor(255); drawText("You scored " + to_string(score) + " points", (ofGetWidth()/2),(ofGetHeight()/2)-40, true);
			ofSetColor(255); drawText("High Score: " + to_string(high_score), (ofGetWidth()/2),(ofGetHeight()/2)+10, true);
			ofSetColor(80); drawText("press any key to play again...", (ofGetWidth()/2),(ofGetHeight()/2)+70, true);
		}
	}
}

// Perform some gameover actions
void ofApp::gameover()
{
	game_state = GAMEOVER;
	if(score > high_score) {
		new_high_score = true;
		high_score = score;
	}
}

// Application exits. All ODE objects should be destroyed here to exit safely
void ofApp::exit()
{
	player.exit();
	train.exit();

	dGeomDestroy(ground);
	dJointGroupDestroy(contact_group);
	dSpaceDestroy(space);
	dWorldDestroy(world);

	dCloseODE();
}



// Key is pressed
void ofApp::keyPressed(int key)
{
	if(game_state == STARTUP) {
		game_state = RUNNING;
	} else if(game_state == GAMEOVER) {
		reset();
		game_state = RUNNING;
	} else if(game_state == RUNNING) {
		switch(key) {
			case 'w': case 'W':
				player.setMagnitude(0, 1.0f);
			break;
			case 'a': case 'A':
				player.setMagnitude(1, 1.0f);
			break;
			case 's': case 'S':
				player.setMagnitude(2, 1.0f);
			break;
			case 'd': case 'D':
				player.setMagnitude(3, 1.0f);
			break;
	#ifdef DEBUG
			case 'q': case 'Q':
				pushFixture(new Wood(player.getPosition().x, player.getPosition().y));
			break;
			case OF_KEY_UP:
				speed = 2.0f;
			break;
			case OF_KEY_DOWN:
				speed = 0.2f;
			break;
	#endif
			case OF_KEY_LEFT_SHIFT:
				player.lunge();
			break;
			case ' ':
				// Handle player interaction with Wood objects
				if(player.getGrabbed()) {
					if(player.getSelected()) player.place();
					else {
						player.drop();
						// If the player recently lunged and dropped (thrown an object) give them some score
						if(player.getLungeCooldown() < 10) addScore(10,1, true, player.getPosition());
					}
				}
				else if(player.getSelected()) player.grab();
			break;
		}
	}
}

// Key is released
void ofApp::keyReleased(int key)
{
	switch(tolower(key)) {
		case 'w': case 'W':
			player.setMagnitude(0, 0.0f);
		break;
		case 'a': case 'A':
			player.setMagnitude(1, 0.0f);
		break;
		case 's': case 'S':
			player.setMagnitude(2, 0.0f);
		break;
		case 'd': case 'D':
			player.setMagnitude(3, 0.0f);
		break;
	}
}



// Gets
dWorldID ofApp::getWorld() {return world;}
dSpaceID ofApp::getSpace() {return space;}
float ofApp::getSpeed() {return speed;}
float ofApp::getDistance() {return distance;}
ofEasyCam& ofApp::getCam() {return cam;}
Player ofApp::getPlayer() {return player;}
Train ofApp::getTrain() {return train;}



// Handle collison detection
void ofApp::collide(dGeomID o1, dGeomID o2)
{
	unsigned long o1Cat = dGeomGetCategoryBits(o1);
	unsigned long o2Cat = dGeomGetCategoryBits(o2);

	const int N = 40;
	dContact contact[N];
	int n = dCollide(o1, o2, N, &contact[0].geom, sizeof(dContact));
	if(n > 0) {
		// If both geoms are collideable...
		if((GROUP_COLLIDE & o1Cat) && (GROUP_COLLIDE & o2Cat)) {
			for(int i = 0; i < n; i++)
			{
				contact[i].surface.mode = dContactSlip1 | dContactSlip2 | dContactApprox1;
				contact[i].surface.mu = 0.2;
				contact[i].surface.slip1 = 1;
				contact[i].surface.slip2 = 1;

				// ...collide
				dJointID j = dJointCreateContact(world, contact_group, &contact[i]);
				dJointAttach(j, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));
			}
		}
	}

	// If one geom is a hittest and the other is a hittest reciever... perform a hittest
	if(((GROUP_HITTEST & o1Cat) && (GROUP_RECIEVE & o2Cat)) || ((GROUP_HITTEST & o2Cat) && (GROUP_RECIEVE & o1Cat))) {
		if(dynamic_cast<Player*>((ofNode*)dGeomGetData((GROUP_HITTEST & o1Cat) ? o1 : o2))) {
			fixture* f = reinterpret_cast<fixture*>(dGeomGetData((GROUP_RECIEVE & o2Cat) ? o2 : o1));
			if(dynamic_cast<Tree*>(f)) f->select();
			if(f) player.select(f);
		}
		else if(dynamic_cast<Train*>((ofNode*)dGeomGetData((GROUP_HITTEST & o1Cat) ? o1 : o2))) {
			fixture* f = reinterpret_cast<fixture*>(dGeomGetData((GROUP_RECIEVE & o2Cat) ? o2 : o1));
			Track* track = dynamic_cast<Track*>(f);
			if(dynamic_cast<Tree*>(f) || (track && !track->hasWood())) train.setAlert(true);
		}
	}
}



// Push to vector, allowing to reference later
void ofApp::pushFixture(fixture* f)
{
	fixtures.push_back(f);
}

// Remove from vector, and safely delete
void ofApp::dropFixture(fixture* f)
{
	if(f == player.getGrabbed()) player.drop();
	fixtures.erase(find(fixtures.begin(), fixtures.end(), f));
	delete f;
}



// Add some score, and optionally visualise it
void ofApp::addScore(int points, int multipler, bool show, ofVec2f pos)
{
	score += points * multipler;
	if(show) {
		int red = min(254, (points*multipler)/5);
		Score score = {points, multipler, ofVec3f(pos.x,pos.y,2), ofColor(255,255-red,255-red)};
		scores.push_back(score);
	}
}

// Play a random sound effect for variety
void ofApp::playSFX(SFX type)
{
	switch(type) {
		case FELL:
			fell_sfx[rand()%4].play();
		break;
		case GRAB:
			grab_sfx[rand()%4].play();
		break;
		case CHOP:
			chop_sfx[rand()%4].play();
		break;
		case PLACE:
			place_sfx[rand()%4].play();
		break;
	}
}

// A simple helper function to clean up some code
void ofApp::drawText(string _text, float x, float y, bool center)
{
	text.drawString(
		_text,
		x-(center ? text.stringWidth(_text)/2 : 0),
		y+(center ? (text.stringHeight(_text)/2)-2 : 0)
	);
}



void nearCallback(void*, dGeomID o1, dGeomID o2)
{
	app->collide(o1, o2);
}
