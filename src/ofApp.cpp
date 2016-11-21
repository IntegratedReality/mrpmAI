#include "ofApp.h"

const std::array<std::string, 4>
robot{
  "mrpmpi1.local",
  "mrpmpi2.local",
  "mrpmpi3.local",
  "mrpmpi4.local"
};
const int PORT_ROBOT = 8000;
const int PORT_AI = 8000;
const std::string localhost = "127.0.0.1";

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(30);
	up = down = right = left = false;

	rcvr.setup(PORT_AI);

	for (int i = 0; i < 4; i++)
	{
		ai[i].init(i);
	}

	gui.setup();
	gui.add(isAI[0].setup("Pi1 is AI?", false));
	gui.add(isAI[1].setup("Pi2 is AI?", false));
	gui.add(isAI[2].setup("Pi3 is AI?", false));
	gui.add(isAI[3].setup("Pi4 is AI?", false));
	gui.add(LIMIT_TOP_TR.setup("LIMIT TOP TR", 0.2, 0.01, 1.0));
	gui.add(LIMIT_TR_RIGHT.setup("LIMIT TR RIGHT", 1.0, 0.01, 1.0));
	gui.add(LIMIT_TOP_TL.setup("LIMIT TOP TL", -0.2, -1.0, -0.01));
	gui.add(LIMIT_TL_LEFT.setup("LIMIT TL LEFT", -1.0, -1.0, -0.01));
	gui.add(LIMIT_SHOT_ANGLE.setup("LIMIT SHOT ANGLE", 0.05, 0.01, 0.2));
	gui.add(DIFF_MOVE.setup("DIFF_MOVE", 100, 10, 300));
	gui.add(DIST_TO_TARGET.setup("DIST_TO_TARGET", 525, 225, 725));
	gui.add(RATE_OF_TARGET.setup("RATE_OF_TARGET", 1.0, 0.1, 2.0));
	gui.add(RATE_OF_OBST.setup("RATE_OF_OBST", 0.2, 0.1, 1.0));
	gui.add(RATE_OF_LENGTH_T.setup("RATE_OF_LENGTH_T", 300, 100, 600));
	gui.add(RATE_OF_LENGTH_O.setup("RATE_OF_LENGTH_O", 100, 10, 300));
}



//--------------------------------------------------------------
void ofApp::update() {
	receiveMessage();

	double floatparams[9];
	floatparams[0] = LIMIT_TOP_TR;
	floatparams[1] = LIMIT_TR_RIGHT;
	floatparams[2] = LIMIT_TOP_TL;
	floatparams[3] = LIMIT_TL_LEFT;
	floatparams[4] = LIMIT_SHOT_ANGLE;
	floatparams[5] = RATE_OF_TARGET;
	floatparams[6] = RATE_OF_OBST;
	floatparams[7] = (double)RATE_OF_LENGTH_T * (double)RATE_OF_LENGTH_T;
	floatparams[8] = (double)RATE_OF_LENGTH_O * (double)RATE_OF_LENGTH_O;

	int intparams[2];
	intparams[0] = DIST_TO_TARGET;
	intparams[1] = DIFF_MOVE;

	for (int i = 0; i < 4; i++) {
		ai[i].setFloatParams(floatparams);
		ai[i].setIntParams(intparams);
	}

	if (isSenderReady) {
		ofxOscMessage m;
		m.setAddress("/operator/operation");
		m.addInt32Arg(getDrc());

		for(int i = 0; i < 4; i++){
			sndr[i].sendMessage(m);
		}

		ofxOscMessage x;
		x.setAddress("/main/toRobot");
		x.addInt32Arg(0);
		x.addDoubleArg(0.f);
		x.addDoubleArg(0.f);
		x.addDoubleArg(0.f);
		for (int i = 0; i < 9; ++i) {
			x.addBoolArg(true);
		}
		for (int i = 0; i < 4; i++) {
			sndr[i].sendMessage(x);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	static int width = ofGetWidth(), height = ofGetHeight();

	std::string str;
	if (isSenderReady) {
		ofSetColor(ofColor::black);
		str = robot[senderRobInd] + ":" + ofToString(PORT_ROBOT);
	}
	else {
		str = "Press 1-4.";
	}
	ofDrawBitmapString(str, 0, 30);

	ofPushMatrix();
	ofTranslate(width / 2, height / 2);
	ofSetColor(ofColor::red);
	if (!(up || down || left || right)) {
		ofDrawCircle(0, 0, 30);
	}
	else {
		ofRotate(getRot());
		ofSetLineWidth(10);
		ofDrawLine(0, 0, 0, -100);
		ofDrawCircle(0, -110, 20);
	}
	ofPopMatrix();

	gui.draw();
}


int32_t ofApp::getDrc() {
	if (up) {
		if (right)return 2;
		else if (left) return 8;
		else return 1;
	}
	else if (down) {
		if (right)return 4;
		else if (left)return 6;
		else return 5;
	}
	else if (right) {
		return 3;
	}
	else if (left) {
		return 7;
	}
	return 0;
}
float ofApp::getRot() {
	if (up) {
		if (right)return 45;
		else if (left) return -45;
		else return 0;
	}
	else if (down) {
		if (right)return 135;
		else if (left)return -135;
		else return 180;
	}
	else if (right) {
		return 90;
	}
	else if (left) {
		return -90;
	}
	return 0;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case OF_KEY_UP:
		up = true;
		break;
	case OF_KEY_DOWN:
		down = true;
		break;
	case OF_KEY_LEFT:
		left = true;
		break;
	case OF_KEY_RIGHT:
		right = true;
		break;
	case OF_KEY_RETURN:
		if (!isSenderReady)setupSender();
		break;
	case OF_KEY_BACKSPACE:
		if (isSenderReady)deleteSender();
		break;
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {
	case OF_KEY_UP:
		up = false;
		break;
	case OF_KEY_DOWN:
		down = false;
		break;
	case OF_KEY_LEFT:
		left = false;
		break;
	case OF_KEY_RIGHT:
		right = false;
		break;
	}
}

void ofApp::setupSender() {
	if (isSenderReady) return;
	std::cout << "set up Sender" << std::endl;
	for (int i = 0; i < 4; i++) {
		ofxOscSender sendtmp;
		if (isAI[i]) {
			sendtmp.setup(robot[i], PORT_ROBOT);
		}
		else {
			sendtmp.setup(localhost, PORT_ROBOT);
		}
		sndr.push_back(sendtmp);
	}
	isSenderReady = true;
}

void ofApp::deleteSender() {
	if (isSenderReady) {
		std::cout << "Delete Sender" << std::endl;
		sndr.clear();
		isSenderReady = false;
	}
}

void ofApp::receiveMessage()
{
	while (rcvr.hasWaitingMessages()) {
		ofxOscMessage m;
		if (m.getAddress() == ("/main/toAI/allpos")) {
			for (int i = 0; i < 4; i++) {
				auto& d = ai[0].data[i];

			}
		}

	}
}

void ofApp:assignPosData()

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
