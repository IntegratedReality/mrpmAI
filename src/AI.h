#pragma once

#include "Const.h"
#include "Operation.h"
#include "RobotData.h"
#include "Position.h"
#include "EMode.h"

enum ERoute {
	DIRECT,
	DODGE,
};

enum EStrategy {
	RANDOM,
	SIMPLE,
};

#ifndef M_PI
static const double M_PI = 3.14159;
#endif

class AI {
public:
	void init(int _myid);
	void setFloatParams(double[]);
	void setIntParams(int[]);
	void setStrategy(EStrategy _estr);
    void setRoute(ERoute _erou);
    
    void setTargetManually(float x, float y);

	void update();
	void setRobotData(int _id, RobotData _data);
	void setPOOwner(int _id, ETeam _owner);
	Operation getOperation();

	void developRandomRoute();
	void developDirectRoute();
	void developDodgeRoute();

	void developSimpleStrategy();
	RobotData data[NUM_OF_ROBOT];
    

protected:
	Operation operation;
	ETeam owner[NUM_OF_POINT_OBJ];

private:
	int Myid;
	ERoute erou;
	EStrategy estr;
	Position target;
	Position POPos[NUM_OF_POINT_OBJ];

	double LIMIT_TOP_TR = 0.2;
	double LIMIT_TR_RIGHT = 1.0;
	double LIMIT_TOP_TL = -0.2;
	double LIMIT_TL_LEFT = -1.0;
	double LIMIT_SHOT_ANGLE = 0.05;

	int DIST_TO_TARGET = 300 + 125 + 100;
	int DIFF_MOVE = 100;
	double RATE_OF_TARGET = 1.0;
	double RATE_OF_OBST = 0.2;
	double RATE_OF_LENGTH_T = 300 * 300;
	double RATE_OF_LENGTH_O = 100 * 100;
};
