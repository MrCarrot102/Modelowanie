#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>

// stale potrzebne do wygenerowania animacji 
const int szerokosc_okna = 800;
const int wysokosc_okna = 600;
const int ilosc_dyskow = 1000;
const float czas = 0.01f;
const float g = 100.0f; // dostosowywanie sily przyciagania 
const sf::Vector2f SRODEK(szerokosc_okna / 2.0f, wysokosc_okna / 2.0f);

// struktura dysk 
struct Dysk {
    sf::CircleShape ksztalt;
    sf::Vector2f v;
    float masa;

    Dysk(float x, float y, float vx, float vy, float m, float srednica, sf::Color kolor)
        : v(vx, vy), masa(m) {
        ksztalt.setRadius(srednica / 2.0f);
        ksztalt.setFillColor(kolor);
        ksztalt.setPosition(x, y);
    }

    void zastosujF(const sf::Vector2f& F) {
        v += F * (czas / masa);
    }

    void zaktualizujPozycję() {
        ksztalt.move(v * czas);

        // nadawanie odbic od brzegow 
        sf::Vector2f pozycja = ksztalt.getPosition();
        if (pozycja.x < 0) {
            pozycja.x = 0;
            v.x *= -1;
        } else if (pozycja.x > szerokosc_okna - 2 * ksztalt.getRadius()) {
            pozycja.x = szerokosc_okna - 2 * ksztalt.getRadius();
            v.x *= -1;
        }
        if (pozycja.y < 0) {
            pozycja.y = 0;
            v.y *= -1;
        } else if (pozycja.y > wysokosc_okna - 2 * ksztalt.getRadius()) {
            pozycja.y = wysokosc_okna - 2 * ksztalt.getRadius();
            v.y *= -1;
        }
        ksztalt.setPosition(pozycja);
    }
};

// obliczanie sil przyciagajacyh do srodka 
sf::Vector2f Fprzyciagania(const sf::Vector2f& pozycja) {
    sf::Vector2f kierunek = SRODEK - pozycja;
    float odleglosc = std::sqrt(kierunek.x * kierunek.x + kierunek.y * kierunek.y);
    if (odleglosc < 1.0f) odleglosc = 1.0f; // unikanie dzielenia przez 0
    kierunek /= odleglosc; // normalizacja 
    float wielkoscSily = g / (odleglosc * odleglosc);
    return kierunek * wielkoscSily;
}

// generowanie liczb losowych do animacji 
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(-50, 50);
std::uniform_real_distribution<> posX(0, szerokosc_okna - 2 * 50); // nadawanie maksymalnego rozmiaru dysku 
std::uniform_real_distribution<> posY(0, wysokosc_okna - 2 * 50);
std::uniform_real_distribution<> rozkładMas(1.0, 5.0);
std::uniform_real_distribution<> rozkładŚrednicy(10.0, 40.0); // zakres srednic dyskow 
std::uniform_int_distribution<> kolor(0, 255); // zakres kolorow

// nadawanie dyskom losowych kolorow
sf::Color losujKolor() {
    return sf::Color(kolor(gen), kolor(gen), kolor(gen));
}

int main() {
    sf::RenderWindow okno(sf::VideoMode(szerokosc_okna, wysokosc_okna), "Symulacja Dysków");

    // nadawanie dyskom losowych wartosci 
    std::vector<Dysk> dyski;
    for (int i = 0; i < ilosc_dyskow; ++i) {
        float x = posX(gen);
        float y = posY(gen);
        float vx = dis(gen);
        float vy = dis(gen);
        float masa = rozkładMas(gen);
        float srednica = rozkładŚrednicy(gen);
        sf::Color kolor = losujKolor();
        dyski.emplace_back(x, y, vx, vy, masa, srednica, kolor);
    }

    // petla do symulacji 
    while (okno.isOpen()) {
        sf::Event event;
        while (okno.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                okno.close();
        }

        // Aktualizacja każdego dysku
        for (auto& dysk : dyski) {
            sf::Vector2f F = Fprzyciagania(dysk.ksztalt.getPosition());
            dysk.zastosujF(F);
            dysk.zaktualizujPozycję();
        }

        // Rysowanie
        okno.clear();
        for (const auto& dysk : dyski) {
            okno.draw(dysk.ksztalt);
        }
        okno.display();
    }

    return 0;
}
