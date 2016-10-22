// This program uses a genetic algorithm to find "solutions" to the traveling salesman problem.
// We make a map consisting of cities.
// We make a population of tours (i.e., itineraries that start and end at one city and visit every city on the map).
// We evolve the population of tours, hoping that we eventually evolve one that's short enough.

#include <cmath> // sqrt
#include <cstdlib> // rand, srand
#include <ctime> // time

#include <iostream> // We use standard console input and output.
#include <string> // We use getline(istream &, string &).

#include <algorithm> // find, max_element, random_shuffle
#include <vector> // We use vectors extensively.

#include "bitmap_image.hpp" // We use this excellent, open source bitmap library.
// It is obtained from https://github.com/ArashPartow/bitmap
// It is provided under the following agreement: https://opensource.org/licenses/cpl1.0.php

using namespace std; // I want to avoid writing "std::" over and over.

// Get one character and ignore the rest, until a new line is reached.
char getOneChar(istream &is = cin)
{
 string str;
 getline(is, str);
 return str[0];
}

// Return a random integer in [a, b).
unsigned int randomIndex(const unsigned int &a, const unsigned int &b)
{
 return (rand() % (b - a)) + a;
}

// Return a random double in [a, b].
double randomDouble(const double &a = 0, const double &b = 1)
{
 return (static_cast<double>(rand()) / RAND_MAX) * (b - a) + a;
}

// A city is just a an ordered pair of integers in [0, width)x[0, height), where width and height are positive integers.
class City {
 public:
  unsigned int x;
  unsigned int y;

  // Construct a random city, i.e., a city whose coordinates are randomly chosen in [0, width)x[0, height).
  City(const unsigned int &width, const unsigned int &height)
  {
   x = rand() % width;
   y = rand() % height;
  }
};

// We have to define == in order to use find with the class City.
bool operator ==(const City &a, const City &b)
{
 return a.x == b.x && a.y == b.y;
}

