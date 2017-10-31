#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCvHaarFinder.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		int camWidth, camHeight;
		ofVideoGrabber vidGrabber;
		ofxCvHaarFinder finder;

		ofxCvColorImage rgb, hsb;
		ofxCvGrayscaleImage hue, sat, bri, filter1, filter2, finalImageL, finalImageR;
		ofxCvContourFinder contoursL, contoursR;

		int findHueL, findSatL, findSatR, findHueR;

		ofImage cat;
		vector<ofImage> images;
		vector<ofImage> pumpkins;
		ofPoint lefty, righty;
		
		ofVec3f p;
		ofVec3f upper;
		vector<pair<ofVec3f,int>> past;
		vector<ofVec2f> broken;

		ofVec2f temp;

		float slope, b, s, x, y;

		int speed;
		bool draw_p, erase_p;
		ofTrueTypeFont font;
};
