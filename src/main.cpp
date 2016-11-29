/* Main executable and game loop. */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>
#include "Area.h"
#include "CellData.h"

#define COLOR_SELECT   	0x00, 0x50, 0x50, 0x40
#define COLOR_GRID     	0x00, 0x00, 0x00, 0xFF 
#define COLOR_BAD_CELL 	0xFF, 0xB8, 0xCF, 0xFF

struct Texture {
	SDL_Texture* texture;
	int width;
	int height;
};

int loadTexture (const std::string &file, Texture &texture);

SDL_Surface* loadSurface (const std::string &file);

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int CELL_SIZE = 40;

//The window we'll be rendering to
SDL_Window* window = NULL;

//The surface contained by the window
SDL_Surface* gSurface = NULL;

SDL_Renderer* gRenderer = NULL;

SDL_Texture* gTexture = NULL;

SDL_Surface* icon = NULL;

Texture bgTxtr;
Texture digTxtrs [9] ;

int sel_x = 0;
int sel_y = 0;

CellData data [9][9];

Area area_rows [9];
Area area_cols [9];
Area area_boxs [9];

int init ()
{
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return -1;
	}
	window = SDL_CreateWindow( 
			"SDL Tutorial", 
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, 
			SCREEN_HEIGHT, 
			SDL_WINDOW_SHOWN );
	if ( window == NULL )
	{
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		return -1;
	}
	
	gSurface = SDL_GetWindowSurface( window );
	if ( gSurface == NULL )
	{
		printf( "Surface could not be created! SDL_Error: %s\n", SDL_GetError() );
		return -1;
	}

	gRenderer = SDL_GetRenderer (window);
	if ( gRenderer == NULL )
	{
		printf( "Renderer could not be created! SDL_Error: %s\n", SDL_GetError() );
		return -1;
	}
	SDL_SetRenderDrawBlendMode (gRenderer, SDL_BLENDMODE_BLEND);
	
	gTexture = SDL_CreateTexture(gRenderer, gSurface->format->format,
		SDL_TEXTUREACCESS_TARGET, 9*CELL_SIZE+2, 9*CELL_SIZE+2); // might be 1 too little
	if ( gTexture == NULL )
	{
		printf( "Texture could not be created! SDL_Error: %s\n", SDL_GetError() );
		return -1;
	}
	
	if ( TTF_Init () < 0 )
	{
		printf( "TTF could not initialize! TTF_Error: %s\n", TTF_GetError() );
		return -1;
	}
	
	SDL_Color color = { 0x00, 0x00, 0x00 };
	TTF_Font* font = TTF_OpenFont( "res/lazy.ttf", 40 );
	if ( font != NULL)
	{
		for (int i = 0; i < 9; i++)
		{
			std::string s = std::to_string(i+1);
			SDL_Surface* textSurface = TTF_RenderText_Blended( font, s.c_str(), color);
			
			if( textSurface == NULL )
			{
				printf( "Unable to render text surface! SDL_ttf error: %s\n", TTF_GetError() );
			}
			else
			{
				//Create texture from surface pixels
				SDL_Texture * texture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
				if( texture == NULL )
				{
					printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
				}
				else
				{
					Texture ft;
					ft.texture = texture;
					ft.width = textSurface->w;
					ft.height = textSurface->h;
					digTxtrs[i] = ft;
				}

				//Get rid of old surface
				SDL_FreeSurface( textSurface );
			}
		}
		TTF_CloseFont (font);
	}
	else 
	{
		printf ("Could not load font! SDL error: %s\n", TTF_GetError());
	}
	
	int imgFlags = IMG_INIT_PNG;
	if( !( IMG_Init( imgFlags ) & imgFlags ) )
	{
		printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
		return -1;
	}
	
	if (loadTexture ("res/dark-tile.gif", bgTxtr) < 0)
	{
		printf("FAIL\n");
		return -1;
	}
	
	icon = loadSurface ("res/sudoku_icon.png");
	SDL_SetWindowIcon (window, icon);	
	SDL_SetWindowTitle (window, "Sudoku in C++ and SDL");
	
	for (int i = 0; i < 9; i++)
	{
		area_cols[i].init(AREA_COLUMN, i);
	}
	for (int i = 0; i < 9; i++)
	{
		area_rows[i].init(AREA_ROW, i);
	}
	for (int i = 0; i < 9; i++)
	{
		area_boxs[i].init(AREA_BOX, i);
	}
	
	return 0;
}

