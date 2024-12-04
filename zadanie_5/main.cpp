#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <cmath>
#include <random>

// Struktura wektora 3D
struct Vector3D {
    float x, y, z;

    Vector3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }

    Vector3D operator*(float scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    Vector3D& operator+=(const Vector3D& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3D normalized() const {
        float len = length();
        return len > 0 ? Vector3D(x / len, y / len, z / len) : Vector3D(0, 0, 0);
    }
};

// Struktura koła
struct Circle {
    Vector3D position;
    float radius;

    Circle(const Vector3D& pos, float r) : position(pos), radius(r) {}

    bool contains(const Vector3D& point) const {
        return (point - position).length() <= radius;
    }
};

// Klasa cząsteczki
class Particle {
public:
    Vector3D position;
    Vector3D velocity;
    sf::Color color;
    float lifeTime;
    float size;

    Particle(const Vector3D& pos, const Vector3D& vel, sf::Color col, float life, float sz)
        : position(pos), velocity(vel), color(col), lifeTime(life), size(sz) {}

    void update(float dt, const Vector3D& wind, const Vector3D& attractionPoint, const std::vector<Circle>& circles) {
        Vector3D attractionForce = (attractionPoint - position).normalized() * 20.0f;
        velocity += (wind + attractionForce) * dt;
        position += velocity * dt;

        for (const auto& circle : circles) {
            if (circle.contains(position)) {
                // Odbicie: proste odbicie w przeciwnym kierunku
                velocity = velocity * -0.8f; // Tłumienie odbicia
                position += velocity * dt;
            }
        }

        lifeTime -= dt;
    }

    bool isAlive() const {
        return lifeTime > 0;
    }
};

// Klasa emitera
class Emitter {
    std::vector<Particle> particles;
    Vector3D position;

public:
    Emitter(const Vector3D& pos) : position(pos) {}

    void emit(int count) {
        for (int i = 0; i < count; ++i) {
            Vector3D velocity = randomVelocity() * 50.0f;
            sf::Color color(rand() % 255, rand() % 255, rand() % 255, 150);
            float lifeTime = static_cast<float>(rand() % 3 + 3);
            float size = rand() % 2 + 1;
            particles.emplace_back(position, velocity, color, lifeTime, size);
        }
    }

    void update(float dt, const Vector3D& wind, const Vector3D& attractionPoint, const std::vector<Circle>& circles) {
        for (auto& particle : particles) {
            particle.update(dt, wind, attractionPoint, circles);
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(),
                                       [](const Particle& p) { return !p.isAlive(); }),
                        particles.end());
    }

    void draw(sf::RenderWindow& window) {
        for (const auto& particle : particles) {
            sf::CircleShape shape(particle.size);
            shape.setPosition(particle.position.x, particle.position.y);
            shape.setFillColor(particle.color);
            window.draw(shape);
        }
    }

private:
    Vector3D randomVelocity() {
        static std::default_random_engine generator;
        static std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
        return Vector3D(distribution(generator), distribution(generator), 0);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Particle System with Circles", sf::Style::Default, sf::ContextSettings(24));
    window.setFramerateLimit(60);

    Emitter emitter(Vector3D(400, 300, 0));
    Vector3D wind(0, 0, 0);
    Vector3D attractionPoint(400, 300, 0);

    std::vector<Circle> circles;

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    attractionPoint = Vector3D(event.mouseButton.x, event.mouseButton.y, 0);
                }
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    circles.emplace_back(Vector3D(event.mouseButton.x, event.mouseButton.y, 0), 50.0f); // Koło o promieniu 50
                }
            }
        }

        // Obsługa klawiatury dla wiatru
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) wind.x = -20.0f;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) wind.x = 20.0f;
        else wind.x = 0;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) wind.y = -20.0f;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) wind.y = 20.0f;
        else wind.y = 0;

        float dt = clock.restart().asSeconds();

        emitter.emit(30);
        emitter.update(dt, wind, attractionPoint, circles);

        window.clear();
        emitter.draw(window);

        for (const auto& circle : circles) {
            sf::CircleShape shape(circle.radius);
            shape.setPosition(circle.position.x - circle.radius, circle.position.y - circle.radius);
            shape.setFillColor(sf::Color(255, 255, 255, 50));
            window.draw(shape);
        }

        window.display();
    }

    return 0;
}
