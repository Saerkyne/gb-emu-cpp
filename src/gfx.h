#pragma once

#include <SDL3/SDL.h>

struct gfx_context
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;
	SDL_Texture* texture;
};