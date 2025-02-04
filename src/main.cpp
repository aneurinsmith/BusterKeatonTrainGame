
#include "ofMain.h"
#include "ofApp.h"

ofApp* app;

int main()
{
	app = new ofApp();
	ofSetupOpenGL(1024,768,OF_WINDOW);
	ofRunApp(app);
}
