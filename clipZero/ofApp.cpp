#include "ofApp.h"

/*
 * A collection of "digital distortion" effects
 * that sound like a bitcrusher.
 * 
 */

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
	
	// Basic Audio Setup
	
	bufferSize = 128;
	audioBuffer.resize(bufferSize * 2);
	sampleRate = 32000;
	
	ofSoundStreamSettings settings;
	auto devices = soundStream.getMatchingDevices("default");
	if(!devices.empty()){
		settings.setInDevice(devices[0]);
	}

	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = sampleRate;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	soundStream.setup(settings);
	
	// Arduino Serial input
    serial.setup("/dev/ttyUSB0", 9600);
    serialData = vector<int>();
    serialData.resize(3);
    serialData[0] = 0;
    serialData[1] = 0;
    serialData[2] = 0;

	// Modes setup
	mode = 5;
	modeNames.resize(10);
	modeNames[0] = "Clip Low";
	modeNames[1] = "Clip High";
	modeNames[2] = "Absolute";
	modeNames[3] = "Riffler";
	modeNames[4] = "Wavefolder";
	modeNames[5] = "Lerper";
	modeNames[6] = "Microcomb";
	modeNames[7] = "Bitcrusher";
	modeNames[8] = "Ring Modulator";
	modeNames[9] = "square";
	
	// Param Setup
	clipToZero = false;
	clampToPoint = 0.0f;
	clipThresh = 0.03f;	// About 0.03 is good. 0.01 has almost no effect; 0.5 is inaudible
	if(clipToZero){
		clipThresh = 1 - clipThresh;
	}
	riffleModulus = 3;
	absSkip = 0;
	foldAmount = 0.2;
	folderThresh = 0.95;
	lerpSteps =100;
	lerpFrom = 99.0f;
	lerpTo = 0.0f;
	currLerpT = 0;
	lerpAmtPerSample = 0.0f;
	combDelay = 64;
	easeAmt = 32;

	// Bit Crusher Setup
	bcBitDepth = 1024;		// max = 1024 but NB this does still introduce a small amount of noise.
	bcSampleRate = sampleRate;	// max = sampleRate of the soundStream
	bcSamplePtr = 0;
	bcPrevSampleValue = 0;
	bcSampleCopy = int(floor(settings.sampleRate / bcSampleRate));
	
	// Ring Mod Setup
	carrierT = 0.0f;
	setCarrierFreq(10000);
	lfoTick = 0;
	wetMix = 1.0;
	dryMix = 0.0;
	
	// Square setup
	pulseWidth = 0;
	
	cout << "Mode: " << modeNames[mode] << endl;
}

//--------------------------------------------------------------
// Ring modulator helper functions
//--------------------------------------------------------------

void ofApp::setCarrierFreq(float freq){
	carrierFreq = freq;
	carrierIncrPerSample = TWO_PI * carrierFreq / sampleRate;
	cout << "carrierFreq: " << carrierFreq << endl;
}

float ofApp::getFreq(){
	if(lfoTick < 500){
		return 50 + lfoTick * 10;
	} else {
		return 5000 - (lfoTick - 500) * 10;
	}
}

float ofApp::getOscSample(const float& t){
	return 0.65*sin(t) + 0.3*sin(2.5*t);
}


