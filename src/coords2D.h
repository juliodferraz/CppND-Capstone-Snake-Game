#ifndef COORDS2D_H
#define COORDS2D_H

#include <utility>
#include "SDL.h"

// TODO: comment
bool operator==(const SDL_Point& a, const SDL_Point& b);
int GetManhattanDistance(const SDL_Point& a, const SDL_Point& b);

// TODO: comment
class Coords2D : public SDL_Point, public SDL_FPoint {
public:
 Coords2D();
 template<typename T> Coords2D(const T& x, const T& y);
 template<typename T> Coords2D(const T& sdlPoint);
 Coords2D(const Coords2D& point);
 Coords2D(Coords2D&& point);
 ~Coords2D() {}

 Coords2D& operator=(const Coords2D& point);
 Coords2D& operator=(Coords2D&& point);
 template<typename T> Coords2D& operator=(const T& sdlPoint);
 
 bool operator==(const SDL_Point& point);
 
 template<typename T> Coords2D& operator+(const T& sdlDelta);
 template<typename T> Coords2D& operator+=(const T& sdlDelta);

 float GetEuclideanDistanceTo(const SDL_FPoint& reference) const;
 int GetManhattanDistanceTo(const SDL_Point& reference) const;

 int GetIntX() const { return this->SDL_Point::x; }
 int GetIntY() const { return this->SDL_Point::y; }
 float GetRealX() const { return this->SDL_FPoint::x; }
 float GetRealY() const { return this->SDL_FPoint::y; }
};

#endif