int loadTexture (const std::string &file, Texture &texture)
{	
	SDL_Surface* image = loadSurface(file);
	if (image == NULL)
	{
		printf("Can't load %s! SDL error: %s\n", file.c_str(), IMG_GetError());
		return -1;
	}
	
	SDL_Texture *bg = SDL_CreateTextureFromSurface (gRenderer, image);
	texture.width = image->w;
	texture.height = image->h;
	texture.texture = bg;
	SDL_FreeSurface (image);
}

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

SDL_Surface* loadSurface (const std::string &file)
{
	SDL_RWops *rwop = SDL_RWFromFile(file.c_str(), "rb");
	if (rwop == NULL)
	{
		printf("Can't find %s!\n", file.c_str());
		return NULL;
	}
	
	SDL_Surface *image;
	std::string sfx_gif = ".gif";
	std::string sfx_png = ".png";
	if (hasEnding(file, sfx_gif))
        image = IMG_LoadGIF_RW (rwop);
    else if (hasEnding(file, sfx_png) )
		image = IMG_Load ( file.c_str() );    
	 
	if (image == NULL)
	{
		printf("Failed to load %s: SDL error: %s\n", file.c_str(), SDL_GetError());
	}
	return image;
}

void drawBackground( )
{
	for (int x = 0 ; x < SCREEN_WIDTH; x += bgTxtr.width)
	{
		for (int y = 0 ; y < SCREEN_HEIGHT; y += bgTxtr.height)
		{
			SDL_Rect rect = { x, y, bgTxtr.width, bgTxtr.height };
			SDL_RenderCopy (gRenderer, bgTxtr.texture, NULL, &rect);
		}
	}
}

void drawGrid ()
{
	int fullsize = 9 * CELL_SIZE + 1;
	
	SDL_SetRenderDrawColor ( gRenderer, COLOR_GRID );		
	for (int i = 0; i < 10; i++){
		int x = i * CELL_SIZE;
		int y = i * CELL_SIZE;
		SDL_RenderDrawLine (gRenderer, 0, y, fullsize, y);
		SDL_RenderDrawLine (gRenderer, x, 0, x, fullsize);
		if (i % 3 == 0)
		{
			SDL_RenderDrawLine (gRenderer, 0, y+1, fullsize, y+1);
			SDL_RenderDrawLine (gRenderer, x+1, 0, x+1, fullsize);
		}
	}
}

void drawSelection ()
{	
	int x = sel_x * CELL_SIZE;
	int y = sel_y * CELL_SIZE;
	
	SDL_SetRenderDrawColor ( gRenderer, COLOR_SELECT );
	
	int startx = sel_x % 3 == 0 ? x + 2 : x +1;
	int width = sel_x % 3 == 0 ? CELL_SIZE - 2 : CELL_SIZE - 1;
	int starty = sel_y % 3 == 0 ? y + 2 : y + 1;
	int height = sel_y % 3 == 0 ? CELL_SIZE - 2 : CELL_SIZE - 1;
	
	SDL_Rect select = { startx, starty, width, height };
	SDL_RenderFillRect (gRenderer, &select);
}

void drawInvalidCells ()
{
	SDL_SetRenderDrawColor ( gRenderer, COLOR_BAD_CELL );
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (data[i][j].vldcnt != 0)
			{
				int x = i * CELL_SIZE;
				int y = j * CELL_SIZE;
	
				int startx = i % 3 == 0 ? x + 2 : x + 1;
				int width = i % 3 == 0 ? CELL_SIZE - 2 : CELL_SIZE - 1;
				int starty = j % 3 == 0 ? y + 2 : y + 1;
				int height = j % 3 == 0 ? CELL_SIZE - 2 : CELL_SIZE - 1;
	
				SDL_Rect select = { startx, starty, width, height };
				SDL_RenderFillRect (gRenderer, &select);
			}	
		}
	}
}

void drawText ()
{
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (data[i][j].digit != 0)
			{
				Texture ft = digTxtrs[data[i][j].digit - 1];				
				
				int x = i * CELL_SIZE;
				int y = j * CELL_SIZE;
	
				int startx = (i % 3 == 0 ? x + 2 : x +1) + (CELL_SIZE - ft.width) / 2;
				int starty = (j % 3 == 0 ? y + 2 : y + 1) + (CELL_SIZE - ft.height) / 2;
				
				SDL_Rect renderQuad = { startx, starty, ft.width, ft.height };
				
				//Render to screen
				SDL_RenderCopy( gRenderer, ft.texture, NULL, &renderQuad );
			}	
		}
	}
}

