#include <iostream>
#include "grid.h"
#include <unistd.h>

// desc : Reports whether or not the input coordinates correspond
//        to a valid position in the grid
// pre  : None
// post : None, aside from description
bool Grid::exists(int x, int y){
    if( (x < 0) || (x >= width) ){
        return false;
    } else if( (y < 0) || (y >= height)){
        return false;
    }
    return true;
}

// desc : Returns whether or not the cell/tile at the input coordinates
//        is alive.
// pre  : Coordinates must be valid for the grid
// post : None, aside from description
bool Grid::get_tile(int x, int y) {
    int index = (width+1) * y + x;
    return buffer[index];
}

// desc : Sets whether or not the cell/tile at the input coordinates
//        is alive.
// pre  : Coordinates must be valid for the grid
// post : None, aside from description
void Grid::set_tile(int x, int y, bool value){
    int index = (width+1) * y + x;
    buffer[index] = value;
}

// desc : Returns grid width
// pre  : None
// post : None, aside from description
int Grid::get_width(){
    return width;
}

// desc : Returns grid height
// pre  : None
// post : None, aside from description
int Grid::get_height(){
    return height;
}

// desc : Overwrites the state of the tile identified by the input
//        coordinates, following the rules of Conway's Game of Life and
//        using the input grid (other) as the state of the preceding
//        generation.
// pre  : Coordinates must be valid for the grid
// post : None, aside from description
void Grid::update_tile(Grid& other, int x, int y, int rule){
    // Intentional delay
    usleep(100);

    // Check if cell was alive in previous state
    bool alive = other.get_tile(x,y);

    // For each cell in the 3x3 grid centered on the
    // input coordinates in the input Grid 'other'...
    int count  = 0;
    for(int i=-1; i<=1; i++){
        for(int j=-1; j<=1; j++){
            // ... do nothing for the cell exactly
            // at (x,y)
            if( (i==0) && (j==0) ){
                continue;
            }

            // Add one to the count if adjacent
            // cells are alive
            int query_x = x+i;
            int query_y = y+j;
            if( !other.exists(query_x,query_y)){
                continue;
            }
            if( other.get_tile(query_x,query_y) ){
                count += 1;
            }
        }
    }

    bool end_state = (rule>>((alive*9)+count)) & 1;
    set_tile(x,y,end_state);
}

// desc : Creates a grid with dimensions matching the input height and
//        width, initializing all tiles as 'dead'
// pre  : Width and height must be positive
// post : None, aside from description
Grid::Grid(int w, int h)
    : height(h)
    , width(w)
{
    buffer = new bool[(width+1)*height];
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            set_tile(j,i,false);
        }
    }
    for(int i=0; i<height; i++){
        buffer[(width+1)*i+width] = '\n';
    }
}

// desc : Creates a grid with dimensions and tile states matching the
//        content of the input file, with each line interpreted as a row
//        and all non-space characters counted as 'alive'.
// pre  : `file_path` must correspond to a valid file
// post : None, aside from description
Grid::Grid(std::string file_path)
{
    std::ifstream file;
    file.open(file_path);
    width  = 0;
    height = 0;
    std::string line;

    // Determine height by the number of lines
    // Determine width  by the maximum line width
    while(std::getline(file,line)){
        int line_size = line.size();
        width = (width>line_size) ? width : line.size();
        height++;
    }

    // Allocate character buffer to store tile data
    buffer = new bool[(width+1)*height];

    // Reset position in the file to the start
    file.clear();
    file.seekg(0,std::ios::beg);

    // Initialize each row based off of each line in
    // the file
    int y = 0;
    while(std::getline(file,line)){
        for(int x=0; x<width; x++){
            bool val = false;
            int line_size = line.size();
            if( x<line_size ){
                val = line[x] != ' ';
            }
            this->set_tile(x,y,val);
        }
        buffer[(width+1)*y+width] = '\n';
        y++;
    }
}

// desc : Frees the grid's buffer
// pre  : None
// post : None, aside from description
Grid::~Grid(){
    delete[] buffer;
}

