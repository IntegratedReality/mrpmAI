#pragma once

#include <vector>
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxGui.h"
#include "AI.h"


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

		ofxPanel gui;
		
		ofxToggle isAI[4];
		ofxFloatSlider LIMIT_TOP_TR;
		ofxFloatSlider LIMIT_TR_RIGHT;
		ofxFloatSlider LIMIT_TOP_TL;
		ofxFloatSlider LIMIT_TL_LEFT;
		ofxFloatSlider LIMIT_SHOT_ANGLE;

		//for dodge route
		ofxIntSlider DIFF_MOVE;
		ofxIntSlider DIST_TO_TARGET;
		ofxFloatSlider RATE_OF_TARGET;
		ofxFloatSlider RATE_OF_OBST;
		ofxIntSlider RATE_OF_LENGTH_T;
		ofxIntSlider RATE_OF_LENGTH_O;

	private:
		bool isSenderReady{false};
		int senderRobInd{0};
		bool up, down, left, right;
		std::vector<ofxOscSender> sndr;
		int32_t getDrc();
		float getRot();

		void setupSender();
		void deleteSender();

		AI ai[4];
};
