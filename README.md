ga.cpp - This file contains the main function and all of my code.

bitmap_image.hpp - This is an excellent, open source bitmap library. I am not the author, but it's use is allowed under the agreement https://opensource.org/licenses/cpl1.0.php. See https://github.com/ArashPartow/bitmap for more information.

This program uses a genetic algorithm to find "solutions" to the traveling salesman problem. We create a map (a set of cities), consisting of cities (ordered pairs in a 2d integer lattice). A tour is an itinerary (an ordering of the cities to be visited) passing through all of the cities in the map and returning to the city from which it started. Clearly, such an itinerary is a closed path, so we should consider itineraries modulo cyclic permutation. An easy way to do this is to require that all tours begin from the same city.

We build a population of random tours. We evolve the population by allowing tours to mate with each other, producing baby tours, and mutating the resulting baby tours. (We always keep the best tour unchanged.) After a few generations, we expect that a good enough tour has evolved.

The program can draw a graphical representation of the shortest tour at any given moment.
