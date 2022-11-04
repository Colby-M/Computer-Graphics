#ifndef __FOUNTAINPARTICLES_H__
#define __FOUNTAINPARTICLES_H__
#include <random>
#include <functional>

class FountainParticles
{
private:
    float* positions;
    float* velocities;
    float* accelerations;
    float* orientations;
    int nbrOfParticles;
    int maxParticles;
    std::default_random_engine generator;
    std::uniform_real_distribution<double> placementDistribution;

public:
    void init(int maxParticles = 1000);
    void generate(int maxNewParticles = 5);
    void update(float timeStep = 1.0f / 30.0f);
    void compact();
    float* getPositions() { return positions; }
    float* getVelocities() { return velocities; }
    float* getAccelerations() { return accelerations; }
    float* getOrientations() { return orientations; }
    int getNumberOfParticles(){ return nbrOfParticles;}
    int getMaxParticles() { return maxParticles; }
    FountainParticles();
};

#endif