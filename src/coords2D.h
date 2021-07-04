#ifndef COORDS2D_H
#define COORDS2D_H

#include <utility>
#include "SDL.h"

// TODO: comment
class Coords2D {
private:
 typedef struct Coords2DReal {
  float x;
  float y;
 } Coords2DReal;

 typedef struct Coords2DInteger {
  int x;
  int y;
 } Coords2DInteger;

public:
 Coords2D();
 Coords2D(const std::pair<float,float>& point);
 Coords2D(const std::pair<int,int>& point);
 Coords2D(const SDL_Point& point);
 Coords2D(const Coords2D& point);
 Coords2D(Coords2D&& point);
 ~Coords2D() {}

 Coords2D& operator=(const std::pair<float,float>& point);
 Coords2D& operator=(const std::pair<int,int>& point);
 Coords2D& operator=(const SDL_Point& point);
 Coords2D& operator=(const Coords2D& point);
 Coords2D& operator=(Coords2D&& point);

 bool operator==(const Coords2D& point);
 bool operator==(Coords2D&& point);

private:
 Coords2DReal real;
 Coords2DInteger integer;
};

#endif