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

// Klasa cząsteczki ognia
class Particle {
public:
    Vector3D position;
    Vector3D velocity;
    sf::Color color;
    float lifeTime;
    float size;

    Particle(const Vector3D& pos, const Vector3D& vel, sf::Color col, float life, float sz)
        : position(pos), velocity(vel), color(col), lifeTime(life), size(sz) {}

    void update(float dt) {
        position += velocity * dt;

        // Stopniowe przejście koloru od czerwonego do żółtego
        float progress = 1.0f - (lifeTime / 5.0f); // Assuming max lifetime is 5 seconds
        color = sf::Color(
            255, // Red stays constant
            static_cast<sf::Uint8>(progress * 255), // Green increases
            0,   // Blue stays zero
            150);

        lifeTime -= dt;
    }

    bool isAlive() const {
        return lifeTime > 0;
    }
};

// Klasa płatków śniegu
class Snowflake {
public:
    Vector3D position;
    Vector3D velocity;
    float size;

    Snowflake(const Vector3D& pos, const Vector3D& vel, float sz)
        : position(pos), velocity(vel), size(sz) {}

    void update(float dt) {
        position += velocity * dt;
        // Delikatny ruch w lewo i prawo
        velocity.x += ((rand() % 3) - 1) * 0.1f;
    }

    void draw(sf::RenderWindow& window) const {
        sf::CircleShape shape(size);
        shape.setPosition(position.x, position.y);
        shape.setFillColor(sf::Color(255, 255, 255, 200));
        window.draw(shape);
    }
};

class Emitter {
    std::vector<Particle> particles;
    Vector3D position;

public:
    Emitter(const Vector3D& pos) : position(pos) {}

    void emit(int count) {
        for (int i = 0; i < count; ++i) {
            Vector3D velocity = randomVelocity() * 50.0f;
            sf::Color color(255, 0, 0, 150); // Start with red
            float lifeTime = static_cast<float>(rand() % 3 + 2); // Lifetimes between 2 and 5 seconds
            float size = rand() % 2 + 2; // Slightly larger particles
            particles.emplace_back(position, velocity, color, lifeTime, size);
        }
    }

    void update(float dt) {
        for (auto& particle : particles) {
            particle.update(dt);
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
        return Vector3D(distribution(generator), -std::abs(distribution(generator)), 0); // Particles move upward
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Particle System - Fire and Snow", sf::Style::Default, sf::ContextSettings(24));
    window.setFramerateLimit(60);

    Emitter fireEmitter(Vector3D(400, 580, 0)); // Position emitter near the bottom of the screen

    std::vector<Snowflake> snowflakes;
    for (int i = 0; i < 200; ++i) { // Increase the number of snowflakes
        float x = rand() % 800;
        float y = rand() % 600;
        Vector3D position(x, y, 0);
        Vector3D velocity(0, 30.0f, 0); // Falling down
        float size = rand() % 3 + 1;
        snowflakes.emplace_back(position, velocity, size);
    }

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();

        fireEmitter.emit(15); // Emit fewer particles for a consistent fire effect
        fireEmitter.update(dt);

        for (auto& snowflake : snowflakes) {
            snowflake.update(dt);
            // Reset snowflake if it goes off screen
            if (snowflake.position.y > 600) {
                snowflake.position.y = 0;
                snowflake.position.x = rand() % 800;
            }
        }

        window.clear();

        // Draw ground (optional, decorative)
        sf::RectangleShape ground(sf::Vector2f(800, 20));
        ground.setPosition(0, 580);
        ground.setFillColor(sf::Color(139, 69, 19)); // Brown color for ground
        window.draw(ground);

        fireEmitter.draw(window);
        
        for (const auto& snowflake : snowflakes) {
            snowflake.draw(window);
        }

        // Draw signature
        sf::Font font;
        if (!font.loadFromFile("arial.ttf")) {
            // Handle font loading error
            return -1;
        }
        
        sf::Text signature("Mateusz Sierakowski", font, 24);
        signature.setFillColor(sf::Color::White);
        signature.setPosition(10, 10);
        window.draw(signature);

        window.display();
    }

    return 0;
}
