#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdint.h>


#include "SDL.h"

typedef struct{
    SDL_Window *window;
    SDL_Renderer *renderer;
    
} sdl_t;


// Emulator configuration object
typedef struct{
 uint32_t window_width;   // SDL window width
 uint32_t window_height;  // SDL window height
 uint32_t fg_color;  // SDL window foreground color RGB
 uint32_t bg_color;  // SDL window background color RGB
 uint32_t scale_factor; // amount to scale a chip8 puxel by 20x larget window
} config_t;

// Emulator states
typedef enum{
   QUIT,
   RUNNING,
   PAUSED
}emulator_state_t;


// CHIP8 Machine objects
typedef struct{
    emulator_state_t state;
}chip8_t;



bool init_sdl(sdl_t *sdl, const config_t config){
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0){
        SDL_Log("Could not initialize %s\n",SDL_GetError());
        return false;
    }
sdl-> window = SDL_CreateWindow("Chip-8 Emulator", 
                                 SDL_WINDOWPOS_CENTERED,   
                                 SDL_WINDOWPOS_CENTERED, 
                                 config.window_width *config.scale_factor,
                                 config.window_height * config.scale_factor,
                                  0);


    if(!sdl->window){
        SDL_Log("Could not create SDL Window %s\n", SDL_GetError());
        return false;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
     if(!sdl->renderer){
        SDL_Log("Could not create SDL renderer %s\n", SDL_GetError());
        return false;
     }

    return true;

}

// set up initial emulator configuration from passed in arguments

bool set_config_from_args(config_t *config, const int argc, char **argv){
    *config = (config_t){
        .window_width = 64,
        .window_height = 32,
        .fg_color = 0XFFFFFFFF,
        .bg_color = 0XFFFF00FF,
        .scale_factor = 20 // default scale factor
    };

    for(int i = 1; i < argc; i++){
        (void)argv[i];
    }

    return true;
}


// Initialize chip8 machine
    bool init_chip8(chip8_t *chip8){
    chip8->state = RUNNING;    // default machine running state
       return true;  // Success  
    }


void final_cleanup(const sdl_t *sdl){
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

// Clear Screen / SDL window to background color
void clear_screen(const sdl_t sdl, config_t config){

    const uint8_t r = (config.bg_color >> 24) & 0XFF;
    const uint8_t g = (config.bg_color >> 16) & 0XFF;
    const uint8_t b = (config.bg_color >> 8) & 0XFF;
    const uint8_t a = (config.bg_color >> 0) & 0XFF;

SDL_SetRenderDrawColor(sdl.renderer,r, g, b, a);
    SDL_RenderClear(sdl.renderer);
}


// update of window of any changes
void update_screen(const sdl_t sdl){
    SDL_RenderPresent(sdl.renderer);
}




void handle_input(chip8_t *chip8){
  // 

  SDL_Event event;
  while(SDL_PollEvent(&event)){
    switch (event.type)
    {
    case SDL_QUIT:
        /* code */

        chip8->state = QUIT; // will exit main 
       return;
    case SDL_KEYDOWN:

    switch(event.key.keysym.sym){
        case SDLK_ESCAPE: 

           chip8->state = QUIT;
           return;  
           
           
           default:
                 break;              // exit the window
    }

    break;
    case SDL_KEYUP:
         break;

    default:
    break;

         
    }
  }
}


// Main function

int main(int argc, char **argv){
    (void) argc;
    (void) argv;

     //Initialize emulator config options
    config_t config  = {0};
    if(!set_config_from_args(&config,argc,argv)) exit(EXIT_FAILURE);

    // Initialize the SDL
     sdl_t sdl = {0};
    if(!init_sdl(&sdl, config)) exit(EXIT_FAILURE);

    // Initialize CHIP8 machine

    chip8_t chip8 = {0};
    if(!init_chip8(&chip8)) exit(EXIT_FAILURE);




    // initial screen clear
    clear_screen(sdl, config);

    //MAIN EMULATOR LOOP
    while(chip8.state == ! QUIT){

        // handle user input
        handle_input(&chip8);
      

        // Emulate chip8 instrucntion here
        //  Delay
        // Update window on changes 
        SDL_Delay(16);
        update_screen(sdl);
    }

    // Final Cleanup function
    final_cleanup(&sdl);
    exit(EXIT_SUCCESS);
}
