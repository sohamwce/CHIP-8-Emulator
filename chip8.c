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


//typedef struct 
typedef struct{
   uint16_t opcode; 
   uint16_t NNN;
    uint8_t  NN;
    uint8_t  N;
    uint8_t  X;
    uint8_t  Y;

}instrunction_t;


// CHIP8 Machine objects
typedef struct{
    emulator_state_t state;
    uint8_t ram[4096]; 
    uint8_t display[64 * 32]; // CHIP8 display 64x32 pixels
    uint16_t stack[12];       // subroutine stack 
    uint8_t V[16];         // data regiesters V0 to VF
    uint16_t I;            // index register
    uint16_t PC;           // program counter
    uint8_t delay_timer;   // decrements at 60Hz
    uint8_t sound_timer;   // decrements at 60Hz and plays tone when > 0 
    bool keypad[16];    // hex keypad state
    const char *rom_name; // loaded rom name
    instrunction_t inst;
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
        .bg_color = 0X00000000,
        .scale_factor = 20 // default scale factor
    };

    for(int i = 1; i < argc; i++){
        (void)argv[i];
    }

    return true;
}


// Initialize chip8 machine
    bool init_chip8(chip8_t *chip8, const char rom_name[]){
      const uint32_t entry_point = 0x200;  
      const uint8_t font[] =
{       0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0   
        0x20, 0x60, 0x20, 0x20, 0x70,   // 1  
        0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2 
        0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,   // 4    
        0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
        0xF0, 0x80, 0xF0, 0x80, 0x80,   // F
}; // default machine running state    

    // load font 
    memcpy(&chip8->ram[0], font, sizeof(font));
      
      // load ROM to chip8 memory
      FILE *rom = fopen(rom_name, "rb"); 
      if(!rom)
     {
            SDL_Log("Could not open ROM file %s\n", rom_name);
            return false;   
     }

     fseek(rom,0,SEEK_END);
     const size_t rom_size = ftell(rom);
     const size_t max_size = sizeof chip8->ram - entry_point;
     rewind (rom);



     if(rom_size > max_size){
        SDL_Log("Rom file %s is too big! Rom size: %zu bytes, max size: %zu bytes\n",
                 rom_name, rom_size, max_size);
                 return false;
     }

     if(fread(&chip8->ram[entry_point],rom_size, 1, rom) != 1){
        SDL_Log("Could not read ROM file %s into chip8 memory \n",
                rom_name);
            return false;
     }

     fclose(rom);
      // set chip8 

      chip8->state = RUNNING; 
      chip8->PC = entry_point;
      chip8->rom_name = rom_name;


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
           
        case SDLK_SPACE:

               if(chip8->state  == RUNNING)
                  chip8->state = PAUSED;
               else {
                  chip8->state = RUNNING;
                  puts("==== PAUSED ====");
               }
               break;
           
           
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


// Emulate chip8 instrunction
void emulate_instrunction(chip8_t *chip8){
      chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8-> ram[chip8->PC+1];
}



// Main function

int main(int argc, char **argv){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <ROM file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

     //Initialize emulator config options
    config_t config  = {0};
    if(!set_config_from_args(&config,argc,argv)) exit(EXIT_FAILURE);

    // Initialize the SDL
     sdl_t sdl = {0};
    if(!init_sdl(&sdl, config)) exit(EXIT_FAILURE);

    // Initialize CHIP8 machine

    chip8_t chip8 = {0};
    const char *rom_name = argv[1];
    if(!init_chip8(&chip8, rom_name)) exit(EXIT_FAILURE);




    // initial screen clear
    clear_screen(sdl, config);

    //MAIN EMULATOR LOOP
    while(chip8.state == ! QUIT){

        // handle user input
        handle_input(&chip8);

        if(chip8.state == PAUSED) continue;
      

        // Emulate chip8 instrucntion here

        emulate_instrunction(&chip8);
        //  Delay
        // Update window on changes 
        SDL_Delay(16);
        update_screen(sdl);
    }

    // Final Cleanup function
    final_cleanup(&sdl);
    exit(EXIT_SUCCESS);
}
