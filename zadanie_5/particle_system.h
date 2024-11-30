#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <vector>

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    sf::Color color;
    float lifespan;

    Particle(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc, sf::Color col, float life);
    void update(float deltaTime);
    void draw();
};

class ParticleSystem {
public:
    std::vector<Particle> particles;
    void addParticle(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, sf::Color color, float lifespan);
    void update(float deltaTime);
    void draw();
};

#endif
