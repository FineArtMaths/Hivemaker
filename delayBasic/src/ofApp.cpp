#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowShape(100, 100);
    ofSetFrameRate(5);
	ofSetVerticalSync(true);
	ofSetCircleResolution(80);
    ofDisableAntiAliasing();
    ofBackground(0);
    ofSetColor(ofColor(0, 255, 255));
    ofNoFill();
    ofSetLineWidth(1.0f);
	
	// Arduino Serial input
    serial.setup("/dev/ttyUSB0", 9600);
    serialData = vector<int>();
    serialData.resize(3);
    serialData[0] = 0;
    serialData[1] = 0;
    serialData[2] = 0;

	// Delay
	delayMillis = 1000;
	feedback = 0.7;
	delaySamples = 32000 * delayMillis/1000;
	recordHead = delaySamples;
	playHead = 0;
	delayBuffer.resize(delaySamples * 2);
	cout << "Setup complete" << endl;

	// Basic sound setup

	bufferSize = 128;
	audioBuffer.resize(bufferSize);
	
	ofSoundStreamSettings settings;
	auto devices = soundStream.getMatchingDevices("default");
	if(!devices.empty()){
		settings.setInDevice(devices[0]);
	}

	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 32000;
	settings.numInputChannels = 1;
	settings.numOutputChannels = 1;
	settings.bufferSize = bufferSize;
	soundStream.setup(settings);
	
}

//--------------------------------------------------------------
void ofApp::update(){
	// Serial read
    //Simple if statement to inform user if Arduino is sending serial messages. 
    if (serial.available() < 0) {
        msg = "Arduino Error";
    } else {
        //While statement looping through serial messages when serial is being provided.
		msg = "";
		int serialDataIdx = 0;
		while (serial.available() > 0) {
			byteData = serial.readByte();
			if ( byteData != OF_SERIAL_NO_DATA && byteData != OF_SERIAL_ERROR ){
				// if comma, store in next serialData and increment serialDataIdx
				if(byteData == 44 || byteData == 42 || byteData == 124){
					if(byteData == 124){
/*						try{
							mode = stoi(msg);
						} catch (...){
							cout << "Error on mode: ";
							cout << msg << endl;
						}						*/
						msg = "";
					} else if(msg.length() > 0){
						serialData[serialDataIdx] = stoi(msg);
						serialDataIdx++;
						msg = "";
					}

					if(byteData == 42){
						serialDataIdx = 0;
					}
				} else {
					if(byteData > 47 && byteData < 58){
						msg.append(1, char(byteData));
					}
				}
			}
		}
		wetMix = float(serialData[2])/1024.0;
		dryMix = 1 - wetMix;
		feedback = ofMap(serialData[1], 0, 1024, 0, 1);
		float prevMillis = delayMillis;
		delayMillis = ofMap(serialData[0], 0, 1024, 40, 1000);
		if(abs(prevMillis - delayMillis) > 5){
			delaySamples = 32000.0 * delayMillis/1000.0;
			delayBuffer.resize(delaySamples * 2);
			//fill(delayBuffer.begin(), delayBuffer.end(), 0);
			cout << "recordHead: " << recordHead << ", ";
			cout << "playHead: " << playHead << ", " << endl;
			while(recordHead > delayBuffer.size()){
				recordHead -= delayBuffer.size();
			}
			playHead = recordHead - delaySamples;
			cout << "playHead: " << playHead << ", " << endl;
			while(playHead < 0){
				playHead += delayBuffer.size();
			}
			cout << "recordHead: " << recordHead << ", ";
			cout << "playHead: " << playHead << ", ";
			cout << "delayBuffer.size(): " << delayBuffer.size() << ", ";
		}
		cout << "delayMillis: " << delayMillis << ", ";
		cout << "feedback: " << feedback << ", ";
		cout << "Mix: " << wetMix << endl;
	}


}

//--------------------------------------------------------------
void ofApp::draw(){

}

void ofApp::audioIn(ofSoundBuffer & input){
	for (size_t i = 0; i < input.size(); i++){
		audioBuffer[i] = input[i];
		int recordHeadOther = recordHead + delaySamples;
		while(recordHeadOther > delayBuffer.size()){
			recordHeadOther -= delayBuffer.size();
		}
		delayBuffer[recordHead] = (input[i] + feedback * delayBuffer[recordHead] + feedback*delayBuffer[recordHeadOther])/(1 + 2*feedback);
		recordHead++;
		if(recordHead >= delayBuffer.size()){
			recordHead = 0;
		}
	}
}

void ofApp::audioOut(ofSoundBuffer & output){
	for (size_t i = 0; i < output.size(); i++){
		playHead++;
		if(playHead >= delayBuffer.size()){
			playHead = 0;
		}
		float samp = delayBuffer[playHead];
		output[i] = wetMix * samp + dryMix * audioBuffer[i];
		
	}
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
