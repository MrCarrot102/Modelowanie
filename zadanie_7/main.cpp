#include <iostream>
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>

// Struktura reprezentująca cząsteczkę
struct Particle {
    sf::Vector2f position;
    sf::Vector2f previousPosition;
    sf::Vector2f acceleration;
    bool isPinned;

    Particle(sf::Vector2f pos, bool pinned = false)
        : position(pos), previousPosition(pos), acceleration(0.f, 0.f), isPinned(pinned) {}

    void applyForce(const sf::Vector2f& force) {
        if (!isPinned) {
            acceleration += force;
        }
    }

    void update(float deltaTime) {
        if (!isPinned) {
            sf::Vector2f temp = position;
            position += position - previousPosition + acceleration * (deltaTime * deltaTime);
            previousPosition = temp;
            acceleration = sf::Vector2f(0.f, 0.f);
        }
    }
};

// Struktura reprezentująca sprężynę
struct Spring {
    Particle* p1;
    Particle* p2;
    float restLength;
    float stiffness;

    Spring(Particle* particle1, Particle* particle2, float stiffness = 0.1f)
        : p1(particle1), p2(particle2), stiffness(stiffness) {
        restLength = std::sqrt(std::pow(p2->position.x - p1->position.x, 2) +
                               std::pow(p2->position.y - p1->position.y, 2));
    }

    void applyConstraint() {
        sf::Vector2f delta = p2->position - p1->position;
        float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        float difference = (distance - restLength) / distance;
        sf::Vector2f offset = delta * stiffness * difference;

        if (!p1->isPinned) p1->position += offset;
        if (!p2->isPinned) p2->position -= offset;
    }
};

int main() {
    // Parametry symulacji
    const int windowWidth = 800;
    const int windowHeight = 600;
    const float gravityStrength = 500.f;
    const float deltaTime = 0.016f;

    // Inicjalizacja okna SFML
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Model fizyczny");
    window.setFramerateLimit(60);

    // Tworzenie cząsteczek
    std::vector<Particle> particles;
    const int numParticles = 10;
    const float particleSpacing = 50.f;

    for (int i = 0; i < numParticles; ++i) {
        particles.emplace_back(sf::Vector2f(300.f + i * particleSpacing, 300.f), i == 0); // Pierwsza cząsteczka przypięta
    }

    // Tworzenie sprężyn
    std::vector<Spring> springs;
    for (int i = 0; i < numParticles - 1; ++i) {
        springs.emplace_back(&particles[i], &particles[i + 1]);
    }

    // Interakcja użytkownika
    bool dragging = false;
    Particle* draggedParticle = nullptr;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePosition(event.mouseButton.x, event.mouseButton.y);
                for (auto& particle : particles) {
                    if (std::hypot(particle.position.x - mousePosition.x, particle.position.y - mousePosition.y) <= 15.f) {
                        dragging = true;
                        draggedParticle = &particle;
                        std::cout << "Dragging particle at position: " << particle.position.x << ", " << particle.position.y << std::endl;
                        break;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                dragging = false;
                draggedParticle = nullptr;
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                std::cout << "Mouse clicked at: " << event.mouseButton.x << ", " << event.mouseButton.y << std::endl;
            }
        }

        // Aktualizacja stanu cząsteczek
        for (auto& particle : particles) {
            particle.applyForce(sf::Vector2f(0.f, gravityStrength));
            particle.update(deltaTime);
        }

        // Aktualizacja sprężyn
        for (auto& spring : springs) {
            spring.applyConstraint();
        }

        // Aktualizacja pozycji przeciąganej cząsteczki
        if (dragging && draggedParticle) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            draggedParticle->position = sf::Vector2f(mousePos.x, mousePos.y);
        }

        // Rysowanie
        window.clear();

        for (const auto& spring : springs) {
            sf::Vertex line[] = {
                sf::Vertex(spring.p1->position, sf::Color::White),
                sf::Vertex(spring.p2->position, sf::Color::White)
            };
            window.draw(line, 2, sf::Lines);
        }

        for (const auto& particle : particles) {
            sf::CircleShape shape(5.f);
            shape.setOrigin(5.f, 5.f);
            shape.setPosition(particle.position);
            shape.setFillColor(particle.isPinned ? sf::Color::Red : sf::Color::Blue);
            window.draw(shape);
        }

        window.display();
    }

    return 0;
}
