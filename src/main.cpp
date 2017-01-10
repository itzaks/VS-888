#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {

  ofGLESWindowSettings settings;
  settings.glesVersion = 2;
	settings.windowMode = OF_FULLSCREEN;

  ofCreateWindow(settings);
	ofRunApp(new ofApp());

}
