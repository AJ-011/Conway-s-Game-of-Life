// p3.cpp
// Aditya Jain

// required headers:
#include "grid.h"
#include "tui.h"
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <condition_variable>
#include <memory>


// struct to keep track of game state:
struct ProgramState {
    int rule;            
    int frame_rate;
    int sim_rate;
    Grid *prev;
    Grid *next;
    tui::Canvas canvas;
    std::mutex mutex;
    bool running;
    bool paused;
    std::condition_variable cond;
};

// draw function that displays the game grid
void draw(ProgramState *state) {

    // keep drawing as long as simulation is running
    while (state->running) {
        {
            // use mutex to ensure safe access to prev
            std::lock_guard<std::mutex> lock(state->mutex);
            size_t x_limit = state->prev->get_width();
            size_t y_limit = state->prev->get_height();

            // iterate through all tiles and update
            for (size_t y = 0; y < y_limit; ++y) {
                for (size_t x = 0; x < x_limit; ++x) {
                    state->canvas(x * 2, y) = state->prev->get_tile(x, y) ? tui::RGB{255, 255, 255} : tui::RGB{0, 0, 0};
                    state->canvas(x * 2 + 1, y) = state->canvas(x * 2, y);
                }
            }
        }

        // display updated canvas and delay based on FPS
        state->canvas.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / state->frame_rate));
    }
}

// update function that updates state of grid
void update(ProgramState *state) {

    
    while (state->running) {
        {
            std::unique_lock<std::mutex> lock(state->mutex);
            // wait for notification from conditional variable to resume
            while (state->paused) {
                state->cond.wait(lock);
            }
        }

        size_t x_limit = state->prev->get_width();
        size_t y_limit = state->prev->get_height();
        std::vector<std::thread> threads;   // vector to store all individual threads

        // spawn threads updating each row of the grid
        for (size_t y = 0; y < y_limit; y++) {
            threads.emplace_back([&state, y, x_limit]() {
                for (size_t x = 0; x < x_limit; x++) {
                    state->next->update_tile(*state->prev, x, y, state->rule);
                }
            });
        }

        // ensure all threads finish
        for (auto &thread : threads) {
            thread.join();
        }

        {
            std::lock_guard<std::mutex> lock(state->mutex);
            std::swap(state->prev, state->next);    // swap current state with prev
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / state->sim_rate));
    }
}

// input function responsible for handling user inputs
void input_thread(ProgramState *state) {
    
    // enable raw mode
    tui::Input::raw_mode();
    char c;

    while (state->running && (read(0, &c, 1) == 1)) {
       
        // if c = q we quit the simulation
        if (c == 'q') {
            {
                std::lock_guard<std::mutex> lock(state->mutex);
                state->running = false;
            }
            state->cond.notify_all();
            break;
        }

        // if c = f or u or r we adjust certain parameters
        if (c == 'f' || c == 'u' || c == 'r') {
            {
                std::lock_guard<std::mutex> lock(state->mutex);
                state->paused = true;   // pause simulation to get user input
            }

            state->canvas.hide();
            tui::Input::cooked_mode();

            int val;
            while (true) {
                write(1, "Enter new value: ", 17);
                std::cin >> val;

                // handle if input fails or is invalid
                if (std::cin.fail() || val <= 0) {

                    if (std::cin.eof()){ 
                        break;  // ensure end of file
                    }

                    //clear faulty input and ask again
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    write(1, "Invalid input: Try again\n", 25);

                } else {
                    break;
                }
            }

            // use mutex to safely update state values
            {
                std::lock_guard<std::mutex> lock(state->mutex);
                if (c == 'f') state->frame_rate = val;
                if (c == 'u') state->sim_rate = val;
                if (c == 'r') state->rule = val;
                state->paused = false;
            }

            // notify waiting threads and display canvas again
            state->cond.notify_all();
            tui::Input::raw_mode();
            state->canvas.display();
        }
    }
    tui::Input::cooked_mode();
}

// main function
int main(int argc, char *argv[]) {

    // handle too many/no arguements
    if (argc != 2) {
        write(2, "Invalid argument number: Please only pass 1 argument\n", 53);
        return 1;
    }

    //open file
    std::string file_path = argv[1];
    std::ifstream file(file_path);

    // ensure file opens properly
    if (!file.is_open()) {
        write(2, "Error: Cannot open file\n", 24);
        return 1;
    }

    // Dynamically allocate grids to prevent memory leaks
    Grid* grid_a = new Grid(file_path);
    Grid* grid_b = new Grid(grid_a->get_width(), grid_a->get_height());

    // set current program state
    ProgramState state{
        .rule = 6152,
        .frame_rate = 1,
        .sim_rate = 1,
        .prev = grid_a,
        .next = grid_b,
        .canvas = tui::Canvas(grid_a->get_width() * 2, grid_a->get_height()),
        .running = true,
        .paused = false,
    };

    // start simulation threads
    std::thread dthread(draw, &state);
    std::thread uthread(update, &state);
    std::thread ithread(input_thread, &state);

    // ensure thread synchronization
    dthread.join();
    uthread.join();
    ithread.join();

    // hide canvas before exit
    state.canvas.hide();

    // Free allocated memory
    delete grid_a;
    delete grid_b;

    return 0;
}
