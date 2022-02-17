#include <SDL2/SDL.h>
#include "sdl-game.h"

struct game_window game;

int init_game_window(int width, int height, const char *title)
{
    int wflags = 0; // window flags
    int rflags = 0; // render flags

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return -1;

    game.window = SDL_CreateWindow(title, 0, 0, width, height, wflags);

    if (!game.window)
    {
        SDL_Quit();
        return -2;
    }

    game.renderer = SDL_CreateRenderer(game.window, -1, rflags);

    if (!game.renderer)
    {
        SDL_DestroyWindow(game.window);
        SDL_Quit();
        return -3;
    }

    // init data
    game.width = width;
    game.height = height;
    game.running = 1;
    game.pause = 0;
    game.delta_t = 0;
    game.max_fps = 60;
    game.fps = 0;
    game.keystate = SDL_GetKeyboardState(NULL);

    SDL_RenderPresent(game.renderer);
    on_game_creation();

    return 0;
}

void close_game_window()
{
    // clean up
    SDL_DestroyWindow(game.window);
    SDL_DestroyRenderer(game.renderer);
    SDL_Quit();

    game.window = NULL;
    game.renderer = NULL;
}

void handle_events()
{
    while (SDL_PollEvent(&(game.event)))
    {
        switch (game.event.type)
        {
            case SDL_QUIT:
                game.running = 0;
                break;
        };
    }
}

int start_game()
{
    unsigned int start, end;
    while (game.running)
    {
        start = SDL_GetTicks();

        handle_events();
        on_game_update();

        if (game.max_fps > 0 && game.max_fps < 1000.0f)
            SDL_Delay((1000.0f / game.max_fps) - (SDL_GetTicks() - start));

        end = SDL_GetTicks();

        game.delta_t = (float)(end - start) / 1000.0f;
        game.fps = 1 / game.delta_t;
    }

    return 0;
}
