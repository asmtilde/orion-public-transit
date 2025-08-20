#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// --- Configuration ---
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 960;
constexpr int TILE_SIZE = 32;
constexpr int FPS = 60;
constexpr const char* GAME_NAME = "Orion Public Transit";
constexpr int AUDIO_SAMPLE_RATE = 44100;

// --- Core Classes ---

class App {
public:
    App(const std::string& title) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

        window = SDL_CreateWindow(title.c_str(),
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window)
            throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer)
            throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    }

    ~App() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    SDL_Renderer* getRenderer() const { return renderer; }

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};

class AudioManager {
public:
    AudioManager() {
        if (Mix_OpenAudio(AUDIO_SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
            throw std::runtime_error(std::string("Mix_OpenAudio failed: ") + Mix_GetError());
    }

    ~AudioManager() {
        stopMusic();
        Mix_CloseAudio();
    }

    void playMusic(const std::string& file, int loops = -1) {
        stopMusic();
        music = Mix_LoadMUS(file.c_str());
        if (!music)
            throw std::runtime_error(std::string("Failed to load music: ") + Mix_GetError());

        if (Mix_PlayMusic(music, loops) == -1)
            throw std::runtime_error(std::string("Failed to play music: ") + Mix_GetError());
    }

    void stopMusic() {
        if (music) {
            Mix_HaltMusic();
            Mix_FreeMusic(music);
            music = nullptr;
        }
    }

private:
    Mix_Music* music = nullptr;
};

class Player {
public:
    Player(int px, int py, int pw, int ph, int pspeed)
        : x(px), y(py), w(pw), h(ph), speed(pspeed) {}

    void handleInput(const Uint8* keystate) {
        velx = vely = 0;
        if (keystate[SDL_SCANCODE_W]) vely = -speed;
        if (keystate[SDL_SCANCODE_S]) vely = speed;
        if (keystate[SDL_SCANCODE_A]) velx = -speed;
        if (keystate[SDL_SCANCODE_D]) velx = speed;
    }

    void update() {
        x += velx;
        y += vely;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x + w > WINDOW_WIDTH) x = WINDOW_WIDTH - w;
        if (y + h > WINDOW_HEIGHT) y = WINDOW_HEIGHT - h;
    }

    void render(SDL_Renderer* renderer) const {
        SDL_Rect rect{x, y, w, h};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

private:
    int x{}, y{};
    int w{}, h{};
    int speed{};
    int velx{}, vely{};
};

namespace RendererUtils {
    void drawGrid(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        for (int x = 0; x < WINDOW_WIDTH; x += TILE_SIZE)
            SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
        for (int y = 0; y < WINDOW_HEIGHT; y += TILE_SIZE)
            SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);
    }
}

class Game {
public:
    Game() : app(GAME_NAME), player(0, 0, TILE_SIZE, TILE_SIZE, 2), running(true) {}

    void run() {
        SDL_Event event;
        const Uint8* keystate = SDL_GetKeyboardState(nullptr);

        while (running) {
            handleEvents(event);
            player.handleInput(keystate);
            player.update();

            renderFrame();

            SDL_Delay(1000 / FPS);
        }
    }

private:
    void handleEvents(SDL_Event& event) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }
    }

    void renderFrame() {
        auto renderer = app.getRenderer();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        RendererUtils::drawGrid(renderer);
        player.render(renderer);

        SDL_RenderPresent(renderer);
    }

private:
    App app;
    AudioManager audio;
    Player player;
    bool running;
};

// --- MAIN ---
int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
