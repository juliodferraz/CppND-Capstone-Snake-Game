#include "coords2D.h"
#include <cmath>
#include <climits>
#include <stdexcept>
#include "clip.h"

bool operator==(const SDL_Point& a, const SDL_Point& b) {
    return (a.x == b.x) && (a.y == b.y);
}

unsigned long int GetManhattanDistance(const SDL_Point& a, const SDL_Point& b) {
    // Due to the possible range of int values, in order to prevent data loss, returned value shall be of unsigned long int type.
    return (unsigned long int) CLPD_INT_ABSDIFF(a.x, b.x) + (unsigned long int) CLPD_INT_ABSDIFF(a.y, b.y);
}

Coords2D::Coords2D() : SDL_Point{0,0}, SDL_FPoint{0.0,0.0} {}

Coords2D::Coords2D(const int x, const int y) : 
    SDL_Point{x, y}, 
    SDL_FPoint{(float) x, (float) y} {}

Coords2D::Coords2D(const float& x, const float& y) :  
    SDL_FPoint{CLPD_FLT_INTLIMS(x), CLPD_FLT_INTLIMS(y)} {
    this->SDL_Point::x = (int) this->SDL_FPoint::x;
    this->SDL_Point::y = (int) this->SDL_FPoint::y;
}

Coords2D::Coords2D(const SDL_Point& sdlPoint) :
    SDL_Point{sdlPoint}, 
    SDL_FPoint{(float) sdlPoint.x, (float) sdlPoint.y} {}

Coords2D::Coords2D(const SDL_FPoint& sdlPoint) :
    SDL_FPoint{CLPD_FLT_INTLIMS(sdlPoint.x), CLPD_FLT_INTLIMS(sdlPoint.y)} {
    this->SDL_Point::x = (int) this->SDL_FPoint::x;
    this->SDL_Point::y = (int) this->SDL_FPoint::y;
}
    

Coords2D& Coords2D::operator=(const SDL_Point& sdlPoint) {
    this->SDL_Point::x = sdlPoint.x;
    this->SDL_Point::y = sdlPoint.y;
    this->SDL_FPoint::x = (float) sdlPoint.x;
    this->SDL_FPoint::y = (float) sdlPoint.y;
    return *this;
}

Coords2D& Coords2D::operator=(const SDL_FPoint& sdlPoint) {
    this->SDL_FPoint::x = CLPD_FLT_INTLIMS(sdlPoint.x);
    this->SDL_FPoint::y = CLPD_FLT_INTLIMS(sdlPoint.y);
    this->SDL_Point::x = (int) this->SDL_FPoint::x;
    this->SDL_Point::y = (int) this->SDL_FPoint::y;
    return *this;
}

bool Coords2D::operator==(const SDL_Point& point) {
    return (this->SDL_Point::x == point.x) && (this->SDL_Point::y == point.y);
}

template<typename T>
Coords2D Coords2D::operator+(const T& sdlDelta) const {
    Coords2D displacedCoords;
    // Displace the continuous/float coordinates first.
    displacedCoords.SDL_FPoint::x = CLPD_FLT_SUM(this->SDL_FPoint::x, (float) sdlDelta.x);
    displacedCoords.SDL_FPoint::y = CLPD_FLT_SUM(this->SDL_FPoint::y, (float) sdlDelta.y);
    // Guarantee the continuous/float coordinates are inside integer range, by clipping them at int limits.
    displacedCoords.SDL_FPoint::x = CLPD_FLT_INTLIMS(displacedCoords.SDL_FPoint::x);
    displacedCoords.SDL_FPoint::y = CLPD_FLT_INTLIMS(displacedCoords.SDL_FPoint::y);
    // Set the int coordinates using the clipped float coordinates. 
    displacedCoords.SDL_Point::x = (int) displacedCoords.SDL_FPoint::x;
    displacedCoords.SDL_Point::y = (int) displacedCoords.SDL_FPoint::y;
    return displacedCoords;
}

