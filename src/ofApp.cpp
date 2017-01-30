#include "ofApp.h"

ofTexture pixelOutput;

//--------------------------------------------------------------
void ofApp::setup(){
	//ofSetLogLevel(OF_LOG_FATAL_ERROR);
	ofHideCursor();

	shader.load("shaders/shader");

  int width = 320;
	int height = 180;

	showMenu = false;

	doLoadNewVideo = false;
	selectedPadImage = 0;
	selectedPadVideo = 0;
	currentRecording = -1;

	// AUDIO
	int bufferSize = 256;
	smoothedVol = 0.0;
	smoothedVolLite = 0.0;
	volume = 0.0;
	volumeSmooth = 0.0;

	soundStream.printDeviceList();
	soundStream.setDeviceID(0);
	soundStream.setup(this, 2, 1, 11025, bufferSize, 4);

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
		if(i == 0) {
			settingsVideo.videoPath = videos[i];
		} else {
			settingsVideo.videoPath = ofToDataPath("video/texture.mp4", true);
		}

		omxPlayers[i].setup(settingsVideo);
	}

	// SETUP IMAGES
	loadedImage = false;
	images[0] = ofToDataPath("images/image0.jpg", true);
	images[1] = ofToDataPath("images/image1.jpg", true);
	images[2] = ofToDataPath("images/image2.jpg", true);
	images[3] = ofToDataPath("images/image3.jpg", true);
	images[4] = ofToDataPath("images/image4.jpg", true);
	images[5] = ofToDataPath("images/image5.jpg", true);
	images[6] = ofToDataPath("images/image6.jpg", true);
	images[7] = ofToDataPath("images/image7.jpg", true);

	// SHADER
  fbo.allocate(width, height);
  maskFbo.allocate(width, height);

	//MIDI
	midiIn.listPorts(); // via instance
	midiIn.openPort("Launch Control 20:0"); // BY NAME TODO
	midiIn.addListener(this);

	midiOut.listPorts(); // via instance
	midiOut.openPort("Launch Control 20:0"); // .openPort("IAC Driver Pure Data In"); BY NAME TODO

	// LAYER COLORS
	setLED(2, 116, 3); // SET TO RED
	setLED(2, 117, 29); // SET TO YELLOW LIGHT

	// CLEAR BUTTONS
	setLED(2, 64, 31);
	setLED(2, 65, 0);
	setLED(2, 66, 0);
	setLED(2, 67, 0);
	setLED(2, 68, 0);
	setLED(2, 69, 0);
	setLED(2, 70, 0);
	setLED(2, 71, 0);

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
	settingsRecorder.bitrateMegabytesPerSecond = 3.0;  //default 2.0, max untested
	settingsRecorder.enablePrettyFileName = false; //default true
	recorder.setup(settingsRecorder);

	recFrameSize = settingsRecorder.width * settingsRecorder.height * numColors;
	pixels = new unsigned char[recFrameSize];
	memset(pixels, 0x00, recFrameSize); //set to white

	ofDirectory::removeDirectory("recordings", true);
	if(!ofDirectory::doesDirectoryExist("recordings")) {
		ofDirectory::createDirectory("recordings");
	}
}