// Return the Euclidean distance between a and b as a double.
double distanceBetweenCities(const City &a, const City &b)
{
 return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

// For the most part, a map is just a list of cities that should be visited along a tour.
// To this end, the class Map is derived from the class vector.
// For convenience, we also record the _width and _height for which all cities belong to [0, _width)x[0, _height).
class Map : public vector<City> {
 private:
  unsigned int _width;
  unsigned int _height;
 public:

  // Create a map of width w and height h, containing n distinct, random cities.
  // The parameters w, h, and n should all be positive integers.
  Map(const unsigned int &w, const unsigned int &h, const unsigned int &n) : _width(w), _height(h)
  {
   // Keep adding random cities until we have n of them.
   while (size() < n)
   {
    City city(_width, _height); // Create a random city.
    if (find(begin(), end(), city) == end()) // Check whether this random city has already been added.
    {
     push_back(city); // If this random city is distinct from those cities already added, then add it.
    }
   }
  }

  // The cities on our map are recorded in a vector of cities.
  // This function returns the Euclidean distance between the city at index i and the city at index j.
  // The parameters i and j should be in [0, size()).
  double distance(const unsigned int &i, const unsigned int &j) const
  {
   return distanceBetweenCities((*this)[i], (*this)[j]);
  }

  unsigned int width() const
  {
   return _width;
  }

  unsigned int height() const
  {
   return _height;
  }
};

// The parameter itinerary, which in the following function is a vector of unsigned integers, indicates the order in which the cities on our map are to be visited.
// If N is equal to map.size(), then any itinerary we would like to consider is just a permutation of the N-1 last elements of the ordered set (0, 1, ..., N-1).
// Return the Euclidean length of the itinerary, beginning and ending at the city map[itinerary[0]].
double lengthOfItinerary(const vector<unsigned int> &itinerary, const Map &map)
{
 unsigned int i;
 double length = map.distance(itinerary[0], itinerary.back());
 for (i = 1; i < itinerary.size(); i ++)
 {
  length += map.distance(itinerary[i - 1], itinerary[i]);
 }
 return length;
}

// A tour is an itinerary together with the itinerary's Euclidean length.
// The itinerary will be followed, starting with the city indicated by its first element and finishing at the same city.
// Hence, the itinerary forms a closed path.
// Any cyclic permutation of the itinerary determines the same closed path, so we kill this redundancy by requiring all itineraries to have the same first element.
// The reason we record the length, and not just the itinerary, is that we want to avoid having to compute the length each time we need it.
class Tour : public vector<unsigned int> {
 private:
  double _length;
 public:

  // Create a random tour of the cities in map.
  explicit Tour(const Map &map)
  {
   // Add the numbers 0, 1, ..., map.size()-1 to the itinerary on which this tour is based.
   unsigned int i;
   for (i = 0; i < map.size(); i ++)
   {
    push_back(i);
   }

   random_shuffle(begin() + 1, end()); // Make the itinerary random by shuffling all but the first element.

   _length = lengthOfItinerary(*this, map); // Record the length of the resulting itinerary.
  }

  // Create a tour based on itinerary and map.
  Tour(const vector<unsigned int> &itinerary, const Map &map)
  {
   assign(itinerary.begin(), itinerary.end()); // Record the indicated itinerary.

   _length = lengthOfItinerary(*this, map);// Record the length of the itinerary.
  }

  const double &length() const
  {
   return _length;
  }

  // Consider three kinds of changes (i.e., mutations) to an itinerary that can shorten it.
  // One way is to swap two cities.
  // Another way is to reverse the order of a subsequence of cities.
  // Yet one more way is to apply a cyclic permutation to a subsequence of cities.
  // The parameter p in [0, 1] indicates the probability with which a mutation occurs.
  // Since we are dealing with tours, not just itineraries, we want to record the length of the mutated itinerary; this requires that we have the corresponding map as well.
  // Return the type of mutation that we performed.
  // (At the moment, nothing in this program actually cares what kind of mutation we performed, but it might be interesting to keep a record of it in a later version of this program.)
  int mutate(const double &p, const Map &map)
  {
   // Randomly decide whether to perform a mutation.
   if (randomDouble(0, 1) > p) // In this case, don't perform a mutation.
   {
    return -1;
   }

   int mutation; // The is where we will record the type of mutation performed.
   // The meaning of the integer mutation is indicated in the switch statement below.

   // Get random indices i and j in [0, size()), with i < j.
   unsigned int i = randomIndex(1, size() - 1);
   unsigned int j = randomIndex(i + 1, size());

   // Given any indices i and j as above, we can certainly perform swap and reverse mutations.
   // However, a rotation requires that there is some index in between i and j.
   while (true)
   {
    mutation = rand() % 3; // Randomly choose a mutation type.

    // Try to perform a mutation.
    switch (mutation)
    {
     case 0:
      ::swap((*this)[i], (*this)[j]);
     break;
     case 1:
      reverse(begin() + i, begin() + j + 1);
     break;
     case 2:
      if (j - i > 2) // If there is an index in between i and j, perform a rotation.
      {
       rotate(begin() + i, begin() + randomIndex(i + 1, j), begin() + j + 1); // Randomly choose an index in between i and j, and perform the corresponding rotation.
      }
      else // In this case, i and j are consecutive, so we can only hope to do a swap or reverse mutation.
      {
       continue; // Go back to the beginning: Randomly choose a mutation, and see if we can perform it.
      }
     break;
    }

    // If we reach this point, we have successfully performed a mutation.
    break; // Don't try to perform any more mutations.
   }

   _length = lengthOfItinerary(*this, map); // Record the length of the mutated itinerary.

   return mutation; // Return the type of mutation.
  }
};

// Take two tours as parameters, and combine them to make a better tour.
// The algorithm to construct itinerary from a and b is straightforward:
/*
 1) Add the initial city (i.e., the city from which all tours begin and end), to itinerary.
 2) Find the first cities from a and b that haven't been added to itinerary.
 3) Among these two cities, add to itinerary the one nearer to the last citi in itinerary.
 4) Go back to step (2).
*/
// Clearly, the resulting itinerary can be no worse than the shorter of a and b.
// Return the tour based on the itinerary created in the algorithm above.
// (Call the function sex for fun!)
// Tours a and b should both be based on map.
Tour sex(const Tour &a, const Tour &b, const Map &map)
{
 unsigned int i = 1; // This is the position from which we should begin searching a.
 unsigned int j = 1; // This is the position from which we should begin searching b.

 vector<unsigned int> itinerary; // This is the itinerary we want to create.

 itinerary.push_back(0); // Set the first city to be the same as the first city of all the itineraries under consideration.

 while (itinerary.size() < map.size())
 {
  // In what follows, a legitimate city is one that does not yet appear in itinerary.

  // Find next legitimate city in a.
  while (find(itinerary.begin(), itinerary.end(), a[i]) != itinerary.end())
  {
   i ++;
  }
  // Now, either i is equal to a.size(), or a[i] is the next legitimate city in a.

  // Do the same thing for b.
  while (find(itinerary.begin(), itinerary.end(), b[j]) != itinerary.end())
  {
   j ++;
  }

  if (i == a.size()) // We've reached the end of a, so the only remaining cities to add are those in b.
  {
   itinerary.push_back(b[j]); // Add the next legitimate city in b.
   j ++;
  }
  else if (j == b.size()) // We've reached the end of b, so the only remaining cities to add are those in a.
  {
   itinerary.push_back(a[i]); // Add the next legitimate city in a.
   i ++;
  }
  else // We've reached the end of neither a nor b.
  {
   // Add the next legitimate city from a or b nearest to the last city added to itinerary.
   if (map.distance(itinerary.back(), a[i]) < map.distance(itinerary.back(), b[j])) // The next legitimate city from a is closer.
   {
    itinerary.push_back(a[i]);
    i ++;
   }
   else // The next legitimate city from b is no worse than that of a.
   {
    itinerary.push_back(b[j]);
    j ++;
   }
  }

  // Repeat this whole process until we reach the end of both a and b.
  // This is clearly equivalent to itinerary having the same size as a, b, or map (all of which have equal size).
 }

 return Tour(itinerary, map); // Return the tour based on the itinerary we created.
}

// We have to define < in order to use max_element.
bool operator <(const Tour &a, const Tour &b)
{
 return a.length() > b.length(); // This is equivalent to returning 1 / a.length() < 1 / b.length().
}

// The class Population consists of a map and a population of tours based on the map.
// It also handles evolution, the basis of the genetic algorithm.
class Population {
 private:
  Map map;

  vector<Tour> tours; // The population of individual tours.
  // These will be evolved in the course of the genetic algorithm.

  // Choose a tour at random from tours, and return it.
  // Depth should be a positive integer less than tours.size().
  // (Of course, there are many ways to choose a good parent...)
  Tour &findParent(const unsigned int &depth)
  {
   random_shuffle(tours.begin(), tours.end());
   return *max_element(tours.begin(), tours.begin() + depth);
  }

 public:

  // Construct a population, consisting of n_tours tours, based on a map, consisting of n_cities cities, of the indicated width and height.
  Population(const unsigned int &width, const unsigned int &height, const unsigned int &n_cities, const unsigned int &n_tours) : map(width, height, n_cities)
  {
   // Add random individual tours to the population of tours until we have enough of them.
   while (tours.size() < n_tours)
   {
/*
    // For genetic diversity, add a random tour that is distinct from those already added.
    // (Of course, the likelihood of duplicating a tour is small...)
    Tour tour(map);
    if (find(tours.begin(), tours.end(), tour) == tours.end()) tours.push_back(tour);
*/

    tours.push_back(Tour(map)); // Add a random tour.
   }
  }

  // Return the shortest tour.
  const Tour &fittest() const
  {
   return *max_element(tours.begin(), tours.end());
  }

  // This is the heart of the genetic algorithm.
  void evolve(const double &p_mutate, const unsigned int &depth)
  {
   vector<Tour> children; // This vector will hold the new generation of tours.

   children.push_back(fittest()); // Keep the best tour that we've already found.

   // Let the tours have sex and make baby tours until we have enough of them.
   while (children.size() < tours.size())
   {
    Tour &a = findParent(depth); // Mother!
    Tour &b = findParent(depth); // Father!
    if (a != b) // If the tours are different, let them have sex.
    {
     children.push_back(sex(a, b, map)); // Add the child tour they conceived.
    }
    else // The tours are identical, so even if they have sex, the resulting child will be the same as a, which is the same as b.
    {
     children.push_back(a); // Everybody's the same...
    }
   }
   // Now, we have made a new generation of baby tours.

   // Randomly perform mutations in order to ensure genetic diversity, but keep unchanged the best tour we've found until this point.
   for (unsigned int i = 1; i < children.size(); i ++)
   {
    children[i].mutate(p_mutate, map);
   }

   tours = children; // Replace the old generation with the new generation.

   return;
  }

  // Return the map on which our population is based.
  const Map &getMap() const
  {
   return map;
  }
};

// This function represents graphically the tour based on the map, by outputting a bitmap image with the indicated file name.
void tourToBMP(const Tour &tour, const Map &map, const char *file_name)
{
 unsigned int i;

 bitmap_image image(map.width(), map.height()); // Create a bitmap of the required dimensions.
 image.set_all_channels(255, 255, 255); // Make the background white.
 image_drawer draw(image); // Use this to draw on the bitmap.
 draw.pen_width(2); // This width looks nice: not too thick, and not too thin.

 // Draw the path indicated by tour as a concatenation of line segments.
 draw.pen_color(0, 0, 0); // Use the color black.
 for (i = 0; i < tour.size() - 1; i ++)
 {
  draw.line_segment(map[tour[i]].x, map[tour[i]].y, map[tour[i + 1]].x, map[tour[i + 1]].y);
 }
 draw.line_segment(map[tour[i]].x, map[tour[i]].y, map[tour[0]].x, map[tour[0]].y);

 // Draw the individual cities on the map.
 draw.pen_color(255, 150, 50); // Use the color orange.
 for (i = 0; i < map.size(); i ++)
 {
  draw.circle(map[i].x, map[i].y, 5); // Draw a little circle.
 }

 image.save_image(file_name); // Output a bitmap file.

 return;
}

int main()
{
 srand(time(0)); // Seed the random number generator in the standard way.

 const unsigned int width = 600; // This is the width of our map.
 const unsigned int height = 400; // This is the height of our map.
 const unsigned int n_cities = 30; // This is the total number of cities on our map.
 const unsigned int n_tours = 150; // This is the total number of tours in our population.

 const unsigned int depth = 10; // This is the depth used for finding a parent.
 const double p_mutate = 0.3; // This is the probability that a mutation occurs.

 const unsigned int n_stop = 100; // This is the stopping condition.
 // If we haven't found a better tour after n_stop generations, then give up looking.

 Population population(width, height, n_cities, n_tours);

 unsigned int n_generations = 0; // This keeps track of which generation the population represents.
 time_t t_total = 0; // This keeps track of the total amount of time (in seconds) spent on the genetic algorithm.

 while (true)
 {
  // Display some information...
  cout << "[Generation #" << n_generations << ']' << endl
       << "Length: " << population.fittest().length() << endl
       << "Elapsed time: " << t_total << " seconds" << endl
       << "Press (enter) to evolve, (b) to draw a picture, or (q) to quit." << endl;

  char ch = getOneChar(); // Get input.

  if (ch == 'b') // Draw a bitmap.
  {
   cout << "Saving bitmap file..." << endl;
   tourToBMP(population.fittest(), population.getMap(), "tour.bmp");
  }
  else if (ch == 'q') // Quit.
  {
   break;
  }
  else // Evolve!
  {
/*
   // Evolve once, and return to the beginning of the loop.
   // (We will check what the user wants to do after each evolution.)
   cout << "Evolving..." << endl;
   double length = population.fittest().length();
   unsigned int i;
   for (i = 1; i <= n_stop; i ++)
   {
    population.evolve(p_mutate, depth);
    if (population.fittest().length() < length)
    {
     cout << "The population improved after " << i << " generations." << endl;
     n_generations += i;
     break;
    }
   }

   // We evolved n_stop times without finding a shorter tour, so if it's reasonable, let's try increasing the likelihood of mutation.
   if (i > n_stop)
   {
    cout << "Nothing improved after " << n_stop << " generations." << endl;
    if (p_mutate < 1.0)
    {
     p_mutate = 1.0;
     cout << "Let's increase the mutation probability to " << p_mutate << '.' << endl;
    }
    n_generations += n_stop;
   }
*/

   // Get ready to evolve.
   cout << "Evolving..." << endl;
   double length = population.fittest().length();
   unsigned int i;
   time_t t_0, t_1;

   // Start performing evolutions.
   t_0 = time(0); // Record the start time.
   do {
    for (i = 0; i < n_stop; i ++)
    {
     population.evolve(p_mutate, depth);
     if (population.fittest().length() < length)
     {
      length = population.fittest().length();
      n_generations += i + 1;
      break;
     }
    }
   } while (i < n_stop);
   t_1 = time(0); // Record the stop time.
   // We've reached the stop condition.

   // Tell the user what happened.
   n_generations += n_stop;
   cout << "We reached the stop condition after " << t_1 - t_0 << " seconds." << endl;
   t_total += t_1 - t_0;
  }

  cout << endl; // Print a line break to keep things pretty.
 }

 return 0;
}
