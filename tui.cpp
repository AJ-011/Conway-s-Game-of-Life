#include "tui.h"

using namespace tui;

// desc : Returns true if `other` represents an equivalent rgb
// pre  : None
// post : None, aside from desc
bool RGB::operator==(RGB const& other) {
    return    (other.red   == red  )
           && (other.green == green)
           && (other.blue  == blue );
}

// desc : Returns false if `other` represents an equivalent rgb
// pre  : None
// post : None, aside from desc
bool RGB::operator!=(RGB const& other) {
    return  ! (*this == other);
}


// desc : Initializes tile as blank (black).
// pre  : None
// post : None, aside from desc
Tile::Tile()
    : symbol(" ")
    , fore_color({0,0,0})
    , back_color({0,0,0})
{}

// desc : Initializes tile as the provided symbol with the given
//        foreground and background color.
// pre  : None
// post : None, aside from desc
Tile::Tile(std::string symbol, RGB fore, RGB back)
    : symbol(symbol)
    , fore_color(fore)
    , back_color(back)
{}

// desc : Initializes tile as a space character using the given 
//        background color.
// pre  : None
// post : None, aside from desc
Tile::Tile(RGB color)
    : symbol(" ")
    , fore_color({0,0,0})
    , back_color(color)
{}


// desc : Return's the tile's string content, preceded by the escapes
//        corresponding to its colors
// pre  : None
// post : None, aside from desc
Tile::operator std::string() {
    std::stringstream ss;
    if(symbol != " "){
        ss << "\033[38;2;"
           << (int) fore_color.red   << ";"
           << (int) fore_color.green << ";"
           << (int) fore_color.blue  << "m";
    }
    ss << "\033[48;2;"
       << (int) back_color.red   << ";"
       << (int) back_color.green << ";"
       << (int) back_color.blue  << "m";
    ss << symbol;
    return ss.str();
}

// desc : Simply returns the tile's string content
// pre  : None
// post : None, aside from desc
std::string Tile::raw_symbol() {
    return symbol;
}

// desc : Initializes the canvas to the provided dimensions at the
//        provided offset
// pre  : None
// post : None, aside from description
Canvas::Canvas(size_t width, size_t height, size_t x, size_t y)
    : width(width)
    , height(height)
    , offset_x(x)
    , offset_y(y)
    , prev_buffer(new Tile[height*width])
    , tile_buffer(new Tile[height*width])
    , should_full_display(true)
{}

// desc/pre/post : Same as constructor above, but with zero offset
Canvas::Canvas(size_t width, size_t height)
    : width(width)
    , height(height)
    , offset_x(0)
    , offset_y(0)
    , prev_buffer(new Tile[height*width])
    , tile_buffer(new Tile[height*width])
    , should_full_display(true)
{}

// desc : Frees the canvas's tile buffers
// pre  : None
// post : None, aside from description
Canvas::~Canvas() {
    delete[] prev_buffer;
    delete[] tile_buffer;
}

// desc : Resizes the canvas to the provided dimensions
// pre  : None
// post : None, aside from description
void Canvas::resize(size_t width, size_t height) {
    // Allocate new buffers matching the desired size
    Tile *new_tile_buffer = new Tile[height*width];
    Tile *new_prev_buffer = new Tile[height*width];
    // Establish bounds for copying tile data
    size_t x_limit = std::min(this->width,width);
    size_t y_limit = std::min(this->width,width);
    // Copy over tile data
    for (size_t y=0; y<y_limit; y++) {
        for (size_t x=0; x<x_limit; x++) {
            new_tile_buffer[y*width+x] = (*this)(x,y);
            new_prev_buffer[y*width+x] = (*this)(x,y);
        }
    }
    // Free the original buffers
    delete[] tile_buffer;
    delete[] prev_buffer;
    // Establish the new buffers in the corresponding members
    tile_buffer  = new_tile_buffer;
    prev_buffer  = new_prev_buffer;
    // Update the width and height members to match the inputs
    this->width  = width;
    this->height = height;
    should_full_display = true;
}

// desc : Repositions the canvas to the provided offset relative
//        to the base cursor positon
// pre  : None
// post : None, aside from description
void Canvas::reposition(size_t x, size_t y) {
    hide();
    offset_x = x;
    offset_y = y;
    full_display();
}

// desc : Returns a reference to the tile at the provided offset
//        (relative to the top-left corner of the canvas)
// pre  : The provided offset must correspond to a valid tile
//        in the canvas's current bounds
// post : None, aside from description
Tile& Canvas::operator()(size_t x, size_t y) {
    if( (x<0) || (x>=width) || (y<0) || (y>=height) ){
        std::stringstream ss;
        ss << "Canvas with dimensions ("
           << width << ',' << height
           << ") accessed out of bounds with coordinates ("
           << x << ',' << y << ')';
        throw std::runtime_error(ss.str());
    }
    return tile_buffer[y*width+x];
}

