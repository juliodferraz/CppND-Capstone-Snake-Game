#include "coords2D.h"
#include <cmath>
#include "clip.h"

bool operator==(const SDL_Point& a, const SDL_Point& b) {
    return (a.x == b.x) && (a.y == b.y);
}

int GetManhattanDistance(const SDL_Point& a, const SDL_Point& b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

Coords2D::Coords2D() : SDL_Point{0,0}, SDL_FPoint{0.0,0.0} {}

template<typename T>
Coords2D::Coords2D(const T& x, const T& y) : 
    SDL_Point{(int)x,(int)y}, 
    SDL_FPoint{(float)x,(float)y} {}

template<typename T>
Coords2D::Coords2D(const T& sdlPoint) :
    SDL_Point{(int)sdlPoint.x,(int)sdlPoint.y}, 
    SDL_FPoint{(float)sdlPoint.x,(float)sdlPoint.y} {}

Coords2D::Coords2D(const Coords2D& point) :
    SDL_Point{point}, SDL_FPoint{point} {}

Coords2D::Coords2D(Coords2D&& point) :
    SDL_Point{point}, SDL_FPoint{point} {}

Coords2D& Coords2D::operator=(const Coords2D& point) {
    if (this != &point) {
        this->SDL_Point::x = point.SDL_Point::x;
        this->SDL_Point::y = point.SDL_Point::y;
        this->SDL_FPoint::x = point.SDL_FPoint::x;
        this->SDL_FPoint::y = point.SDL_FPoint::y;
    }
    return *this;
}

Coords2D& Coords2D::operator=(Coords2D&& point) {
    this->SDL_Point::x = point.SDL_Point::x;
    this->SDL_Point::y = point.SDL_Point::y;
    this->SDL_FPoint::x = point.SDL_FPoint::x;
    this->SDL_FPoint::y = point.SDL_FPoint::y;
    return *this;
}

template<typename T>
Coords2D& Coords2D::operator=(const T& sdlPoint) {
    this->SDL_Point::x = (int)sdlPoint.x;
    this->SDL_Point::y = (int)sdlPoint.y;
    this->SDL_FPoint::x = (float)sdlPoint.x;
    this->SDL_FPoint::y = (float)sdlPoint.y;
    return *this;
}

bool Coords2D::operator==(const SDL_Point& point) {
    return (this->SDL_Point::x == point.x) && (this->SDL_Point::y == point.y);
}

template<typename T>
Coords2D& Coords2D::operator+(const T& sdlDelta) {
    this->SDL_FPoint::x = CLPD_FLT_SUM(this->SDL_FPoint::x, (float) sdlDelta.x);
    this->SDL_FPoint::y = CLPD_FLT_SUM(this->SDL_FPoint::y, (float) sdlDelta.y);
    this->SDL_Point::x = CLPD_FLT2INT(this->SDL_FPoint::x);
    this->SDL_Point::y = CLPD_FLT2INT(this->SDL_FPoint::y);
    return *this;
}

template<typename T>
Coords2D& Coords2D::operator+=(const T& sdlDelta) {
    this->SDL_FPoint::x = CLPD_FLT_SUM(this->SDL_FPoint::x, (float) sdlDelta.x);
    this->SDL_FPoint::y = CLPD_FLT_SUM(this->SDL_FPoint::y, (float) sdlDelta.y);
    this->SDL_Point::x = CLPD_FLT2INT(this->SDL_FPoint::x);
    this->SDL_Point::y = CLPD_FLT2INT(this->SDL_FPoint::y);
    return *this;
}

float Coords2D::GetEuclideanDistanceTo(const SDL_FPoint& reference) const {
    return sqrt(pow(this->SDL_FPoint::x - reference.x, 2) + pow(this->SDL_FPoint::y - reference.y, 2));
}

int Coords2D::GetManhattanDistanceTo(const SDL_Point& reference) const {
    return abs(this->SDL_Point::x - reference.x) + abs(this->SDL_Point::y - reference.y);
}

// Explicit instantiation of all valid templates
template Coords2D::Coords2D(const int& x, const int& y);
template Coords2D::Coords2D(const float& x, const float& y);
template Coords2D::Coords2D(const SDL_Point& sdlPoint);
template Coords2D::Coords2D(const SDL_FPoint& sdlPoint);
template Coords2D& Coords2D::operator=(const SDL_Point& sdlPoint);
template Coords2D& Coords2D::operator=(const SDL_FPoint& sdlPoint);
template Coords2D& Coords2D::operator+(const SDL_Point& sdlDelta);
template Coords2D& Coords2D::operator+(const SDL_FPoint& sdlDelta);
template Coords2D& Coords2D::operator+=(const SDL_Point& sdlDelta);
template Coords2D& Coords2D::operator+=(const SDL_FPoint& sdlDelta);