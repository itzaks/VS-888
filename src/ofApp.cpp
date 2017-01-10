#include "ofApp.h"

ofTexture pixelOutput;

//--------------------------------------------------------------
void ofApp::setup(){
	shader.load("shadersES2/shader");

  int width = 320;
	int height = 180;

	doLoadNewVideo = false;
	currentVideo = 0;

	// SETUP VIDEOS

	videos[0] = ofToDataPath("video/video0.mp4", true);
	videos[1] = ofToDataPath("video/video1.mp4", true);
	videos[2] = ofToDataPath("video/video2.mp4", true);
	videos[3] = ofToDataPath("video/video3.mp4", true);

	for (int i=0; i<3; i++) {
		ofxOMXPlayerSettings settings;

		settings.useHDMIForAudio = false;
		settings.enableAudio = false;
		settings.videoPath = videos[i];
		omxPlayers[i].setup(settings);
	}

	ofxOMXPlayerSettings settings;
	settings.useHDMIForAudio = false;
	settings.enableAudio = false;
	settings.videoPath = ofToDataPath("noise.mov", true);
	omxPlayerNoise.setup(settings);

  fbo.allocate(width, height);
  maskFbo.allocate(width, height);

	//MIDI
	midiIn.listPorts(); // via instance
	midiIn.openPort(1);
	midiIn.addListener(this);
	midiIn.setVerbose(true);

	//RECORING
	doStartRecording = false;
	doStopRecording = false;

	int numColors = 3;

	colorFormat = GL_RGB;

	ofxOMXRecorderSettings settingsRecorder;
	settingsRecorder.width = width;
	settingsRecorder.height = height;
	settingsRecorder.fps = 30;
	settingsRecorder.colorFormat = colorFormat;
	settingsRecorder.bitrateMegabytesPerSecond = 1.0;  //default 2.0, max untested
	settingsRecorder.enablePrettyFileName = true; //default true
	recorder.setup(settingsRecorder);

	int dataSize = width * height * numColors;
	pixels = new unsigned char[dataSize];
	memset(pixels, 0xff, dataSize); //set to white
}

//--------------------------------------------------------------
void ofApp::update(){
		if(doStartRecording) {
	    doStartRecording = false;
	    recorder.startRecording();
		}

	  if (doStopRecording) {
		  doStopRecording = false;
		  recorder.stopRecording();
		}

		if(recorder.isRecording()) {
	    recorder.update(pixels);
	  }

		if (doLoadNewVideo) {
			ofLogVerbose(__func__) << "doing reload";

			//with the texture based player this must be done here - especially if the videos are different resolutions
			loadNewVideo();
		}
}