// desc : Hides the canvas's content by overwriting it with
//        default-colored space character tiles
// pre  : None
// post : None, aside from description
void Canvas::hide() {
    std::string output;
    output += "\033[s";
    // Handle y offset
    if (offset_y != 0){
        output += "\033[" + std::to_string(offset_y) + "B";
    }
    // Switch to default colors
    output += "\033[39m\033[49m";
    for (size_t y=0; y<height; y++) {
        // Handle x offset
        output += "\033[" + std::to_string(offset_x+1) + "G";
        for (size_t x=0; x<width; x++) {
            // Just write spaces everywhere
            output += ' ';
        }
        output += "\r\n";
    }
    output += "\033[u";
    std::cout << output;
    should_full_display = true;
}


// desc : Fully renders the canvas's content
// pre  : None
// post : None, aside from description
void Canvas::full_display() {
    std::string output;
    Tile *last_tile = nullptr;
    output += "\033[s";
    // Handle y offset
    if (offset_y != 0) {
        output += "\033[" + std::to_string(offset_y) + "B";
    }
    bool mismatch;
    for (int y=0; y<height; y++) {
        mismatch = true;
        for (int x=width-1; x>=0; x--) {
            size_t index = y*width+x;
            // Handle x offset
            output += "\033[" + std::to_string(offset_x+x+1) + "G";

            Tile &current_tile = tile_buffer[index];
            if (last_tile != nullptr) {
                mismatch |= last_tile->fore_color != current_tile.fore_color;
                mismatch |= last_tile->back_color != current_tile.back_color;
            }
            last_tile = &current_tile;
            // Write out tile, avoiding escapes if they aren't necessary
            if (mismatch) {
                output += (std::string) current_tile;
            } else {
                output += current_tile.raw_symbol();
            }
            mismatch = false;
            // Record the state of the written tile
            prev_buffer[index] = current_tile;
        }
        // Escape to default colors when  moving to the next line
        output += "\033[39m\033[49m";
        output += "\r\n";
    }
    output += "\033[u";
    std::cout << output;
    should_full_display = false;
}


// desc : Updates a canvas's content, assuming it has previously been
//        fully displayed and that only changed tiles need to update
// pre  : None, aside from description
// post : None, aside from description
void Canvas::lazy_display() {
    std::string output;
    Tile *last_tile = nullptr;

    // Save cursor position
    output += "\033[s";

    // Handle y offset
    if (offset_y != 0) {
        output += "\033[" + std::to_string(offset_y) + "B";
    }

    // Save position of the last tile we had to update
    int last_y = 0;
    int last_x = -1;

    bool first = true;
    for (int y=0; y<height; y++) {
        // Indicates the first tile in a row
        for (int x=width-1; x>=0; x--) {

            // Get references to the previously displayed tile state for
            // this positon and the state that must now be displayed
            int index = y*width+x;
            Tile &prev_state = prev_buffer[y*width+x];
            Tile &next_state = tile_buffer[y*width+x];

            // Current tile needs to be updated if:
            //     - foreground color changes
            //     - background color changes
            //     - symbol changes
            bool touched = (prev_state.fore_color != next_state.fore_color);
            touched = touched || (prev_state.back_color != next_state.back_color);
            touched = touched || (prev_state.symbol != next_state.symbol);

            // Only re-display a tile if it an update is required
            if (touched) {

                // Adjust cursor x and y coordinates from last displayed tile
                // position to the current position

                // We move the cursor horizontally from right to left and by
                // absolute positon to account for multi-column symbols (eg: emoji)
                output += "\033[" + std::to_string(offset_x+x+1) + "G";

                // We move the cursor from the top down and by relative position so that
                // we can lock the canvas to a specific scroll position, meaning we don't
                // destroy any of the terminal's previously printed lines.
                if ( y != last_y ) {
                    int delta = y - last_y;
                    char trailer = (delta > 0) ? 'B' : 'A';
                    output += "\033[" + std::to_string(std::abs(delta)) + trailer;
                }
                last_x = x;
                last_y = y;

                // Whether or not the tile we are scanning through has colors
                // matching the previous tile we actually updated
                bool mismatch = first;
                if (last_tile != nullptr) {
                    mismatch |= last_tile->fore_color != next_state.fore_color;
                    mismatch |= last_tile->back_color != next_state.back_color;
                }
                first = false;

                // Update our prev_buffer to reflect the symbol that will be displayed
                prev_state = next_state;

                // If the colors don't match, add in the appropriate color escapes,
                // otherwise just print the symbol
                if(mismatch){
                    output += (std::string) tile_buffer[y*width+x];
                } else {
                    output += tile_buffer[y*width+x].raw_symbol();
                }

                // Remember the tile we most recently displayed
                last_tile = &next_state;
            }
        }
    }

    // restore the previously saved cursor position
    output += "\033[u";
    // Set the foreground and background colors back to their defaults, just in case
    output += "\033[39m\033[49m";
    std::cout << output;
    std::cout.flush();
}