void close ()
{
	if (icon != NULL)
		SDL_FreeSurface(icon);
	if (gRenderer != NULL)
		SDL_DestroyRenderer(gRenderer);
	if (gSurface != NULL)
		SDL_FreeSurface(gSurface);
	if (window != NULL)
		SDL_DestroyWindow(window);
	if (gTexture != NULL)
		SDL_DestroyTexture(gTexture);
	if (bgTxtr.texture != NULL)
		SDL_DestroyTexture(bgTxtr.texture);
	
	for (int i = 0; i < 9; i++)
		SDL_DestroyTexture(digTxtrs[i].texture);
	
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void handleKey (SDL_Event &event)
{
	int sym = event.key.keysym.sym;
	switch( sym )
	{
		// movement
		case SDLK_UP: 
		{
			sel_y--;
			if (sel_y < 0) sel_y = 8;
			break;
		}
		case SDLK_DOWN: 
		{
			sel_y++;
			if (sel_y > 8)	sel_y = 0;		
			break;
		}
		case SDLK_LEFT:
		{
			sel_x--; 
			if (sel_x < 0) sel_x = 8;
			break;
		}
		case SDLK_RIGHT:
		{
			sel_x++;
			if (sel_x > 8)	sel_x = 0;
			break;
		}
		case SDLK_KP_1: case SDLK_KP_2: case SDLK_KP_3:
		case SDLK_KP_4: case SDLK_KP_5: case SDLK_KP_6:
		case SDLK_KP_7:	case SDLK_KP_8:	case SDLK_KP_9:
		{
			int prev = data [sel_x][sel_y].digit;
			int nr = sym - SDLK_KP_1 + 1;
			if (prev == nr)
				break;
			
			int box = 3 * (sel_y / 3) + (sel_x / 3);
			
			if (prev != 0)
			{
				data [sel_x][sel_y].digit = 0;
				area_cols[sel_x].DigitRemoved (prev, data, sel_x, sel_y);
				area_rows[sel_y].DigitRemoved (prev, data, sel_x, sel_y);
				area_boxs[box].DigitRemoved (prev, data, sel_x, sel_y);
			}
			
			data [sel_x][sel_y].digit = nr;
			area_cols[sel_x].DigitAdded (nr, data, sel_x, sel_y);
			area_rows[sel_y].DigitAdded (nr, data, sel_x, sel_y);
			area_boxs[box].DigitAdded (nr, data, sel_x, sel_y);
			break;
		}
		case SDLK_BACKSPACE:
		{
			if (data [sel_x][sel_y].digit == 0)
				break;
			
			int nr = data [sel_x][sel_y].digit;
			data [sel_x][sel_y].digit = 0;
			
			int box = 3 * (sel_y / 3) + (sel_x / 3);
			
			area_cols[sel_x].DigitRemoved (nr, data, sel_x, sel_y);
			area_rows[sel_y].DigitRemoved (nr, data, sel_x, sel_y);
			area_boxs[box].DigitRemoved (nr, data, sel_x, sel_y);
			break;
		}
	}		
}

int main( int argc, char* args[] )
{
	if (init () < 0)
		return -1;

	bool quit = false;
	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			//User requests quit
			if( event.type == SDL_QUIT )
			{
				quit = true;
			} 
			else if ( event.type == SDL_KEYDOWN )
			{
				handleKey ( event );
			}
		}
		
		//Fill the surface white
		SDL_SetRenderDrawColor ( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
		SDL_RenderClear(gRenderer);
		drawBackground ();
		
		SDL_SetRenderTarget(gRenderer, gTexture);
		SDL_RenderClear(gRenderer);
		
		drawGrid();
		drawInvalidCells();
		drawSelection ();
		drawText ();
		SDL_SetRenderTarget(gRenderer, NULL);
		
		int size = 9 * CELL_SIZE + 2;
		SDL_Rect dst = { ( SCREEN_WIDTH - size ) / 2, ( SCREEN_HEIGHT - size ) / 2, size, size };
		SDL_RenderCopy(gRenderer, gTexture, NULL, &dst);
		
		// draw everything
		SDL_RenderPresent(gRenderer);
		
		SDL_Delay (16.6);
	}
	close();

	//Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();

	
	return 0;
}