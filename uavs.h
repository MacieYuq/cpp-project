#pragma once
#include <thread>
#include <string>
#include <atomic>


using namespace std;

class ECE_UAV
{
public:
	void start();
	friend void threadFunction(ECE_UAV* pUAV);

	double position[3] = {};
	double iniPosition[3] = {};


	atomic<bool> m_stop = false;
	atomic<bool> m_sphere = false;
	atomic<bool> start_timer = false;

	double mass = 1;
	double Velocity = 0;
	double force = 0;
	
	double velocity[3] = {};
	double acceleration[3] = {};
	double tangent[3] = {};
	double sphereVelocity[3] = {1, 1, 1};

	double spring[3] = {};
	int dir[2] = {};

	thread kinematicsThread;
 };