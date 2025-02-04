
#pragma once
#include "ofMain.h"
#include "ode/ode.h"

#include "player.h"
#include "train.h"
#include "wood.h"
#include "tree.h"
#include "track.h"

struct Score {
	int value;
	int multipler;
	ofVec3f pos;
    ofColor col;
};

enum SFX {
	GRAB = 0,
	CHOP = 1,
	FELL = 2,
	PLACE = 3
};

enum GameState {
	STARTUP = 0,
	RUNNING = 1,
	GAMEOVER = 2
};

/*
	These are the geom category bits used when
	carrying out collision detection.

	GROUP_HITTEST	-	Run the collision script
	GROUP_COLLIDE	-	Run the collide script
	GROUP_RECIEVE	-	Used to specify that it
						can recieve hittests
*/
const unsigned long GROUP_HITTEST = 1 << 1;
const unsigned long GROUP_COLLIDE = 1 << 2;
const unsigned long GROUP_RECIEVE = 1 << 3;

class ofApp : public ofBaseApp {

public:
	ofApp();
	~ofApp();

	void setup();
	void reset();
	void update();
	void draw();
	void gameover();
	void exit();

	void collide(dGeomID, dGeomID);

	void keyPressed(int key);
	void keyReleased(int key);

	dWorldID getWorld();
	dSpaceID getSpace();
	float getSpeed();
	float getDistance();
	ofEasyCam& getCam();
	Player getPlayer();
	Train getTrain();

	void pushFixture(fixture*);
	void dropFixture(fixture*);

	void addScore(int, int, bool = true, ofVec2f = {0,0});
	void playSFX(SFX type);

private:
	void drawText(string, float, float, bool = false);

	int marker_pos;
	int game_state;
	int score, high_score;
	bool new_high_score;
	float speed, distance;

	dWorldID world;
	dSpaceID space;
	dGeomID ground;
	dJointGroupID contact_group;

	ofEasyCam cam;
	ofLight light;

	Player player;
	Train train;
	vector<fixture*> fixtures;
	vector<Track*> tracks;
	vector<Score> scores;

	ofTrueTypeFont text;
	ofSoundPlayer* fell_sfx;
	ofSoundPlayer* grab_sfx;
	ofSoundPlayer* chop_sfx;
	ofSoundPlayer* place_sfx;
};

static void nearCallback(void*, dGeomID, dGeomID);
extern ofApp* app;
