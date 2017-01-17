#include "ofApp.h"

ofTexture pixelOutput;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetLogLevel(OF_LOG_FATAL_ERROR);
	shader.load("shadersES2/shader");

  int width = 320;
	int height = 180;

	doLoadNewVideo = false;
	currentVideo = 0;
	currentRecording = -1;

	// AUDIO
	soundStream.printDeviceList();
	soundStream.setDeviceID(0);

	int bufferSize = 256;

	smoothedVol = 0.0;
	volume = 0.0;

	audioInput.assign(bufferSize, 0.0);
	soundStream.setup(this, 2, 1, 44100, bufferSize, 4);

	ofLog() << "NEW BUILD";

	// SETUP VIDEOS
	videos[0] = ofToDataPath("video/video0.mp4", true);
	videos[1] = ofToDataPath("video/video1.mp4", true);
	videos[2] = ofToDataPath("video/video2.mp4", true);
	videos[3] = ofToDataPath("video/video3.mp4", true);
	videos[4] = ofToDataPath("video/video4.mp4", true);
	videos[5] = ofToDataPath("video/video5.mp4", true);
	videos[6] = ofToDataPath("video/video6.mp4", true);
	videos[7] = ofToDataPath("video/video7.mp4", true);

	for (int i=0; i<2; i++) {
		ofxOMXPlayerSettings settingsVideo;

		settingsVideo.useHDMIForAudio = false;
		settingsVideo.enableAudio = false;
		settingsVideo.videoPath = videos[i];
		omxPlayers[i].setup(settingsVideo);
	}

	// SHADER
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
	settingsRecorder.fps = 60;
	settingsRecorder.colorFormat = colorFormat;
	settingsRecorder.bitrateMegabytesPerSecond = 2.0;  //default 2.0, max untested
	settingsRecorder.enablePrettyFileName = false; //default true
	recorder.setup(settingsRecorder);

	int dataSize = width * height * numColors;
	pixels = new unsigned char[dataSize];
	memset(pixels, 0xff, dataSize); //set to white
}