//--------------------------------------------------------------
// Main code
//--------------------------------------------------------------

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
						try{
							mode = stoi(msg);
						} catch (...){
							cout << "Error on mode: ";
							cout << msg << endl;
						}						
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
		if(mode == 0){
			clipThresh = ofMap(serialData[0], 0, 1024, 0.001, 0.4);
			clampToPoint = ofMap(serialData[1], 0, 1024, 0, 1);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "clipThresh: " << clipThresh << ", ";
			cout << "clampToPoint: " << clampToPoint << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 1){
			clipThresh = ofMap(serialData[0], 0, 1024, 0.001, 0.4);
			clampToPoint = ofMap(serialData[1], 0, 1024, 0, 1);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "); ";
			cout << "clipThresh: " << clipThresh << ", ";
			cout << "clampToPoint: " << clampToPoint << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 2){
			absSkip = floor(serialData[0]/5)*2;
			absMix = ofMap(serialData[1], 0, 1024, 0, 1);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "absSkip: " << absSkip << ", ";
			cout << "absMix: " << absMix << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 3){
			riffleModulus = ofMap(serialData[0], 0, 1024, 1, bufferSize);
			riffleModulus += ofMap(serialData[1], 0, 1024, 1, 10);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "riffleModulus: " << riffleModulus << ", ";
			//cout << "UNUSED: " << clampToPoint << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 4){
			foldAmount = ofMap(serialData[0], 0, 1024, 0, 2);
			folderThresh = ofMap(serialData[1], 0, 1024, 0.1, 1);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "foldAmount: " << foldAmount << ", ";
			cout << "folderThresh: " << folderThresh << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 5){
			lerpSteps = ofMap(serialData[0], 0, 1024, 1, 120);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "lerpSteps: " << lerpSteps << ", ";
			//cout << "UNUSED: " << clampToPoint << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 6){
			combDelay = ofMap(serialData[0], 0, 1024, 1, bufferSize/2);
			combDelay = floor(combDelay) * 2;
			easeAmt = floor(ofMap(serialData[1], 0, 1024, 0, bufferSize/2));
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "combDelay: " << combDelay << ", ";
			//cout << "UNUSED: " << clampToPoint << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 7){
			bcBitDepth = serialData[0]/10;
			bcSampleRate = serialData[1] * 32;
			bcSampleCopy = ceil(sampleRate / bcSampleRate);
			combDelay = ofMap(serialData[0], 0, 1024, 1, 200);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "bcBitDepth: " << bcBitDepth << ", ";
			cout << "bcSampleRate: " << bcSampleRate << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 8){
			float fine = serialData[1];
			float coarse = serialData[0];
			if(coarse < 6){
				coarse = 0;
				fine = fine /5;
			}
			setCarrierFreq(coarse*3 + fine/5);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "fine: " << fine << ", ";
			cout << "coarse: " << coarse << ", ";
			cout << "Mix: " << wetMix << endl;
		} else if(mode == 9){
			pulseWidth = ofMap(serialData[0], 0, 1024, 0.0018, 0.08);
			middle = ofMap(serialData[1], 0, 1024, 0, 1);
			cout << "Mode: " << modeNames[mode] << " (" << mode << "): ";
			cout << "pulseWidth: " << pulseWidth << ", ";
			cout << "middle: " << middle << ", ";
			cout << "Mix: " << wetMix << endl;
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw(){

}

void ofApp::audioIn(ofSoundBuffer & input){
	audioBufferPrev = audioBuffer;
	for (size_t i = 0; i < input.size(); i++){
		audioBuffer[i] = input[i];
	}
}


void ofApp::audioOut(ofSoundBuffer & output){
	for (size_t i = 0; i < output.size(); i++){
		if (mode == 3){
			// Riffler
			for(int r = 0; r < riffleModulus; r++){
				if(i % riffleModulus == r){
					int idx = i/riffleModulus + r*audioBuffer.size()/riffleModulus;
					float samp = audioBuffer[i];
					output[idx] = wetMix * samp + dryMix * audioBuffer[idx];
				}
			}
		} else {
			float samp = 0.0;
			if(mode == 0){
				// Clip Zero
				if(abs(audioBuffer[i]) < clipThresh){
					samp = clampToPoint;
				} else {
					samp = audioBuffer[i];
				}
			} else if(mode == 1){
				// Clip One
				if(audioBuffer[i] > clipThresh){
					samp = clampToPoint;
				}else if(audioBuffer[i] < -1*clipThresh){
					samp = -1*clampToPoint;
				} else {
					samp = audioBuffer[i];
				}
			} else if (mode == 2){
				// Absolute
				if(abs(audioBuffer[i]) < 0.001){
					samp = 0.0;
				} else {
					float avg = 0.0f;
					for(int f = 0; f < audioBuffer.size(); f++){
						avg += audioBuffer[f];
					}
					avg /= (2 * audioBuffer.size());
					float a = abs(audioBuffer[i] * 2);
					samp = a - avg;
					float samp2 = 1.0f;
					if(i - absSkip > 0){
						samp2 = audioBuffer[i - absSkip];
					} else {
						samp2 = audioBufferPrev[audioBufferPrev.size() + i - absSkip];
					}
					samp = absMix * samp * samp2 + (1 - absMix) * samp;
				}
			} else if (mode == 4){
				// Wavefolder
				samp = audioBuffer[i] * (1.0 + foldAmount);
				while(samp < -1*folderThresh || samp > folderThresh){
					if(samp > folderThresh){
						samp = folderThresh - samp;
					}else if (samp < -1*folderThresh){
						samp = folderThresh + samp;
					} 
				}
			} else if (mode == 5){
				// Lerper
				int lerpStepSize = output.size()/lerpSteps;
				int prevLerpStep = lerpStepSize * floor(i/lerpStepSize);
				int nextLerpStep = prevLerpStep + lerpStepSize;
				if(nextLerpStep > output.size() - 1){
					nextLerpStep = output.size() - 1;
				}
				samp = audioBuffer[prevLerpStep] + (audioBuffer[nextLerpStep] - audioBuffer[prevLerpStep])*float(i)/output.size();
			} else if (mode == 6){
				// Microcomb
				int offset = i - combDelay;
				if(offset < 0){
					offset += audioBuffer.size();
				}
				float easer = 1.0f;
				if(i < easeAmt){
					easer = float(i)/easeAmt;
				} else if (i > audioBuffer.size() - easeAmt){
					easer = float(audioBuffer.size() - i)/easeAmt;
				}
				samp = (audioBuffer[i]  + audioBuffer[offset] * easer)/(1 + easer);
			} else if (mode == 7){
				if(bcSamplePtr == 0){
					bcPrevSampleValue = audioBuffer[i];
				}
				if(abs(audioBuffer[i]) < 0.005){
					samp = 0.0;
				} else {
					samp = floor(bcPrevSampleValue * bcBitDepth)/bcBitDepth;
				}
				bcSamplePtr++;
				if(bcSamplePtr >= bcSampleCopy){
					bcSamplePtr = 0;
				}
			} else if (mode == 8){
				if(abs(audioBuffer[i]) < 0.001){
					samp = 0.0;
				} else {
					samp = audioBuffer[i] * getOscSample(carrierT);
				}
				carrierT += carrierIncrPerSample;
				if(carrierT > TWO_PI){
					carrierT -= TWO_PI;
				}
			} else if (mode == 9){
				if(audioBuffer[i] + pulseWidth > 0){
					samp = 1.0;
				} else if(audioBuffer[i] + pulseWidth + middle > 0){
					samp = 0.0;
				} else {
					samp = -1.0;
				}
			}
				
			output[i] = wetMix * samp + dryMix * audioBuffer[i];
		}
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
