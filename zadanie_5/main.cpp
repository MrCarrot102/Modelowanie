#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>

// ----------------------------------------
// Struktura reprezentująca cząsteczkę
// ----------------------------------------
struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifetime;

    Particle(sf::Vector2f pos, sf::Vector2f vel, sf::Color col, float life)
        : position(pos), velocity(vel), color(col), lifetime(life) {}

    void update(float dt) {
        position += velocity * dt;
        lifetime -= dt;
    }

    bool isAlive() const {
        return lifetime > 0.0f;
    }
};

// ----------------------------------------
// Klasa emitera cząsteczek
// ----------------------------------------
class Emitter {
public:
    Emitter(sf::Vector2f pos, unsigned int maxParticles)
        : position(pos), maxParticles(maxParticles) {}

    void emit() {
        if (particles.size() < maxParticles) {
            particles.push_back(createParticle());
        }
    }

    void update(float dt) {
        for (auto& particle : particles) {
            particle.update(dt);
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                           [](const Particle& p) { return !p.isAlive(); }),
            particles.end());
    }

    void draw(sf::RenderWindow& window) {
        for (const auto& particle : particles) {
            sf::CircleShape shape(2.0f);  // Mniejszy rozmiar cząsteczek dla lepszego efektu
            shape.setFillColor(particle.color);
            shape.setPosition(particle.position);
            window.draw(shape);
        }
    }

private:
    sf::Vector2f position;
    unsigned int maxParticles;
    std::vector<Particle> particles;

    Particle createParticle() {
        // Generowanie losowej prędkości i pozycji wokół emitera
        float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
        float speed = static_cast<float>(rand() % 100) / 10.0f + 50.0f;

        sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);
        sf::Color color(rand() % 255, rand() % 255, rand() % 255);
        float lifetime = 3.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f);

        return Particle(position + sf::Vector2f(rand() % 20 - 10, rand() % 20 - 10),
                        velocity, color, lifetime);
    }
};

// ----------------------------------------
// Główna funkcja programu
// ----------------------------------------
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Particle System");
    window.setFramerateLimit(60);

    Emitter emitter(sf::Vector2f(400, 300), 1000);  // Emiter na środku ekranu
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();

        emitter.emit();
        emitter.update(dt);

        window.clear();
        emitter.draw(window);
        window.display();
    }

    return 0;
}
