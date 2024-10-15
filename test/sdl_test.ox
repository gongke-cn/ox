ref "sdl"
ref "std/log"
ref "std/math"
ref "std/rand"
ref "json"

log: Log("sdl")

SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTTHREAD)

s = SDL_SetVideoMode(640, 480, 32, SDL_DOUBLEBUF)
e = SDL_Event()

/*timer = SDL_AddTimer(100, global func (interval) {
	log.debug("timer")
	return interval
})
*/
SDL_WM_SetCaption("test")

x1 = 0
y1 = 0
x2 = 0
y2 = 0
rect = SDL_Rect()

while true {
	sched {
		r = SDL_WaitEvent(e)
	}
	if r {
		log.debug(JSON.to_str(e))
		case e.type {
		SDL_KEYDOWN {
			case e.key.keysym.sym {
			SDLK_f {
				log.debug("toggle full screen")
				SDL_WM_ToggleFullScreen(s)
				SDL_Flip(s)
			}
			SDLK_s {
				log.debug("save screen")
				SDL_SaveBMP(s, "screen.bmp")
			}
			SDLK_q {
				log.debug("quit")
				break
			}
			}
		}
		SDL_MOUSEBUTTONDOWN {
			log.debug("button down {e.button.button}@{e.button.x},{e.button.y}")
			x1 = e.button.x
			y1 = e.button.y
		}
		SDL_MOUSEBUTTONUP {
			log.debug("button up {e.button.button}@{e.button.x},{e.button.y}")
			x2 = e.button.x
			y2 = e.button.y

			rect.{
				x: min(x1, x2)
				y: min(y1, y2)
				w: abs(x1 - x2) + 1
				h: abs(y1 - y2) + 1
			}

			color = SDL_MapRGB(s.format, rand()%256, rand()%256, rand()%256)
			SDL_FillRect(s, rect, color)
			SDL_Flip(s)
		}
		SDL_QUIT {
			log.debug("quit")
			break
		}
		}
	}
}

SDL_Quit()
