#pragma once

#define WEBCAM

#include "ofMain.h"
#include "ofxOMXPlayer.h"


class ofApp : public ofBaseApp, public ofxOMXPlayerListener{
public:

    void setup();
    void update();
    void draw();

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

    ofImage image;
    ofImage imageMask;
    ofVideoPlayer movie;
    ofVideoGrabber camera;
    ofxOMXPlayer omxPlayer;
    ofxOMXPlayer omxPlayerNoise;

    int camWidth;
    int camHeight;

    bool doShader;
    bool doTextures;
    void loadShader();

    void onVideoEnd(ofxOMXPlayerListenerEventData& e);
    void onVideoLoop(ofxOMXPlayerListenerEventData& e);

    ofFbo fbo;
    ofFbo maskFbo;
};
