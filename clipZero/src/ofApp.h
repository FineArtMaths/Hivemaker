#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		// Audio Thru
		ofSoundStream soundStream;
		void audioIn(ofSoundBuffer & input);
		void audioOut(ofSoundBuffer & output);
		int bufferSize;
		vector<float> audioBuffer;
		vector<float> audioBufferPrev;
		float wetMix;
		float dryMix;
		int sampleRate;
		
		// Incoming Arduino serial data
		string msg;
		char byteData;
			ofSerial serial;
		vector<int> serialData;
		
		
		
		// Modes
		int mode;
		vector<string> modeNames;
		
		// clipZero
		float clipThresh;
		bool clipToZero;
		float clampToPoint;
		
		// Riffler
		int riffleModulus;
		
		// Absolute
		int absSkip;
		float absMix;
		
		// Wavefolder
		float foldAmount;
		float folderThresh;
		
		// Lerper
		int lerpSteps;
		float lerpFrom;
		float lerpTo;
		float lerpAmtPerSample;
		int currLerpT;
		
		//Microcomb
		int combDelay;

		// Bit crusher
		float bcBitDepth;
		float bcSampleRate;
		int bcSamplePtr;
		float bcPrevSampleValue;
		int bcSampleCopy;

		// Ring mod
		float getOscSample(const float& t);
		void setCarrierFreq(float freq);
		float getFreq();

		float carrierT;
		float carrierFreq;
		float carrierIncrPerSample;
		int lfoTick;
		
		// Square
		float pulseWidth;
		float middle;
		
		// Interaction
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
};
