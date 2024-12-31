#include <iostream>
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

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

sf::Color calculateSpringColor(float distance, float restLength) {
    float ratio = distance / restLength;
    ratio = std::min(1.f, ratio); // Clamp ratio to 1
    return sf::Color(255 * ratio, 255 * (1 - ratio), 0); // Gradient od zielonego do czerwonego
}

int main() {
    // Parametry symulacji
    const int windowWidth = 800;
    const int windowHeight = 600;
    const float gravityStrength = 500.f;
    const float deltaTime = 0.016f;

    // Inicjalizacja okna SFML
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Zaawansowany model fizyczny");
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

    // Stan tworzenia sprężyny
    bool creatingSpring = false;
    Particle* firstParticle = nullptr;

    // Tryb edycji
    bool isEditing = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    isEditing = !isEditing; // Przełącz tryb edycji
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (!isEditing) {
                        for (auto& particle : particles) {
                            if (std::hypot(particle.position.x - event.mouseButton.x, particle.position.y - event.mouseButton.y) < 10.f) {
                                dragging = true;
                                draggedParticle = &particle;
                                break;
                            }
                        }
                    } else {
                        // Tryb edycji - przesuwanie nieprzypiętych cząsteczek
                        for (auto& particle : particles) {
                            if (!particle.isPinned &&
                                std::hypot(particle.position.x - event.mouseButton.x, particle.position.y - event.mouseButton.y) < 10.f) {
                                dragging = true;
                                draggedParticle = &particle;
                                break;
                            }
                        }
                    }
                } else if (event.mouseButton.button == sf::Mouse::Right) {
                    if (isEditing) {
                        bool foundParticle = false;
                        for (auto& particle : particles) {
                            if (std::hypot(particle.position.x - event.mouseButton.x, particle.position.y - event.mouseButton.y) < 10.f) {
                                if (!creatingSpring) {
                                    creatingSpring = true;
                                    firstParticle = &particle;
                                } else {
                                    springs.emplace_back(firstParticle, &particle);
                                    creatingSpring = false;
                                    firstParticle = nullptr;
                                }
                                foundParticle = true;
                                break;
                            }
                        }

                        if (!foundParticle) {
                            sf::Vector2f newPosition(event.mouseButton.x, event.mouseButton.y);
                            particles.emplace_back(newPosition);

                            // Znajdź najbliższą istniejącą cząsteczkę i połącz nową sprężyną
                            Particle* closestParticle = nullptr;
                            float closestDistance = std::numeric_limits<float>::max();
                            for (auto& particle : particles) {
                                if (&particle != &particles.back()) {
                                    float distance = std::hypot(particle.position.x - newPosition.x, particle.position.y - newPosition.y);
                                    if (distance < closestDistance) {
                                        closestDistance = distance;
                                        closestParticle = &particle;
                                    }
                                }
                            }

                            if (closestParticle) {
                                springs.emplace_back(closestParticle, &particles.back());
                            }
                        }
                    }
                } else if (event.mouseButton.button == sf::Mouse::Middle) {
                    if (isEditing) {
                        for (auto it = particles.begin(); it != particles.end(); ++it) {
                            if (std::hypot(it->position.x - event.mouseButton.x, it->position.y - event.mouseButton.y) < 10.f) {
                                // Usuwanie sprężyn powiązanych z tą cząsteczką
                                springs.erase(std::remove_if(springs.begin(), springs.end(), [&](const Spring& spring) {
                                    return spring.p1 == &(*it) || spring.p2 == &(*it);
                                }), springs.end());
                                particles.erase(it);
                                break;
                            }
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                dragging = false;
                draggedParticle = nullptr;
            }
        }

        if (!isEditing) {
            // Aktualizacja stanu cząsteczek
            for (auto& particle : particles) {
                particle.applyForce(sf::Vector2f(0.f, gravityStrength));
                particle.update(deltaTime);
            }

            // Aktualizacja sprężyn
            for (auto& spring : springs) {
                spring.applyConstraint();
            }
        }

        // Aktualizacja pozycji przeciąganej cząsteczki w trybie edycji
        if (dragging && draggedParticle) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            draggedParticle->position = sf::Vector2f(mousePos.x, mousePos.y);
        }

        // Rysowanie
        window.clear();

        for (const auto& spring : springs) {
            float distance = std::sqrt(std::pow(spring.p2->position.x - spring.p1->position.x, 2) +
                                       std::pow(spring.p2->position.y - spring.p1->position.y, 2));
            sf::Color color = calculateSpringColor(distance, spring.restLength);
            sf::Vertex line[] = {
                sf::Vertex(spring.p1->position, color),
                sf::Vertex(spring.p2->position, color)
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

        // Informacja o trybie edycji
        if (isEditing) {
            sf::Font font;
            if (font.loadFromFile("arial.ttf")) {
                sf::Text text("Editing Mode", font, 20);
                text.setFillColor(sf::Color::White);
                text.setPosition(10.f, 10.f);
                window.draw(text);
            }
        }

        window.display();
    }

    return 0;
}
