#pragma once

#define WEBCAM

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxOMXPlayer.h"
#include "ofxOMXRecorder.h"

class ofApp : public ofBaseApp, public ofxOMXPlayerListener, public ofxMidiListener {
public:

    void setup();
    void update();
    void draw();
		void exit();

    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

	  ofShader shader;

    ofVideoGrabber camera;
	  map<int, ofxOMXPlayer> omxPlayers;
    ofxOMXPlayer omxPlayerNoise;
    ofxOMXPlayerSettings settings;

    int width;
    int height;

    int currentVideo;
    int changeToVideo;
    bool doLoadNewVideo;

    void loadNewVideo();

		void onVideoEnd(ofxOMXPlayerListenerEventData& e);
		void onVideoLoop(ofxOMXPlayerListenerEventData& e);

    ofFbo fbo;
    ofFbo maskFbo;

    void newMidiMessage(ofxMidiMessage& eventArgs);

    ofxMidiIn midiIn;
    ofxMidiMessage midiMessage;

    float controllers[7];
    string videos[4];

    ofxOMXRecorder recorder;
    GLint colorFormat;
    unsigned char* pixels;

    bool doStartRecording;
    bool doStopRecording;
    bool loadedRecording;
};