// desc : Uses `full_display` to display the canvas if it hasn't been
//        fully displayed since construction or the most recent resize.
//        Otherwise, it uses `lazy_display` under the assumption that
//        it would produce correct behaviour.
// pre  : None
// post : None, aside from description
void Canvas::display() {
    if (should_full_display) {
        full_display();
    } else {
        lazy_display();
    }
}

// desc : Returns the canvas's width
// pre  : None
// post : None, aside from description
size_t Canvas::get_width() {
    return width;
}

// desc : Returns the canvas's height
// pre  : None
// post : None, aside from description
size_t Canvas::get_height() {
    return height;
}



// Raw mode logic adapted from the interesting tutorial at:
// https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
// which is published under CC BY 4.0 (https://creativecommons.org/licenses/by/4.0/)

// desc : Restores the state of the terminal from before it
//        was ever set to raw mode 
// pre  : None
// post : None, aside from description
void Input::cooked_mode() {
    if (!in_raw_mode) {
        return;
    }
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &Input::original_termios);
    in_raw_mode = false;
}

// desc : Function that is registered to handle exiting raw mode
//        in case of an interrupt
// pre  : None
// post : None, aside from description
void Input::last_meal(int signal) {
    if (!in_raw_mode) {
        return;
    }
    // Escape out of raw mode
    Input::cooked_mode();
    // Reset colors to their defaults
    std::cout << "\033[39m\033[49m";
    // Make sure the reset occurs before exit
    std::cout.flush();
    // Kill the program
    exit(1);
}


// desc : Causes the terminal to not echo bytes recieved from
//        standard input and to immediately relay bytes to the
//        foreground process (without waiting for the user to
//        press enter).
// pre  : None
// post : None, aside from description
void Input::raw_mode() {
    if (in_raw_mode) {
        return;
    }
    // Get the initial terminal config and save it
    tcgetattr(STDIN_FILENO, &Input::original_termios);
    in_raw_mode = true;
    // Tell the runtime to revert to cooked mode on exit
    atexit(cooked_mode);
    // Tell the runtime to revert to cooked mode and exit when
    // it encounters a SIGINT or SIGSEGV
    signal(SIGINT, Input::last_meal);
    signal(SIGSEGV,Input::last_meal);
    // Use initial config as a starting point for building up
    // our raw mode config
    termios raw_termios = Input::original_termios;
    raw_termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw_termios.c_oflag &= ~(OPOST);
    raw_termios.c_cflag |= (CS8);

    //// This line would remove the conversion of ctrl-C and ctrl-Z
    //// into signals. For safety, this is excluded.
    // raw_termios.c_lflag &= ~(ISIG);
    raw_termios.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_termios);
}

// Need to declare static members
termios Input::original_termios;
bool    Input::in_raw_mode = false;


// desc : Shifts all of the text in the box upwards by one row 
// pre  : None
// post : None, aside from description
void TextBox::scroll_down () {
    for (size_t y=0; y<(height-1); y++) {
        for (size_t x=0; x<(width-1); x++) {
            (*this)(x,y) = (*this)(x,y+1);
        }
    }
    for (size_t x=0; x<(width-1); x++) {
        (*this)(x,height-1) = RGB{0,0,0};
    }
}

// desc : Overwrites entire canvas with black space tiles
// pre  : None
// post : None, aside from description
void TextBox::clear () {
    for (size_t y=0; y<(height-1); y++) {
        for (size_t x=0; x<(width-1); x++) {
            (*this)(x,y) = RGB{0,0,0};
        }
    }
}

// desc : Initializes the textbox to the provided dimensions
//        at the provided absolute offset (relative to the
//        base cursor position).
// pre  : None
// post : None, aside from description
TextBox::TextBox(size_t width, size_t height, size_t x, size_t y)
    : Canvas(width,height,x,y)
    , cursor_x(0)
    , cursor_y(1)
{}

// pre/post/desc : Same as previous constructor, but with zero offset
TextBox::TextBox(size_t width, size_t height)
    : TextBox(width,height,0,0)
{}