template<typename T>
Coords2D& Coords2D::operator+=(const T& sdlDelta) {
    // Displace the continuous/float coordinates first.
    this->SDL_FPoint::x = CLPD_FLT_SUM(this->SDL_FPoint::x, (float) sdlDelta.x);
    this->SDL_FPoint::y = CLPD_FLT_SUM(this->SDL_FPoint::y, (float) sdlDelta.y);
    // Guarantee the continuous/float coordinates are inside integer range, by clipping them at int limits.
    this->SDL_FPoint::x = CLPD_FLT_INTLIMS(this->SDL_FPoint::x);
    this->SDL_FPoint::y = CLPD_FLT_INTLIMS(this->SDL_FPoint::y);
    // Set the int coordinates using the clipped float coordinates.
    this->SDL_Point::x = (int) this->SDL_FPoint::x;
    this->SDL_Point::y = (int) this->SDL_FPoint::y;
    return *this;
}

float Coords2D::GetEuclideanDistanceTo(const SDL_FPoint& reference) const {
    // As the coordinates are controlled to be inside integer limits, there's no risk of data loss in returning a float number. 
    // (Considering the "pow" and "sqrt" operations return double-precision values)
    return sqrt(pow(this->SDL_FPoint::x - reference.x, 2) + pow(this->SDL_FPoint::y - reference.y, 2));
}

unsigned long int Coords2D::GetManhattanDistanceTo(const SDL_Point& reference) const {
    return (unsigned long int) CLPD_INT_ABSDIFF(this->SDL_Point::x, reference.x) 
            + (unsigned long int) CLPD_INT_ABSDIFF(this->SDL_Point::y, reference.y);
}

// Explicit instantiation of all valid templates for Coords2D sum operators.
template Coords2D Coords2D::operator+(const SDL_Point& sdlDelta) const;
template Coords2D Coords2D::operator+(const SDL_FPoint& sdlDelta) const;
template Coords2D& Coords2D::operator+=(const SDL_Point& sdlDelta);
template Coords2D& Coords2D::operator+=(const SDL_FPoint& sdlDelta);

Direction2D GetLeftOf(const Direction2D reference) {
  return static_cast<Direction2D>((static_cast<uint8_t>(reference) + 3) % 4); 
}

Direction2D GetRightOf(const Direction2D reference) { 
  return static_cast<Direction2D>((static_cast<uint8_t>(reference) + 1) % 4); 
}

Direction2D GetOppositeOf(const Direction2D reference) { 
  return static_cast<Direction2D>((static_cast<uint8_t>(reference) + 2) % 4); 
}

SDL_Point GetAdjPosition(const SDL_Point& position, const Direction2D direction) {
  SDL_Point output = position;

  switch (direction) {
    case Direction2D::Up:
      if (position.y == INT_MIN) throw std::runtime_error("2D coordinates space representation limit reached.");
      output.y = CLPD_INT_SUM(output.y, -1);
      break;
    case Direction2D::Right:
      if (position.x == INT_MAX) throw std::runtime_error("2D coordinates space representation limit reached.");
      output.x = CLPD_INT_SUM(output.x, 1);
      break;
    case Direction2D::Down:
      if (position.y == INT_MAX) throw std::runtime_error("2D coordinates space representation limit reached.");
      output.y = CLPD_INT_SUM(output.y, 1);
      break;
    case Direction2D::Left:
      if (position.x == INT_MIN) throw std::runtime_error("2D coordinates space representation limit reached.");
      output.x = CLPD_INT_SUM(output.x, -1);
      break;
    default:
      break;
  }
  
  return output;
}

SDL_Point GetVersor(const SDL_Point& origin, const SDL_Point& dest, const Direction2D refDir) {
  SDL_Point output;
  output.x = CLPD_INT_SUM(dest.x, -origin.x);
  output.y = CLPD_INT_SUM(dest.y, -origin.y);

  int aux;
  switch (refDir) {
    case Direction2D::Up:
      // Do nothing
      break;
    case Direction2D::Right:
      aux = output.x;
      output.x = output.y;
      output.y = -aux;
      break;
    case Direction2D::Down:
      output.x = -output.x;
      output.y = -output.y;
      break;
    case Direction2D::Left:
      aux = output.x;
      output.x = -output.y;
      output.y = aux;
      break;
    default:
      break;
  }
  
  return output;
}