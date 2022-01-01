#ifndef COORDS2D_H
#define COORDS2D_H

#include "SDL.h"

/**
 *  \brief "Equal" condition operator for the SDL_Point struct defined in SDL2 library.
 *  \param a First point.
 *  \param b Second point.
 *  \return Boolean indicating true if points are equal, or false otherwise.
 */
bool operator==(const SDL_Point& a, const SDL_Point& b);

/**
 *  \brief Calculates the Manhattan distance between two SDL_Point objects.
 *  \param a First point.
 *  \param b Second point.
 *  \return Unsigned long integer representing the Manhattan distance between the two points.
 */
unsigned long int GetManhattanDistance(const SDL_Point& a, const SDL_Point& b);

/**
 *  \brief Class representing a point in a 2D space, encapsulating both its discrete and continuous coordinates, and 
 * guaranteeing they are coherent and inside valid data range.
 * Inherits from both SDL_Point (discrete) and SDL_FPoint (continuous) structs of the SDL2 library.
 */
class Coords2D : public SDL_Point, public SDL_FPoint {
 public:
  /**
   *  \brief Default constructor (origin at zero).
   */
  Coords2D();

  /**
   *  \brief Constructor using int inputs.
   *  \param x X-axis coordinate.
   *  \param y Y-axis coordinate.
   */
  Coords2D(const int x, const int y);

  /**
   *  \brief Constructor using float inputs.
   *  \param x X-axis coordinate.
   *  \param y Y-axis coordinate.
   */
  Coords2D(const float& x, const float& y);

  /**
   *  \brief Constructor using SDL2 SDL_Point (int) discrete coordinates.
   *  \param sdlPoint SDL2 discrete coordinates to be copied for Coords2D initialization.
   */
  Coords2D(const SDL_Point& sdlPoint);

  /**
   *  \brief Constructor using SDL2 SDL_FPoint (float) continuous coordinates.
   *  \param sdlPoint SDL2 continuous coordinates to be copied for Coords2D initialization.
   */
  Coords2D(const SDL_FPoint& sdlPoint);

  /**
   *  \brief Assignment operator for usage with SDL_Point type.
   *  \param sdlPoint Point representing the discrete coordinates to be assigned.
   */
  Coords2D& operator=(const SDL_Point& sdlPoint);

  /**
   *  \brief Assignment operator for usage with SDL_FPoint type.
   *  \param sdlPoint Point representing the continuous coordinates to be assigned.
   */
  Coords2D& operator=(const SDL_FPoint& sdlPoint);

  /**
   *  \brief Comparison operator for usage with SDL2 struct types (i.e. SDL_Point or SDL_FPoint).
   *  \param point SDL discrete point representing the coordinates to be compared to.
   *  \return Boolean indicating true if object's discrete position is equal to argument; or false otherwise.
   */
  bool operator==(const SDL_Point& point);
 
  /**
   *  \brief Sum operator for usage with SDL2 struct types (i.e. SDL_Point or SDL_FPoint).
   *  \param sdlDelta Struct representing the displacement to be summed with the Coords2D object.
   *  \return New Coords2D object pointing to the displaced coordinates.
   */
  template<typename T> Coords2D operator+(const T& sdlDelta) const;

  /**
   *  \brief Sum and assignment operator for usage with SDL2 struct types (i.e. SDL_Point or SDL_FPoint).
   *  \param sdlDelta Struct representing the displacement to be summed with the Coords2D object.
   *  \return Reference to the same Coords2D object, now with displaced coordinates.
   */
  template<typename T> Coords2D& operator+=(const T& sdlDelta);

  /**
   *  \brief Calculates and returns the Euclidean distance between the object and a SDL2 continuous coordinate.
   *  \param reference Struct representing the coordinates for which to calculate the distance to.
   *  \return The Euclidean distance.
   */
  float GetEuclideanDistanceTo(const SDL_FPoint& reference) const;

  /**
   *  \brief Calculates and returns the Manhattan distance between the object and a SDL2 discrete coordinate.
   *  \param reference Struct representing the coordinates for which to calculate the distance to.
   *  \return The Manhattan distance, in unsigned long integer format (to prevent data loss, due to int range of inputs).
   */
  unsigned long int GetManhattanDistanceTo(const SDL_Point& reference) const;

  /**
   *  \brief Inline accessor for the discrete X-axis coordinate.
   *  \return Integer value.
   */
  inline int GetIntX() const { return this->SDL_Point::x; }

  /**
   *  \brief Inline accessor for the discrete Y-axis coordinate.
   *  \return Integer value.
   */
  inline int GetIntY() const { return this->SDL_Point::y; }

  /**
   *  \brief Inline accessor for the continuous X-axis coordinate.
   *  \return Float value.
   */
  inline float GetRealX() const { return this->SDL_FPoint::x; }

  /**
   *  \brief Inline accessor for the continuous Y-axis coordinate.
   *  \return Float value.
   */
  inline float GetRealY() const { return this->SDL_FPoint::y; }
};

/**
 *  \brief Global 2D direction enum. Values are clockwise ordered.
 */
enum class Direction2D { Up = 0, Right = 1, Down = 2, Left = 3};

/**
 *  \brief Returns the direction located left (relatively) of the input direction.
 *  \param reference Reference direction.
 *  \return Direction located left of the input one.
 */
Direction2D GetLeftOf(const Direction2D reference);

/**
 *  \brief Returns the direction located right (relatively) of the input direction.
 *  \param reference Reference direction.
 *  \return Direction located right of the input one.
 */
Direction2D GetRightOf(const Direction2D reference);

/**
 *  \brief Returns the direction contrary to the input direction.
 *  \param reference Reference direction.
 *  \return Direction opposite to the input one.
 */
Direction2D GetOppositeOf(const Direction2D reference);

/**
 *  \brief Returns the point adjacent to the input position, in the input direction.
 * If the input position is already at a limit of the 2D space representation, and an adjacent position cannot
 * be reached in the intended direction, a runtime exception is thrown, and the same input position is returned.
 *  \param position Reference position.
 *  \param direction Reference direction.
 *  \return 2D coordinates adjacent to the input position in the input direction.
 */
SDL_Point GetAdjPosition(const SDL_Point& position, const Direction2D direction);

/**
 *  \brief Returns the dimensions of a versor pointing from "origin" to "dest", relative to "refDir" direction.
 * The versor dimensions are truncated at the integer representation limits.
 *  \param origin Origin of the versor.
 *  \param dest Coordinates to where the versor points.
 *  \param refDir Reference direction (intended direction for the upper Y-axis of the versor 2D space).
 *  \return The mapping of the versor going from "origin" to "dest" considering "refDir" direction as the upper Y-axis.
 */
SDL_Point GetVersor(const SDL_Point& origin, const SDL_Point& dest, const Direction2D refDir);

#endif