//--------------------------------------------------------------
void ofApp::update(){
		volume = ofMap(smoothedVolLite, 0.0, 0.17, 0.0, 1.0, true);
		volumeSmooth = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);

		if(doStartRecording) {
	    doStartRecording = false;
			string absoluteFilePath;
			absoluteFilePath = ofToDataPath("recordings/" + ofGetTimestampString() + ".h264", true);
			recorder.startRecording(absoluteFilePath);
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
			videos[7] = recorder.recordings[latestRecordingIndex].path();
			//string recordedFile = recorder.recordings[latestRecordingIndex].path();
			//omxPlayers[0].loadMovie(recordedFile);
			currentRecording = latestRecordingIndex;
			memset(pixels, 0x00, recFrameSize);
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
			shader.setUniformTexture("VIDEO", omxPlayers[0].getTextureReference(), 1);
	    shader.setUniformTexture("TEXTURE", omxPlayers[1].getTextureReference(), 2);

			if(loadedImage == true) {
				shader.setUniformTexture("IMAGE", image, 3);
			}

	    shader.setUniformTexture("VID_BUFFER", fbo.getTextureReference(), 4);

			omxPlayers[0].draw(0, 0, width, height);
			omxPlayers[1].draw(0, 0, width, height);

			shader.setUniform1f("TIME", TIME);
			shader.setUniform1f("VOLUME", volume);
			shader.setUniform1f("VOLUME_SMOOTH", volumeSmooth);

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

	    // we are drawing this fbo so it is used just as a frame.
	    maskFbo.draw(0, 0);
  	shader.end();

		if(recorder.isRecording()) {
			glReadPixels(0,0, fbo.getWidth(), fbo.getHeight(), colorFormat, GL_UNSIGNED_BYTE, pixels);
		}

	if(loadedImage) {
		image.draw(0, 0, width, height);
	}

  fbo.end();
  fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	if(showMenu) {
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
		info << "Volume: " << volume << endl;
		info << "Volume Smooth: " << volumeSmooth << endl;

		if (isRecording) {
	    info << "FRAMES RECORDED: "  << recorder.getFrameCounter() << endl;
	  }

		ofDrawBitmapStringHighlight(info.str(), 100, 100, ofColor::black, ofColor::yellow);
	}
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
	// make a copy of the latest message
	midiMessage = msg;

	if(midiMessage.status == MIDI_CONTROL_CHANGE) {

		// KNOBS
		if(midiMessage.control >= 13 && midiMessage.control <= 28) {
			controllers[midiMessage.control - 13] = midiMessage.value / 127.0f;
		}

		// MENU
		if(midiMessage.control == 114){
			showMenu = (midiMessage.value == 127);
			setLED(2, 114, midiMessage.value); // SET TO RED
		}

		// RECORDING
		if(midiMessage.control == 115 && midiMessage.value == 127){
			if(doStartRecording || doStopRecording) {
				return;
			}

	    if(recorder.isRecording() == false) {
				setLED(2, 115, 3); // SET TO RED
				doStartRecording = true;
				ofLog() << "START REC";
			} else {
				setLED(2, 115, 0); // SET TO RED
		    ofLog() << "STOP REC";
			  doStopRecording = true;
			}
		}

		// LAYER SELECTION
		if(midiMessage.control == 116 && midiMessage.value == 127) {
			setLED(2, 116, 3); // SET TO RED
			setLED(2, 117, 29); // SET TO YELLOW LIGHT
			activeLayer = 0;
		}

		if(midiMessage.control == 117 && midiMessage.value == 127) {
			setLED(2, 117, 50); // SET TO YELLOW
			setLED(2, 116, 9); // SET TO RED LIGHT
			activeLayer = 1;
		}

		// SAMPLE SELECTOR / PADS
		if((midiMessage.control >= 64 && midiMessage.control <= 71) && midiMessage.value == 127) {
			activePad = midiMessage.control - 64;
			doLoadNewVideo = true;
		}
	}
}

float ofApp::smoothValue(float newValue, float value) {
	float smoothing = 0.7;
	value *= smoothing;
	value += (1.0 - smoothing) * newValue;
	return value;
}

void ofApp::loadNewVideo() {
	// YELLOW LIGHT 29
	// YELLOW 33
	// YELLOW 50
	// RED 3
	// RED MID 6
	// RED LIGHT 9
	// GREEN LIGHT 16, 20
	// GREEN MID 100
	// GREEN 18
	// ORANGE LIGHT 29
	// ORANGE MID 30
	// ORANGE 31

	int color = 0;

	if(activeLayer == 0) {
		// SET NEW PAD
		if(selectedPadImage == activePad) {
			color = 31; // ORANGE IF SAME AS IMAGE
		} else {
			color = 3; // ELSE RED
		}

		setLED(2, 64 + activePad, color); // SET NEW COLOR

		// UNSET OLD PAD
		if(selectedPadImage == selectedPadVideo) {
			color = 50; // HAS IMAGE, SET TO YELLOW
		} else if(selectedPadVideo != activePad) {
			color = 0;
		}

		setLED(2, 64 + selectedPadVideo, color); // SET TO BLANK

		selectedPadVideo = activePad;
		omxPlayers[0].loadMovie(videos[selectedPadVideo]);

	} else if(activeLayer == 1) {
		// SET NEW PAD
		if(selectedPadVideo == activePad) {
			color = 31; // ORANGE IF SAME AS VIDEO
		} else {
			color = 50; // ELSE YELLOW
		}

		setLED(2, 64 + activePad, color); // SET NEW COLOR

		// UNSET OLD PAD
		if(selectedPadImage == selectedPadVideo) {
			color = 3; // HAS IMAGE, SET TO RED
		} else if(selectedPadImage != activePad) {
			color = 0;
		}

		setLED(2, 64 + selectedPadImage, color); // SET TO BLANK
		selectedPadImage = activePad;

		image.loadImage(images[selectedPadImage]);

		if(!loadedImage) {
				loadedImage = true;
		}
	}
	doLoadNewVideo = false;
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer& buffer) {
	ofScopedLock lock(audioMutex);
	this->inputBuffer = buffer;

	float curVol = 0.0;
	int numCounted = 0;

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume
	for(int i = 0 ; i < buffer.getNumFrames(); i++){
	  curVol += buffer[i] * buffer[i] * 0.25;
	  numCounted += 2;
	}

	//this is how we get the mean of rms :)
	curVol /= (float)numCounted;
	curVol = sqrt( curVol );

	smoothedVol = smoothValue(curVol, smoothedVol);
	smoothedVolLite = curVol;
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& buffer) {
	ofScopedLock lock(audioMutex);
  this->inputBuffer.copyTo(buffer);
}

void ofApp::setLED(int channel, int button, int color) {
		midiOut.sendControlChange(channel, button, color);
		midiOut.sendNoteOn(channel, button, color);

		ofLogNotice() << "channel: " << channel << " note: " << button << " velocity: " << color;
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
	midiOut.closePort();

	midiIn.closePort();
	midiIn.removeListener(this);
}
