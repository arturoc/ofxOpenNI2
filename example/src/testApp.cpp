#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofSetLogLevel(ofxOpenNI::LOG_NAME,OF_LOG_VERBOSE);
	ofSetLogLevel(ofxOpenNICapture::LOG_NAME,OF_LOG_VERBOSE);

	bool live = true;

	if(live){
		openNI.setupFromXML("openni/config/ofxopenni_config.xml",false);
	}else{
		openNI.setupFromRecording("recording.oni");
	}

	bRecording = false;

}

//--------------------------------------------------------------
void testApp::update(){
	openNI.update();
	if(bRecording){
		recorder.update();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	openNI.draw(0,0);
	openNI.drawRGB(640,0);
	ofDrawBitmapString(ofToString(ofGetFrameRate()),20,20);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if(key=='r'){
		bRecording = !bRecording;
		if(bRecording){
			recorder.setup(openNI,"recording.oni");
			recorder.startCapture();
		}else{
			recorder.stopCapture();
		}
	}

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

