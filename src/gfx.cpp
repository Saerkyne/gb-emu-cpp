#include "gfx.h"
#include <stdlib.h>
#include <stdio.h>

gfx_context* gfx_init(unsigned int width, unsigned int height, const char* windowTitle)
{
	gfx_context* ctx = (gfx_context*)malloc(sizeof(gfx_context));

	if (!ctx)
	{
		return NULL;
	}

	//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
	if (!SDL_CreateWindowAndRenderer(windowTitle, width, height, 0, &ctx->window, &ctx->renderer))
	{
		printf("GFX Unable to initialize Window or Renderer\n");
		gfx_destroy(ctx);
		return NULL;
	}

	ctx->texture = SDL_CreateTexture(ctx->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_SetTextureScaleMode(ctx->texture, SDL_SCALEMODE_NEAREST);
	if (!ctx->texture)
	{
		printf("GFX Unable to create texture\n");
		gfx_destroy(ctx);
		return NULL;
	}
	//SDL_SetRenderVSync(ctx->renderer, 1);

	return ctx;
}

void gfx_clear(gfx_context* ctx, SDL_Color color)
{
	SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(ctx->renderer);
}

void gfx_putpixel(gfx_context* ctx, unsigned int x, unsigned int y, SDL_Color color)
{
	SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
	SDL_RenderPoint(ctx->renderer, x, y);
}

void gfx_update(gfx_context* ctx)
{
	SDL_RenderPresent(ctx->renderer);
}

void gfx_destroy(gfx_context* ctx)
{
	SDL_DestroyRenderer(ctx->renderer);
	SDL_DestroyWindow(ctx->window);
	SDL_DestroyTexture(ctx->texture);
	free(ctx);
}