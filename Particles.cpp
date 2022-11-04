#include "Particles.h"

void FountainParticles::init(int maxParticles)
{
	auto releaseArray = [](auto* arrayName) { if (arrayName != nullptr) delete[] arrayName; };

	releaseArray(positions);
	releaseArray(velocities);
	releaseArray(accelerations);
	releaseArray(orientations);
	nbrOfParticles = 0;
	this->maxParticles = maxParticles;
	positions = new float[maxParticles * 3];
	velocities = new float[maxParticles * 3];
	accelerations = new float[maxParticles * 3];
	orientations = new float[maxParticles * 4];
	return;
}

void FountainParticles::generate(int maxNewParticles)
{
	if (maxNewParticles + nbrOfParticles < maxParticles) { // generate the particles
		for (int currentParticle = 0; currentParticle < maxNewParticles; ++currentParticle) {
			positions[(nbrOfParticles + currentParticle) * 3] = placementDistribution(generator) * 2.0 - 1.0;;
			positions[(nbrOfParticles + currentParticle) * 3 + 1] = 1.0f;
			positions[(nbrOfParticles + currentParticle) * 3 + 2] = placementDistribution(generator) * 2.0 - 1.0;;
			
			velocities[(nbrOfParticles + currentParticle) * 3] = 0;
			velocities[(nbrOfParticles + currentParticle) * 3 + 1] = 0;
			velocities[(nbrOfParticles + currentParticle) * 3 + 2] = 0;

			accelerations[(nbrOfParticles + currentParticle) * 3] = 0.0;
			accelerations[(nbrOfParticles + currentParticle) * 3 + 1] = -1.0;
			accelerations[(nbrOfParticles + currentParticle) * 3 + 2] = 0.0;

			orientations[(nbrOfParticles + currentParticle) * 4] = 1.0;
			orientations[(nbrOfParticles + currentParticle) * 4 + 1] = 0.0;
			orientations[(nbrOfParticles + currentParticle) * 4 + 2] = 0.0;
			orientations[(nbrOfParticles + currentParticle) * 4 + 3] = 0.0;
		}
		nbrOfParticles += maxNewParticles;
	}
}

void FountainParticles::update(float timeStep)
{
	float newPosition[3];
	float newVelocity[3];
	float newAcceleration[3];
	float newOrientation[4];

	for (int current = 0; current < nbrOfParticles * 3; current += 3) {
		for (int i = 0; i < 3; i++) {
			newPosition[i] = positions[current + i] + velocities[current + i] * timeStep + accelerations[current + i] * timeStep * timeStep / 2.0f;
		}
		for (int i = 0; i < 3; i++) {
			newVelocity[i] = velocities[current + i] + accelerations[current + i] * timeStep;
		}
		for (int i = 0; i < 3; i++) {
			positions[current + i] = newPosition[i];
			velocities[current + i] = newVelocity[i];
		}
	}
}

void FountainParticles::compact()
{
	int current, i;

	for (current = 0; current < nbrOfParticles; ++current) {
		while (positions[current * 3 + 1]  < -1.0f && current < nbrOfParticles) {
			for (i = 0; i < 3; ++i) {
				positions[current * 3 + i] = positions[(nbrOfParticles - 1) * 3 + i];
				velocities[current * 3 + i] = velocities[(nbrOfParticles - 1) * 3 + i];
				accelerations[current * 3 + i] = accelerations[(nbrOfParticles - 1) * 3 + i];
			}
			for (i = 0; i < 4; ++i) {
				orientations[current * 4 + i] = orientations[(nbrOfParticles - 1) * 4 + i];
			}
			--nbrOfParticles;
		}
	}
}

FountainParticles::FountainParticles()
{
	positions = nullptr;
	velocities = nullptr;
	accelerations = nullptr;
	orientations = nullptr;
	nbrOfParticles = 0;
	maxParticles = 0;
}
