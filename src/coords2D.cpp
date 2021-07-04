#include "coords2D.h"

Coords2D::Coords2D() : real{0,0}, integer{0,0} {}

Coords2D::Coords2D(const std::pair<float,float>& point) : 
    real{point.first,point.second},
    integer{static_cast<int>(point.first),static_cast<int>(point.second)} {}

Coords2D::Coords2D(const std::pair<int,int>& point) :
    real{static_cast<float>(point.first),static_cast<float>(point.second)},
    integer{point.first,point.second} {}

Coords2D::Coords2D(const SDL_Point& point) :
    real{static_cast<float>(point.x),static_cast<float>(point.y)},
    integer{point.x,point.y} {}

Coords2D::Coords2D(const Coords2D& point) :
    real{point.real},
    integer{point.integer} {}

Coords2D::Coords2D(Coords2D&& point) :
    real{point.real},
    integer{point.integer} {}

Coords2D& Coords2D::operator=(const std::pair<float,float>& point) {
    this->real.x = point.first;
    this->real.y = point.second;
    this->integer.x = static_cast<int>(point.first);
    this->integer.y = static_cast<int>(point.second);
    return *this;
}

Coords2D& Coords2D::operator=(const std::pair<int,int>& point) {
    this->real.x = static_cast<float>(point.first);
    this->real.y = static_cast<float>(point.second);
    this->integer.x = point.first;
    this->integer.y = point.second;
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
    this->real = point.real;
    this->integer = point.integer;
    return *this;
}

Coords2D& Coords2D::operator=(Coords2D&& point) {
    this->real = point.real;
    this->integer = point.integer;
    return *this;
}

bool Coords2D::operator==(const Coords2D& point) {
    return (this->integer.x == point.integer.x) && (this->integer.y == point.integer.y);
}

bool Coords2D::operator==(Coords2D&& point) {
    return (this->integer.x == point.integer.x) && (this->integer.y == point.integer.y);
}