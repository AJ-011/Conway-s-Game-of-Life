#ifndef TUI
#define TUI

#include <iostream>
#include <string>
#include <sstream>
#include <atomic>
#include <sys/signal.h>
#include <termios.h>
#include <unistd.h>

namespace tui {


// Represents a 24-bit RGB color triplet
struct RGB{

    // Color components, as values between 0 and 255
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    // desc : Returns true if `other` represents an equivalent rgb
    // pre  : None
    // post : None, aside from desc
    bool operator ==(RGB const& other);
    
    // desc : Returns false if `other` represents an equivalent rgb
    // pre  : None
    // post : None, aside from desc
    bool operator !=(RGB const& other);
};




// Represents a unicode symbol, foreground color, and
// background color, for use in Canvases
struct Tile {
    // This is a string, rather than a character, to support unicode
    // rendering, since unicode characters are more than one char.
    // We could get fancy with unicode representation/conversion, but
    // a single string is sufficient for our purposes.
    std::string symbol;

    // The foreground color of the tile, as in the color of the symbol
    // in the tile.
    RGB fore_color;
    // The background color of the tile, as in the color that appears
    // behind the symbol in the tile.
    RGB back_color;

    // desc : Initializes tile as blank (black).
    // pre  : None
    // post : None, aside from desc
    Tile();
    
    // desc : Initializes tile as the provided symbol with the given
    //        foreground and background color.
    // pre  : None
    // post : None, aside from desc
    Tile(std::string symbol, RGB fore, RGB back);
    
    // desc : Initializes tile as a space character using the given 
    //        background color.
    // pre  : None
    // post : None, aside from desc
    Tile(RGB color);

    // desc : Return's the tile's string content, preceded by the escapes
    //        corresponding to its colors
    // pre  : None
    // post : None, aside from desc
    operator std::string();
    
    // desc : Simply returns the tile's string content
    // pre  : None
    // post : None, aside from desc
    std::string raw_symbol();
};



// Displays colored text within a bounded grid in the terminal connected
// to stdout
class Canvas {

    protected:
    // The dimensions of the canvas
    size_t width;
    size_t height;

    // The offset of the canvas, relative to the base cursor positon
    size_t offset_x;
    size_t offset_y;

    // A tile array representing the last image displayed
    Tile *prev_buffer;

    // A tile array representing the image that would be output the next
    // time a display occurs
    Tile *tile_buffer;

    // Tracks whether or not the `display` function should perform a
    // `full_display` call
    bool should_full_display;

    public:
    
    // desc : Initializes the canvas to the provided dimensions at the
    //        provided offset
    // pre  : None
    // post : None, aside from description
    Canvas(size_t width, size_t height, size_t x, size_t y);
    
    // desc/pre/post : Same as constructor above, but with zero offset
    Canvas(size_t width, size_t height);
    
    // desc : Frees the canvas's tile buffers
    // pre  : None
    // post : None, aside from description
    ~Canvas();

    // desc : Resizes the canvas to the provided dimensions
    // pre  : None
    // post : None, aside from description
    void resize(size_t width, size_t height);
    
    // desc : Repositions the canvas to the provided offset relative
    //        to the base cursor positon
    // pre  : None
    // post : None, aside from description
    void reposition(size_t x, size_t y);

    // desc : Returns a reference to the tile at the provided offset
    //        (relative to the top-left corner of the canvas)
    // pre  : The provided offset must correspond to a valid tile
    //        in the canvas's current bounds
    // post : None, aside from description
    Tile& operator()(size_t x, size_t y);

    // desc : Hides the canvas's content by overwriting it with
    //        default-colored space character tiles
    // pre  : None
    // post : None, aside from description
    void hide();

    // desc : Fully renders the canvas's content
    // pre  : None
    // post : None, aside from description
    void full_display();

    // desc : Updates a canvas's content, assuming it has previously been
    //        fully displayed and that only changed tiles need to update
    // pre  : None, aside from description
    // post : None, aside from description
    void lazy_display();
    
    // desc : Uses `full_display` to display the canvas if it hasn't been
    //        fully displayed since construction or the most recent resize.
    //        Otherwise, it uses `lazy_display` under the assumption that
    //        it would produce correct behaviour.
    // pre  : None
    // post : None, aside from description
    void display();

    // desc : Returns the canvas's width
    // pre  : None
    // post : None, aside from description
    size_t get_width();
    
    // desc : Returns the canvas's height
    // pre  : None
    // post : None, aside from description
    size_t get_height();
};




// Used to configure the way the program recieves input
class Input {

    // Stores the user's default settings, for later resoration
    static termios original_termios;
    // Tracks whether or not the terminal is currently in raw mode
    static bool    in_raw_mode;

    public:

    // desc : Restores the state of the terminal from before it
    //        was ever set to raw mode 
    // pre  : None
    // post : None, aside from description
    static void cooked_mode();

    // desc : Function that is registered to handle exiting raw mode
    //        in case of an interrupt
    // pre  : None
    // post : None, aside from description
    static void last_meal(int signal);

    // desc : Causes the terminal to not echo bytes recieved from
    //        standard input and to immediately relay bytes to the
    //        foreground process (without waiting for the user to
    //        press enter).
    // pre  : None
    // post : None, aside from description
    static void raw_mode();
};




// A convenience class that renders formatted output to a canvas
class TextBox : public Canvas {

    protected:

    // The stream used to format output
    std::stringstream stream;

    // Coordinates used to track the position of the virtual
    // cursor used by the textbox
    size_t cursor_x;
    size_t cursor_y;

    // desc : Shifts all of the text in the box upwards by one row 
    // pre  : None
    // post : None, aside from description
    void scroll_down ();

    public:

    // desc : Overwrites entire canvas with black space tiles
    // pre  : None
    // post : None, aside from description
    void clear ();

    // desc : Initializes the textbox to the provided dimensions
    //        at the provided absolute offset (relative to the
    //        base cursor position).
    // pre  : None
    // post : None, aside from description
    TextBox(size_t width, size_t height, size_t x, size_t y);

    // pre/post/desc : Same as previous constructor, but with zero offset
    TextBox(size_t width, size_t height);

    // desc : Renders formatted text to the canvas, using a
    //        std::stringstream to perform formatting.
    // pre  : None
    // post : Any 'normal' characters (as well as \t and \n) in the
    //        standard ASCII range (0-127) produced by formatting
    //        will be rendered to the screen. Unrecognized characters
    //        will be skipped.
    template<typename T>
    TextBox &operator<<(T &&arg) {
        // Ensure the stream is empty
        stream.clear();
        // Format arg via stream
        stream << arg;
        // Iterate through each char of formatted text in the stream
        char c;
        while (stream.read(&c,1)) {
            if (c=='\n') {
                cursor_y++;
                cursor_x = 0;
            } else if (c=='\t') {
                cursor_x += 4;
            } else if ((c>=' ')&&(c<='~')) {
                std::string str = {c,'\0'};
                (*this)(cursor_x,cursor_y) = Tile {
                    str,
                    {255,255,255},
                    {0,0,0}
                };
                cursor_x ++;
            }
            // Wrap text if full textbox width is exceeded
            if (cursor_x >= width) {
                cursor_x = 0;
                cursor_y ++;
            }
            // Scroll down if full textbox height is exceeded
            while (cursor_y >= height) {
                scroll_down();
                cursor_y--;
            }
        }
        return (*this);
    }

};


} // namespace tui


#endif //TUI