//--------------------------------------------------------------
void ofApp::draw(){
	float TIME = ofGetElapsedTimef();

  //------------------------------------------- draw to mask fbo.
  maskFbo.begin();
  ofClear(255, 0, 0, 255);
  maskFbo.end();

  //------------------------------------------- draw to final fbo.
  fbo.begin();
  	ofClear(0, 0, 0, 255);
    shader.begin();
			shader.setUniformTexture("tex0", omxPlayers[currentVideo].getTextureReference(), 1);
	    shader.setUniformTexture("tex1", omxPlayerNoise.getTextureReference(), 2);
	    shader.setUniformTexture("tex2", fbo.getTextureReference(), 3);
			omxPlayers[currentVideo].draw(0, 0, width, height);
			omxPlayerNoise.draw(0, 0, width, height);

			shader.setUniform1f("TIME", TIME);
			shader.setUniform1f("mod0", controllers[0]);
			shader.setUniform1f("mod1", controllers[1]);
			shader.setUniform1f("mod2", controllers[2]);
			shader.setUniform1f("mod3", controllers[3]);
			shader.setUniform1f("mod4", controllers[4]);
			shader.setUniform1f("mod5", controllers[5]);
			shader.setUniform1f("mod6", controllers[6]);

	    // we are drawing this fbo so it is used just as a frame.
	    maskFbo.draw(0, 0);
  	shader.end();

		if(recorder.isRecording()) {
			glReadPixels(0,0, fbo.getWidth(), fbo.getHeight(), colorFormat, GL_UNSIGNED_BYTE, pixels);
		}

  fbo.end();
  fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

  bool isRecording = recorder.isRecording();

  stringstream info;
  info << "App FPS: " << ofGetFrameRate() << endl;
  info << "PRESS 1 to START RECORDING "   << isRecording << endl;
  info << "PRESS 2 to STOP RECORDING "    << isRecording << endl;

	// draw the last recieved message contents to the screen
	info << "Received: " << ofxMidiMessage::getStatusString(midiMessage.status) << endl;
	info << "control: " << midiMessage.control << endl;
	info << "value: " << midiMessage.value << endl;
	info << "pitch: " << midiMessage.pitch << endl;

	if (isRecording) {
    info << "FRAMES RECORDED: "  << recorder.getFrameCounter() << endl;
  }

	ofDrawBitmapStringHighlight(info.str(), 100, 100, ofColor::black, ofColor::yellow);
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
	// make a copy of the latest message
	midiMessage = msg;

	if(midiMessage.status == MIDI_CONTROL_CHANGE) {
		if(midiMessage.control == 13) {
			controllers[0] = midiMessage.value / 127.0f;
			ofLog() << "SET CONTROLLER_1 TO " << controllers[0];
		} else if(midiMessage.control == 14) {
			controllers[1] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 15) {
			controllers[2] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 16) {
			controllers[3] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 17) {
			controllers[4] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 18) {
			controllers[5] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 19) {
			controllers[6] = midiMessage.value / 127.0f;
		}

		// RECORD
		if(midiMessage.control == 64 && midiMessage.value == 127){
	    doStartRecording = true;
			ofLog() << "START REC";
		}

		if(midiMessage.control == 65 && midiMessage.value == 127) {
	    ofLog() << "STOP REC";
		  doStopRecording = true;
		}

		// PLAY VIDEOS
		if(midiMessage.control == 66 && midiMessage.value == 127) {
			changeToVideo = 0;
			doLoadNewVideo = true;
			ofLog() << "PLAY_0";
		}

		if(midiMessage.control == 67 && midiMessage.value == 127) {
			changeToVideo = 1;
			doLoadNewVideo = true;
			ofLog() << "PLAY_1";
		}

		if(midiMessage.control == 68 && midiMessage.value == 127) {
			changeToVideo = 2;
			doLoadNewVideo = true;
			ofLog() << "PLAY_2";
		}

		if(midiMessage.control == 69 && midiMessage.value == 127) {
			changeToVideo = 3;
			doLoadNewVideo = true;
			ofLog() << "PLAY_3";
		}
	}
}

void ofApp::loadNewVideo() {
	if(changeToVideo == currentVideo) {
		return;
	}
	//omxPlayers[changeToVideo].seekToTimeInSeconds(0);
	/*
		if(omxPlayers[currentVideo].isPlaying() || !omxPlayers[currentVideo].isPaused()) {
				omxPlayers[currentVideo].setPaused(true);
		}

		if(!omxPlayers[changeToVideo].isPlaying() || omxPlayers[changeToVideo].isPaused()) {
			omxPlayers[changeToVideo].seekToTimeInSeconds(0);
			omxPlayers[changeToVideo].setPaused(false);
		}
	*/
	currentVideo = changeToVideo;
	doLoadNewVideo = false;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

//--------------------------------------------------------------
void ofApp::onVideoEnd(ofxOMXPlayerListenerEventData& e) {
	ofLogVerbose(__func__) << "at: " << ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void ofApp::onVideoLoop(ofxOMXPlayerListenerEventData& e) {
	ofLogVerbose(__func__) << "at: " << ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void ofApp::exit() {
	// clean up
	midiIn.closePort();
	midiIn.removeListener(this);
}
