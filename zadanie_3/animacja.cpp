#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>

const int szerokosc_okna = 800;
const int wysokosc_okna = 600;
const int ilosc_dyskow = 1000;
const float czas = 0.01f;
const float G = 100.0f; // Stała przyciągania
const sf::Vector2f SRODEK(szerokosc_okna / 2.0f, wysokosc_okna / 2.0f);

// Struktura Dysku
struct Dysk {
    sf::CircleShape ksztalt;
    sf::Vector2f v;
    float masa;
    float wsp_oporu;

    Dysk(float x, float y, float vx, float vy, float m, float srednica, sf::Color kolor)
        : v(vx, vy), masa(m) {
        ksztalt.setRadius(srednica / 2.0f);
        ksztalt.setFillColor(kolor);
        ksztalt.setPosition(x, y);
        wsp_oporu = 0.01f + (y / wysokosc_okna) * 0.05f; // Wartość oporu zmienia się z pozycją
    }

    void zastosujSile(const sf::Vector2f& F) {
        v += F * (czas / masa);
    }

    void zaktualizujPozycje() {
        sf::Vector2f opor = -wsp_oporu * v;  // Opór proporcjonalny do prędkości
        zastosujSile(opor);
        ksztalt.move(v * czas);

        // Odbicia od krawędzi
        sf::Vector2f pozycja = ksztalt.getPosition();
        if (pozycja.x < 0) { pozycja.x = 0; v.x *= -1; }
        else if (pozycja.x > szerokosc_okna - 2 * ksztalt.getRadius()) { pozycja.x = szerokosc_okna - 2 * ksztalt.getRadius(); v.x *= -1; }
        if (pozycja.y < 0) { pozycja.y = 0; v.y *= -1; }
        else if (pozycja.y > wysokosc_okna - 2 * ksztalt.getRadius()) { pozycja.y = wysokosc_okna - 2 * ksztalt.getRadius(); v.y *= -1; }
        ksztalt.setPosition(pozycja);
    }
};

// Siła przyciągająca do centrum lub innych punktów
sf::Vector2f silaPrzyciagania(const sf::Vector2f& pozycja, const sf::Vector2f& punkt, float intensywnosc) {
    sf::Vector2f kierunek = punkt - pozycja;
    float odleglosc = std::sqrt(kierunek.x * kierunek.x + kierunek.y * kierunek.y);
    if (odleglosc < 1.0f) odleglosc = 1.0f;
    kierunek /= odleglosc;
    return kierunek * (intensywnosc / (odleglosc * odleglosc));
}

// Generowanie liczb losowych
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(-50, 50);
std::uniform_real_distribution<> posX(0, szerokosc_okna - 2 * 50);
std::uniform_real_distribution<> posY(0, wysokosc_okna - 2 * 50);
std::uniform_real_distribution<> rozkladMas(1.0, 5.0);
std::uniform_real_distribution<> rozkladSrednicy(10.0, 40.0);
std::uniform_int_distribution<> kolor(0, 255);

sf::Color losujKolor() {
    return sf::Color(kolor(gen), kolor(gen), kolor(gen));
}

int main() {
    sf::RenderWindow okno(sf::VideoMode(szerokosc_okna, wysokosc_okna), "Rozszerzona Symulacja Dysków");
    std::vector<Dysk> dyski;
    std::vector<sf::Vector2f> punktyPrzyciagania{SRODEK};

    for (int i = 0; i < ilosc_dyskow; ++i) {
        float x = posX(gen);
        float y = posY(gen);
        float vx = dis(gen);
        float vy = dis(gen);
        float masa = rozkladMas(gen);
        float srednica = rozkladSrednicy(gen);
        sf::Color kolor = losujKolor();
        dyski.emplace_back(x, y, vx, vy, masa, srednica, kolor);
    }

    while (okno.isOpen()) {
        sf::Event event;
        while (okno.pollEvent(event)) {
            if (event.type == sf::Event::Closed) okno.close();
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f nowyPunkt(event.mouseButton.x, event.mouseButton.y);
                    punktyPrzyciagania.push_back(nowyPunkt); // Dodaj nowy punkt przyciągania
                }
            }
        }

        // Aktualizacja sił i pozycji
        for (auto& dysk : dyski) {
            sf::Vector2f silaCalkowita(0, 0);

            // Przyciąganie do każdego punktu
            for (const auto& punkt : punktyPrzyciagania) {
                silaCalkowita += silaPrzyciagania(dysk.ksztalt.getPosition(), punkt, G);
            }

            // Przyciąganie między dyskami
            for (const auto& inny : dyski) {
                if (&dysk != &inny) {
                    silaCalkowita += silaPrzyciagania(dysk.ksztalt.getPosition(), inny.ksztalt.getPosition(), G * dysk.masa * inny.masa);
                }
            }

            dysk.zastosujSile(silaCalkowita);
            dysk.zaktualizujPozycje();
        }

        // Renderowanie
        okno.clear();
        for (const auto& dysk : dyski) {
            okno.draw(dysk.ksztalt);
        }
        okno.display();
    }

    return 0;
}
