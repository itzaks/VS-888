#pragma once

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

    // GENERAL
    vector<string> arguments;
    float smoothValue(float newValue, float value);
    bool showMenu;

    // AUDIO
    void audioIn(ofSoundBuffer& buffer);
    void audioOut(ofSoundBuffer& buffer);

    ofMutex audioMutex;
    ofSoundBuffer inputBuffer;

    ofSoundStream soundStream;
    vector <float> audioInput;

    float smoothedVol;
    float smoothedVolLite;
    float volume;
    float volumeSmooth;

    // SHADER
	  ofShader shader;
    ofFbo fbo;
    ofFbo maskFbo;

    // PLAYER
	  map<int, ofxOMXPlayer> omxPlayers;
    ofxOMXPlayerSettings settingsVideo;

    int width;
    int height;

    int activeVideo;
    int currentVideo;
    int changeToVideo;
    bool doLoadNewVideo;
    string videos[8];

    void loadNewVideo();
		void onVideoEnd(ofxOMXPlayerListenerEventData& e);
		void onVideoLoop(ofxOMXPlayerListenerEventData& e);

    // MIDI
    void newMidiMessage(ofxMidiMessage& eventArgs);

    ofxMidiIn midiIn;
    ofxMidiMessage midiMessage;

    float controllers[24];

    // RECORDER
    ofxOMXRecorder recorder;
    GLint colorFormat;
    int recFrameSize;
    unsigned char* pixels;

    bool doStartRecording;
    bool doStopRecording;
    int latestRecordingIndex;
    int currentRecording;
};
