#include "ofApp.h"

const std::array<std::string, 4>
robot{
  "mrpmpi1.local",
  "mrpmpi2.local",
  "mrpmpi3.local",
  "mrpmpi4.local"
};
const int PORT_ROBOT = 8000;
const int PORT_OPERATOR = 8001;
const std::string localhost = "127.0.0.1";

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(30);
	up = down = right = left = false;

	rcvr.setup(PORT_OPERATOR);

	for (int i = 0; i < 4; i++)
	{
		ai[i].init(i);
	}

	gui.setup();
	gui.add(connect[0].setup("connect Pi1?", false));
	gui.add(connect[1].setup("connect Pi2?", false));
	gui.add(connect[2].setup("connect Pi3?", false));
	gui.add(connect[3].setup("connect Pi4?", false));
	gui.add(isAI[0].setup("Pi1 is AI?", false));
	gui.add(isAI[1].setup("Pi2 is AI?", false));
	gui.add(isAI[2].setup("Pi3 is AI?", true));
	gui.add(isAI[3].setup("Pi4 is AI?", true));
	gui.add(isDodge[0].setup("Pi1 choose Dodge route?", false));
	gui.add(isDodge[1].setup("Pi2 choose Dodge route?", false));
	gui.add(isDodge[2].setup("Pi3 choose Dodge route?", true));
	gui.add(isDodge[3].setup("Pi4 choose Dodge route?", true));
	
    
    gr_duty.setup();
    gr_duty.setName("Duty Coeffs");
	gr_duty.add(DUTY[0].setup("Pi1 duty coefficient", ofVec2f(1.0, 1.0), ofVec2f(0.1, 0.1), ofVec2f(3.0, 3.0)));
	gr_duty.add(DUTY[1].setup("Pi2 duty coefficient", ofVec2f(1.0, 1.0), ofVec2f(0.1, 0.1), ofVec2f(3.0, 3.0)));
	gr_duty.add(DUTY[2].setup("Pi3 duty coefficient", ofVec2f(1.0, 1.0), ofVec2f(0.1, 0.1), ofVec2f(3.0, 3.0)));
	gr_duty.add(DUTY[3].setup("Pi4 duty coefficient", ofVec2f(1.0, 1.0), ofVec2f(0.1, 0.1), ofVec2f(3.0, 3.0)));
    gui.add(&gr_duty);

    gr_dodgeConsts.setup();
    gr_dodgeConsts.setName("CONSTs");
    gr_dodgeConsts.add(LIMIT_TOP_TR.setup("LIMIT TOP TR", 0.2, 0.01, 1.0));
	gr_dodgeConsts.add(LIMIT_TR_RIGHT.setup("LIMIT TR RIGHT", 1.0, 0.01, 1.0));
	gr_dodgeConsts.add(LIMIT_TOP_TL.setup("LIMIT TOP TL", -0.2, -1.0, -0.01));
	gr_dodgeConsts.add(LIMIT_TL_LEFT.setup("LIMIT TL LEFT", -1.0, -1.0, -0.01));
	gr_dodgeConsts.add(LIMIT_SHOT_ANGLE.setup("LIMIT SHOT ANGLE", 0.05, 0.01, 0.2));
	gr_dodgeConsts.add(DIFF_MOVE.setup("DIFF_MOVE", 100, 10, 300));
	gr_dodgeConsts.add(DIST_TO_TARGET.setup("DIST_TO_TARGET", 525, 225, 725));
	gr_dodgeConsts.add(RATE_OF_TARGET.setup("RATE_OF_TARGET", 1.0, 0.1, 2.0));
	gr_dodgeConsts.add(RATE_OF_OBST.setup("RATE_OF_OBST", 0.2, 0.1, 1.0));
	gr_dodgeConsts.add(RATE_OF_LENGTH_T.setup("RATE_OF_LENGTH_T", 300, 100, 600));
	gr_dodgeConsts.add(RATE_OF_LENGTH_O.setup("RATE_OF_LENGTH_O", 100, 10, 300));
    gui.add(&gr_dodgeConsts);
    
    gr_targets.setup();
    gr_targets.setName("Target Coords");
    for(int i=0; i<4; i++){
        std::string t="Target for Pi"+ofToString(i);
        gr_targets.add(targetSlider[i].setup
                       (t,
                        ofVec2f(ofRandom(WIDTH_OF_FIELD),ofRandom(HEIGHT_OF_FIELD)),
                        ofVec2f(0,0),
                        ofVec2f(WIDTH_OF_FIELD,HEIGHT_OF_FIELD)
                        ));
    
        std::string s="Pi"+ofToString(i)+" auto drive";
        gr_targets.add(autoDrive[i].setup(s,false));
    }
    gui.add(&gr_targets);
    
    gui.minimizeAll();
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
        ai[i].setRoute(isDodge[i] ? DODGE : DIRECT);
        
        if(autoDrive[i]){
            ai[i].update();}
        else{
            ofVec2f f=targetSlider[i];
            ai[i].setTargetManually(f.x, f.y);
        }
	}

	if (isSenderReady) {
		for (int i = 0; i < 4; i++) {
			ofxOscMessage m;
			m.setAddress("/operator/operation");
			if (isAI[i]) {
				m.addInt32Arg(ai[i].getOperation().direction);
			}
			else {
				m.addInt32Arg(getDrc());
			}
			sndr[i].sendMessage(m);
		}

		for (int i = 0; i < 4; i++) {
			if (!isAI[i]) continue;
			ofxOscMessage m2;
			m2.setAddress("/operator/shot");
            m2.addInt32Arg(i);
			m2.addBoolArg(ai[i].getOperation().shot);
			sndr[i].sendMessage(m2);
		}

		for (int i = 0; i < 4; i++) {
			if (!connect[i]) continue;
			ofxOscMessage m2;
			m2.setAddress("/operator/duty");
			ofVec2f duties = DUTY[i];
			m2.addDoubleArg(duties.x);
			m2.addDoubleArg(duties.y);
			sndr[i].sendMessage(m2);
		}

		//ofxOscMessage x;
		//x.setAddress("/main/toRobot");
		//x.addInt32Arg(0);
		//x.addDoubleArg(0.f);
		//x.addDoubleArg(0.f);
		//x.addDoubleArg(0.f);
		//for (int i = 0; i < 9; ++i) {
		//	x.addBoolArg(true);
		//}
		//for (int i = 0; i < 4; i++) {
		//	sndr[i].sendMessage(x);
		//}
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
    } else {
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
		if (connect[i]) {
			sendtmp.setup(robot[i], PORT_ROBOT);
		}
		else {
			sendtmp.setup(localhost, 8010);
		}
		sndr.push_back(sendtmp);
	}
	isSenderReady = true;
	std::cout << "FInish setting up Sender" << std::endl;
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
			for (int j = 0; j < 4; j++) {
				auto tmpx = m.getArgAsDouble(3 * j);
				auto tmpy = m.getArgAsDouble(3 * j + 1);
				auto tmpth = m.getArgAsDouble(3 * j + 2);

				for (int i = 0; i < 4; i++) {
					auto& d = ai[i].data[j];
					d.id = j;
					d.pos.x = tmpx;
					d.pos.y = tmpy;
					d.pos.theta = tmpth;
				}
			}
		}
		else if (m.getAddress() == "/main/toAI/POowner") {
			for (int j = 0; j < 3; j++) {
				auto team = (ETeam)m.getArgAsInt(j);

				for (int i = 0; i < 4; i++) {
					ai[i].setPOOwner(j, team);
				}
			}
		}
		else if (m.getAddress() == "/main/toAI/gameState") {
			gamestate = (EMode)m.getArgAsInt(0);
		}
	}
}


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
