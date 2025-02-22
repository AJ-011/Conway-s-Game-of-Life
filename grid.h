#ifndef GRID
#define GRID

#include <fstream>
#include <vector>
#include "tui.h"

///////////////////////////////////////////////////////////
// Represents a grid of tiles in Conways Game of Life.
///////////////////////////////////////////////////////////
class Grid {

    int   height;
    int   width;
    bool *buffer;

    public:

    // desc : Reports whether or not the input coordinates correspond
    //        to a valid position in the grid
    // pre  : None
    // post : None, aside from description
    bool exists(int x, int y);


    // desc : Sets whether or not the cell/tile at the input coordinates
    //        is alive.
    // pre  : Coordinates must be valid for the grid
    // post : None, aside from description
    void set_tile(int x, int y, bool value);

    public:

    // desc : Returns whether or not the cell/tile at the input coordinates
    //        is alive.
    // pre  : Coordinates must be valid for the grid
    // post : None, aside from description
    bool get_tile(int x, int y);

    // desc : Overwrites the state of the tile identified by the input
    //        coordinates, following the rules of Conway's Game of Life and
    //        using the input grid (other) as the state of the preceding
    //        generation.
    // pre  : Coordinates must be valid for the grid
    // post : None, aside from description
    void update_tile(Grid& other, int x, int y, int rule);

    // desc : Returns grid width
    // pre  : None
    // post : None, aside from description
    int get_width();

    // desc : Returns grid height
    // pre  : None
    // post : None, aside from description
    int get_height();

    // desc : Creates a grid with dimensions matching the input height and
    //        width, initializing all tiles as 'dead'
    // pre  : Width and height must be positive
    // post : None, aside from description
    Grid(int w, int h);

    // desc : Creates a grid with dimensions and tile states matching the
    //        content of the input file, with each line interpreted as a row
    //        and all non-space characters counted as 'alive'.
    // pre  : `file_path` must correspond to a valid file
    // post : None, aside from description
    Grid(std::string file_path);

    // desc : Frees the grid's buffer
    // pre  : None
    // post : None, aside from description
    ~Grid();

};

#endif //GRID
