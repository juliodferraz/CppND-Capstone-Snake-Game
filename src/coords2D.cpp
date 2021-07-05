#include "coords2D.h"
#include <cmath>
#include <cassert>

Coords2D::Coords2D() : real{0,0}, integer{0,0} {}

Coords2D::Coords2D(const std::initializer_list<float>& point) {
    assert(point.size() == 2);

    auto it = point.begin();
    this->real.x = *it;
    this->real.y = *(it+1);

    this->integer.x = static_cast<int>(this->real.x);
    this->integer.y = static_cast<int>(this->real.y);
}

Coords2D::Coords2D(const SDL_Point& point) :
    real{static_cast<float>(point.x),static_cast<float>(point.y)},
    integer{point.x,point.y} {}

Coords2D::Coords2D(const Coords2D& point) :
    real{point.real},
    integer{point.integer} {}

Coords2D::Coords2D(Coords2D&& point) :
    real{point.real},
    integer{point.integer} {}

Coords2D& Coords2D::operator=(const std::initializer_list<float>& point) {
    assert(point.size() == 2);

    auto it = point.begin();
    this->real.x = *it;
    this->real.y = *(it+1);

    this->integer.x = static_cast<int>(this->real.x);
    this->integer.y = static_cast<int>(this->real.y);

    return *this;
}

Coords2D& Coords2D::operator=(const SDL_Point& point) {
    this->real.x = static_cast<float>(point.x);
    this->real.y = static_cast<float>(point.y);
    this->integer.x = point.x;
    this->integer.y = point.y;
    return *this;
}

Coords2D& Coords2D::operator=(const Coords2D& point) {
    if (this != &point) {
        this->real = point.real;
        this->integer = point.integer;
    }
    return *this;
}

Coords2D& Coords2D::operator=(Coords2D&& point) {
    this->real = point.real;
    this->integer = point.integer;
    return *this;
}

bool Coords2D::operator==(const std::initializer_list<int>& point) {
    assert(point.size() == 2);
    auto it = point.begin();
    return (this->integer.x == *it) && (this->integer.y == *(it+1));
}

bool Coords2D::operator==(const Coords2D& point) {
    return (this->integer.x == point.integer.x) && (this->integer.y == point.integer.y);
}

bool Coords2D::operator==(Coords2D&& point) {
    return (this->integer.x == point.integer.x) && (this->integer.y == point.integer.y);
}

bool Coords2D::operator==(const SDL_Point& point) {
    return (this->integer.x == point.x) && (this->integer.y == point.y);
}

Coords2D& Coords2D::operator+(const std::initializer_list<float>& delta) {
    assert(delta.size() == 2);

    auto it = delta.begin();
    this->real.x += *it;
    this->real.y += *(it+1);

    this->integer.x = static_cast<int>(this->real.x);
    this->integer.y = static_cast<int>(this->real.y);

    return *this;
}

Coords2D& Coords2D::operator+=(const std::initializer_list<float>& delta) {
    assert(delta.size() == 2);

    auto it = delta.begin();
    this->real.x += *it;
    this->real.y += *(it+1);

    this->integer.x = static_cast<int>(this->real.x);
    this->integer.y = static_cast<int>(this->real.y);
    
    return *this;
}

float Coords2D::GetEuclideanDistanceTo(const Coords2D& reference) const {
    return sqrt(pow(this->real.x - reference.real.x, 2) + pow(this->real.y - reference.real.y, 2));
}

int Coords2D::GetManhattanDistanceTo(const Coords2D& reference) const {
    return abs(this->integer.x - reference.integer.x) + abs(this->integer.y - reference.integer.y);
}

int Coords2D::GetManhattanDistanceTo(const SDL_Point& reference) const {
    return abs(this->integer.x - reference.x) + abs(this->integer.y - reference.y);
}