//--------------------------------------------------------------
void ofApp::update(){
		volume = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);

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
			loadNewVideo();
		}

		int latestRecordingIndex = recorder.recordings.size() - 1;
		if (latestRecordingIndex != -1 && currentRecording != latestRecordingIndex) {
			string recordedFile = recorder.recordings[latestRecordingIndex].path();
			omxPlayers[0].loadMovie(recordedFile);
			currentRecording = latestRecordingIndex;
			memset(pixels, 0x00, width * height * 3);
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
			shader.setUniformTexture("VID_1", omxPlayers[0].getTextureReference(), 1);
	    shader.setUniformTexture("VID_2", omxPlayers[1].getTextureReference(), 2);

	    shader.setUniformTexture("VID_BUFFER", fbo.getTextureReference(), 3);

			omxPlayers[0].draw(0, 0, width, height);
			omxPlayers[1].draw(0, 0, width, height);

			shader.setUniform1f("TIME", TIME);
			shader.setUniform1f("VOLUME", volume);
			shader.setUniform1f("VOLUME2", smoothedVol);
			
			shader.setUniform1f("TOP_KNOB_1", controllers[0]);
			shader.setUniform1f("TOP_KNOB_2", controllers[1]);
			shader.setUniform1f("TOP_KNOB_3", controllers[2]);
			shader.setUniform1f("TOP_KNOB_4", controllers[3]);
			shader.setUniform1f("TOP_KNOB_5", controllers[4]);
			shader.setUniform1f("TOP_KNOB_6", controllers[5]);
			shader.setUniform1f("TOP_KNOB_7", controllers[6]);
			shader.setUniform1f("TOP_KNOB_8", controllers[7]);

			shader.setUniform1f("BOTTOM_KNOB_1", controllers[8]);
			shader.setUniform1f("BOTTOM_KNOB_2", controllers[9]);
			shader.setUniform1f("BOTTOM_KNOB_3", controllers[10]);
			shader.setUniform1f("BOTTOM_KNOB_4", controllers[11]);
			shader.setUniform1f("BOTTOM_KNOB_5", controllers[12]);
			shader.setUniform1f("BOTTOM_KNOB_6", controllers[13]);
			shader.setUniform1f("BOTTOM_KNOB_7", controllers[14]);
			shader.setUniform1f("BOTTOM_KNOB_8", controllers[15]);

			shader.setUniform1f("BUTTON_1", controllers[16]);
			shader.setUniform1f("BUTTON_2", controllers[17]);
			shader.setUniform1f("BUTTON_3", controllers[18]);
			shader.setUniform1f("BUTTON_4", controllers[19]);
			shader.setUniform1f("BUTTON_5", controllers[20]);
			shader.setUniform1f("BUTTON_6", controllers[21]);
			shader.setUniform1f("BUTTON_7", controllers[22]);
			shader.setUniform1f("BUTTON_8", controllers[23]);

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
	info << "volume: " << volume << endl;
	info << "VOLME2: " << smoothedVol << endl;

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
			//TOP ROW
			controllers[0] = midiMessage.value / 127.0f;
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
		} else if(midiMessage.control == 20) {
			controllers[7] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 21) {
			// BOTTOM ROW
			controllers[8] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 22) {
			controllers[9] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 23) {
			controllers[10] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 24) {
			controllers[11] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 25) {
			controllers[12] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 26) {
			controllers[13] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 27) {
			controllers[14] = midiMessage.value / 127.0f;
		} else if(midiMessage.control == 28) {
			controllers[15] = midiMessage.value / 127.0f; // NO SMOOTHING AS ITS THE SMOOTHER :)
		}

		// RECORD
		if(midiMessage.control == 116 && midiMessage.value == 127){
			if(doStartRecording || doStopRecording) {
				return;
			}

	    if(recorder.isRecording() == false) {
				doStartRecording = true;
				ofLog() << "START REC";
			} else {
		    ofLog() << "STOP REC";
			  doStopRecording = true;
			}
		}

		// PLAY VIDEOS
		if(midiMessage.control == 64 && midiMessage.value == 127) {
			changeToVideo = 0;
			doLoadNewVideo = true;
		} else if(midiMessage.control == 65 && midiMessage.value == 127) {
			changeToVideo = 1;
			doLoadNewVideo = true;
		} else if(midiMessage.control == 66 && midiMessage.value == 127) {
			changeToVideo = 2;
			doLoadNewVideo = true;
		} else if(midiMessage.control == 67 && midiMessage.value == 127) {
			changeToVideo = 3;
			doLoadNewVideo = true;
		} else if(midiMessage.control == 68 && midiMessage.value == 127) {
			changeToVideo = 4;
			doLoadNewVideo = true;
		} else if(midiMessage.control == 69 && midiMessage.value == 127) {
			changeToVideo = 5;
			doLoadNewVideo = true;
		} else if(midiMessage.control == 70 && midiMessage.value == 127) {
			changeToVideo = 6;
			doLoadNewVideo = true;
		} else if(midiMessage.control == 71 && midiMessage.value == 127) {
			changeToVideo = 7;
			doLoadNewVideo = true;
		}
	}
}

float ofApp::smoothValue(float newValue, float value) {
	value *= controllers[15];
	value += (1.0 - controllers[15]) * newValue;
	return value;
}

void ofApp::loadNewVideo() {
	if(changeToVideo == currentVideo) {
		return;
	}

	currentVideo = changeToVideo;
	omxPlayers[1].loadMovie(videos[currentVideo]);
	doLoadNewVideo = false;
}

//--------------------------------------------------------------
void ofApp::audioIn(float * buffer, int bufferSize, int nChannels){
	float curVol = 0.0;
	int numCounted = 0;

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume
	for(int i = 0 ; i < bufferSize; i++){
	  audioInput[i] = buffer[i];
	  curVol += audioInput[i]*audioInput[i] * 0.25;
	  numCounted+=2;
	}

	//this is how we get the mean of rms :)
	curVol /= (float)numCounted;
	curVol = sqrt( curVol );

	smoothedVol = smoothValue(curVol, smoothedVol);
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer & buffer){
	for (int i = 0; i < buffer.getNumFrames(); i++){
		buffer[i*buffer.getNumChannels()    ] = audioInput[i];
		buffer[i*buffer.getNumChannels() + 1] = audioInput[i];
	}
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
