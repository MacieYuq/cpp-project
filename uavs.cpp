#include <chrono>
#include "uavs.h"
#include <cmath>
#include <random>
#include <ctime>
#include <iostream>

using namespace std;
void threadFunction(ECE_UAV* pUAV)
{

	double d_x = pow(0 - pUAV->iniPosition[0], 2);
	double d_y = pow(0 - pUAV->iniPosition[1], 2);
	double d_z = pow(50 - pUAV->iniPosition[2], 2);

	//copy the initial position
	pUAV->position[0] = pUAV->iniPosition[0];
	pUAV->position[1] = pUAV->iniPosition[1];
	pUAV->position[2] = pUAV->iniPosition[2];

	//Initial force vector
	pUAV->acceleration[0] = (0 - pUAV->iniPosition[0]) * 0.5;
	pUAV->acceleration[1] = (0 - pUAV->iniPosition[1]) * 0.5;
	pUAV->acceleration[2] = (50 - pUAV->iniPosition[2]) * 0.5;
	

	this_thread::sleep_for(chrono::seconds(5));

	do
	{
		//calculate the distance between the uav and the surface of the sphere
		double s_x = pow(0 - pUAV->position[0], 2);
		double s_y = pow(0 - pUAV->position[1], 2);
		double s_z = pow(50 - pUAV->position[2], 2);

		if (sqrt(s_x + s_y + s_z) < 10.2 && sqrt(s_x + s_y + s_z) > 9.8) //reach the surface of the sphere, switch to sphere mode, start timer
		{
			pUAV->m_sphere = true;
			pUAV->start_timer = true;
		}

		//before reaching the sphere
		if (!pUAV->m_sphere)
		{
			//Total maximum magnitude is 20N
			pUAV->force = sqrt(pow(pUAV->acceleration[0], 2) + pow(pUAV->acceleration[1], 2) + pow(pUAV->acceleration[2], 2));
			while (pUAV->force > 20)
			{
				pUAV->acceleration[0] *= 0.5;
				pUAV->acceleration[1] *= 0.5;
				pUAV->acceleration[2] *= 0.5;
				pUAV->force = sqrt(pow(pUAV->acceleration[0], 2) + pow(pUAV->acceleration[1], 2) + pow(pUAV->acceleration[2], 2));
			}


			//calculate velocity
			pUAV->Velocity = sqrt(pow(pUAV->velocity[0], 2) + pow(pUAV->velocity[1], 2) + pow(pUAV->velocity[2], 2));
			if (pUAV->Velocity < 2)
			{
				pUAV->velocity[0] += pUAV->acceleration[0] * 0.01;
				pUAV->velocity[1] += pUAV->acceleration[1] * 0.01;
				pUAV->velocity[2] += pUAV->acceleration[2] * 0.01;

				//calculate position
				pUAV->position[0] += 0.5 * pUAV->acceleration[0] * 0.01 * 0.01 + pUAV->velocity[0] * 0.01;
				pUAV->position[1] += 0.5 * pUAV->acceleration[1] * 0.01 * 0.01 + pUAV->velocity[1] * 0.01;
				pUAV->position[2] += 0.5 * pUAV->acceleration[2] * 0.01 * 0.01 + pUAV->velocity[2] * 0.01;
			}
			else //make sure the maximum speed is 2 m/s
			{
				pUAV->velocity[0] = 2 * ((0 - pUAV->iniPosition[0]) / sqrt(d_x + d_y + d_z));
				pUAV->velocity[1] = 2 * ((0 - pUAV->iniPosition[1]) / sqrt(d_x + d_y + d_z));
				pUAV->velocity[2] = 2 * ((50 - pUAV->iniPosition[2]) / sqrt(d_x + d_y + d_z));

				//calculate position(with constant speed now)
				pUAV->position[0] += pUAV->velocity[0] * 0.01;
				pUAV->position[1] += pUAV->velocity[1] * 0.01;
				pUAV->position[2] += pUAV->velocity[2] * 0.01;
			}
			pUAV->Velocity = sqrt(pow(pUAV->velocity[0], 2) + pow(pUAV->velocity[1], 2) + pow(pUAV->velocity[2], 2));

			
		}

		//sphere mode
		else
		{
			//generate spring force vector, its direction is towards the center of the sphere
			double D = sqrt(pow(0 - pUAV->position[0], 2) + pow(0 - pUAV->position[1], 2) + pow(50 - pUAV->position[2], 2)); //Distance between the uav and the centre of the sphere
			pUAV->spring[0] = (pUAV->position[0]) * (10 - D) * 5;
			pUAV->spring[1] = (pUAV->position[1]) * (10 - D) * 5;
			pUAV->spring[2] = (pUAV->position[2] - 50) * (10 - D) * 5;
			
			//generate a force vector perpendicular to the spring vector, tangent to the sphere
			pUAV->tangent[0] = 0.07 * pUAV->spring[1] * pUAV->spring[2];
			pUAV->tangent[1] = 0.07 * pUAV->spring[0] * pUAV->spring[2];
			pUAV->tangent[2] = -0.14 * pUAV->spring[0] * pUAV->spring[1];

			if (pUAV->Velocity <= 2) //increase tangential force to make it faster when the speed is too slow
			{
				pUAV->tangent[0] *= 13;
				pUAV->tangent[1] *= 13;
				pUAV->tangent[2] *= 13;
			}
			if (pUAV->Velocity > 10) //reduce tangential force to make it slower when the speed is too fast
			{
				pUAV->tangent[0] *= 0.2;
				pUAV->tangent[1] *= 0.2;
				pUAV->tangent[2] *= 0.2;
			}

			//sum up the two force 
			pUAV->acceleration[0] = pUAV->spring[0] + pUAV->tangent[0];
			pUAV->acceleration[1] = pUAV->spring[1] + pUAV->tangent[1];
			pUAV->acceleration[2] = pUAV->spring[2] + pUAV->tangent[2];

		

			//Total maximum magnitude is 20N
			pUAV->force = sqrt(pow(pUAV->acceleration[0], 2) + pow(pUAV->acceleration[1], 2) + pow(pUAV->acceleration[2], 2));
			if (pUAV->force > 20)
			{
				pUAV->acceleration[0] *= 0.5;
				pUAV->acceleration[1] *= 0.5;
				pUAV->acceleration[2] *= 0.5;
			}

			//calculate the velocity
			pUAV->sphereVelocity[0] += pUAV->acceleration[0] * 0.01;
			pUAV->sphereVelocity[1] += pUAV->acceleration[1] * 0.01;
			pUAV->sphereVelocity[2] += pUAV->acceleration[2] * 0.01;

			//update position
			pUAV->position[0] += 0.5 * pUAV->acceleration[0] * 0.01 * 0.01 + pUAV->sphereVelocity[0] * 0.01;
			pUAV->position[1] += 0.5 * pUAV->acceleration[1] * 0.01 * 0.01 + pUAV->sphereVelocity[1] * 0.01;
			pUAV->position[2] += 0.5 * pUAV->acceleration[2] * 0.01 * 0.01 + pUAV->sphereVelocity[2] * 0.01;

			pUAV->Velocity = sqrt(pow(pUAV->sphereVelocity[0], 2) + pow(pUAV->sphereVelocity[1], 2) + pow(pUAV->sphereVelocity[2], 2));
			
		}

		//updates the kinematic information every 10 msec
		this_thread::sleep_for(chrono::milliseconds(10));

	} while (!pUAV->m_stop);
}

void ECE_UAV::start()
{
	kinematicsThread = thread(threadFunction, this);
}