#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		// Basic Audio Setup
		
		ofSoundStream soundStream;
		void audioIn(ofSoundBuffer & input);
		void audioOut(ofSoundBuffer & output);
		int bufferSize;
		vector<float> audioBuffer;
		float wetMix;
		float dryMix;
		int sampleRate;
				
		// Incoming Arduino serial data
		string msg;
		char byteData;
			ofSerial serial;
		vector<int> serialData;
		
		// Delay
		vector<float> delayBuffer;
		int recordHead;
		int playHead;
		float delayMillis;
		int delaySamples;
		float feedback;
		
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
