#include "particle_system.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>

Particle::Particle(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc, sf::Color col, float life)
    : position(pos), velocity(vel), acceleration(acc), color(col), lifespan(life) {}

void Particle::update(float deltaTime) {
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;
    lifespan -= deltaTime;
}

void Particle::draw() {
    glBegin(GL_POINTS);
    glColor4f(color.r / 255.f, color.g / 255.f, color.b / 255.f, lifespan / 5.f);
    glVertex3f(position.x, position.y, position.z);
    glEnd();
}

void ParticleSystem::addParticle(glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, sf::Color color, float lifespan) {
    particles.push_back(Particle(position, velocity, acceleration, color, lifespan));
}

void ParticleSystem::update(float deltaTime) {
    for (auto& particle : particles) {
        particle.update(deltaTime);
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](Particle& p) { return p.lifespan <= 0; }), particles.end());
}

void ParticleSystem::draw() {
    for (auto& particle : particles) {
        particle.draw();
    }
}
