ref "std/shell"
ref "../../build/pinfo"

config = argv[0]

{
    name: "sdl"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "Simple DirectMedia Layer - Game development library"
        zh: "Simple DirectMedia Layer - 游戏开发库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "libSDL")
    development_dependencies: get_deps("ox_devel", "pb", "libsdl_devel")
    libraries: [
        "sdl"
    ]
    oxngen_targets: {
        sdl: {
            cflags: Shell.output("{config.pc} sdl --cflags").trim()
            libs: Shell.output("{config.pc} sdl --libs").trim()
            input_files: [
                "SDL.h"
            ]
            file_filters: [
                "SDL.h"
                "SDL_video.h"
                "SDL_keyboard.h"
                "SDL_keysym.h"
                "SDL_mouse.h"
                "SDL_events.h"
                "SDL_timer.h"
                "SDL_version.h"
            ]
            decl_filters: [
                /SDL.+/p
            ]
            rev_decl_filters: [
                "SDL_LoadBMP_RW"
                "SDL_SaveBMP_RW"
                "SDL_CreateRGBSurface"
                "SDL_UpperBlit"
                "SDL_LowerBlit"
            ]
            functions: {
                SDL_AllocSurface: {
                    parameters: {
                        flags: "Uint32"
                        width: "int"
                        height: "int"
                        depth: "int"
                        Rmask: "Uint32"
                        Gmask: "Uint32"
                        Bmask: "Uint32"
                        Amask: "Uint32"
                    }
                    return: "SDL_Surface *"
                }
                SDL_BlitSurface: {
                    parameters: {
                        src: "SDL_Surface *"
                        srcrect: "SDL_Rect *"
                        dst: "SDL_Surface *"
                        dstrect: "SDL_Rect *"
                    }
                    return: "int"
                }
                SDL_LoadBMP: {
                    parameters: {
                        file: "const char *"
                    }
                    return: "SDL_Surface *"
                }
                SDL_SaveBMP: {
                    parameters: {
                        surface: "SDL_Surface *"
                        file: "const char *"
                    }
                    return: "int"
                }
            }
        }
    }
}
