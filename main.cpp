#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<fstream>
//#include<dirent.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}


#define SCREEN_WIDTH	930
#define SCREEN_HEIGHT	720
#define PIC_HEIGHT      2825
#define COORDS          SCREEN_HEIGHT - PIC_HEIGHT / 2
#define ROZMIAR_PRZESZKODY 125			

#define LEFT_BORDER     296
#define RIGHT_BORDER    626
#define RIGHT_SLOW      576
#define LEFT_SLOW       346

#define OBSTACLE_WIDTH	125
#define RANDOM_POS      100

#define CAR_WIDTH		92
#define CAR_HEIGHT		164

#define WSPOLRZEDNA_SPRITEA_X SCREEN_WIDTH / 2
#define WSPOLRZEDNA_SPRITEA_Y SCREEN_HEIGHT / 2

struct obstacle {
	int X;
	int Y;
};

typedef struct obstacle obstacleType;

// narysowanie napisu txt na powierzchni screen, zaczynaj�c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj�ca znaki
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt �rodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d�ugo�ci l w pionie (gdy dx = 0, dy = 1) 
// b�d� poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostok�ta o d�ugo�ci bok�w l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};


// nowa gra
void resetGame(double* worldTime, double* score, double* etiSpeed, double* distance2, double* distance, int* newGame, int* move) {
	*worldTime = 0;
	*score = 0;
	*etiSpeed = 0;
	*distance2 = 0;
	*distance = 0;
	*newGame = 0;
	*move = 0;
}


// obliczenie wyniku
void countPoints(double* etiSpeed, double* score, double* delta, bool* pause, int move) {

	// za wolny ruch dac 150 pkt/sekunde
	if (*etiSpeed <= 3000 && *etiSpeed > 0) {
		if (*pause != true) { *score += 150 * (*delta); }
		else *score += 150 * !pause;
	}
	// za szybki ruch dac 250 pkt/sekunde
	else if (*etiSpeed > 3000) {
		//*score += 250 * (*delta);
		if (*pause != true) { *score += 250 * (*delta); }
		else *score += 250 * !pause;
	}

}


// rysowanie mapy
void drawMap(double& distance2, SDL_Surface* screen, SDL_Surface* background, bool& spawn) {

	SDL_Surface* obstacle = SDL_LoadBMP("./obstacle.bmp");
	if (COORDS + (distance2) >= COORDS + PIC_HEIGHT) {
		distance2 = 0.0;
		spawn = false;
	}
	DrawSurface(screen, background, SCREEN_WIDTH / 2, COORDS + (distance2));
	DrawSurface(screen, background, SCREEN_WIDTH / 2, COORDS - PIC_HEIGHT + (distance2));

}


// rysowanie przeszkod
void drawObstacles(SDL_Surface* screen, SDL_Surface* obstacle, double distance2, int* randomX) {

	DrawSurface(screen, obstacle, *randomX, COORDS + (distance2));

}


// rysowanie spritea
void drawSprite(SDL_Surface* screen, SDL_Surface* eti, int* move) {
	DrawSurface(screen, eti, SCREEN_WIDTH / 2 + (*move), SCREEN_HEIGHT / 2);
}


// obliczenie FPS
void countFPS(double* fpsTimer, double* delta, double* fps, int* frames) {

	*fpsTimer += (*delta);
	if ((*fpsTimer) > 0.5) {
		*fps = (*frames) * 2;
		*frames = 0;
		*fpsTimer -= 0.5;
	}

}


void drawData(SDL_Surface* screen, int* zielony, int* czarny, char* text, double* worldTime, double* fps, SDL_Surface* charset, double* score) {

	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, *zielony, *czarny);

	sprintf(text, "Spy Hunter | Taras Shuliakevych | 196615 | Czas trwania gry: %.1lf s |  %.0lf klatek / s", *worldTime, *fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

	sprintf(text, "wynik: %d", (int)(*score));
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

}


void drawTutorial(SDL_Surface* screen, int* zielony, int* czarny, char* text, double* worldTime, double* fps, SDL_Surface* charset, double* score) {

	DrawRectangle(screen, 530, 645, 384, 40, *zielony, *czarny);

	sprintf(text, "strzalki: poruszanie graczem na planszy"); // 39 - 624 - 240
	DrawString(screen, screen->w - (strlen(text) * 16 - 240), 650, text, charset);

	sprintf(text, "esc: zamkniecie programu"); // 24 - 384
	DrawString(screen, screen->w - strlen(text) * 16, 660, text, charset);

	sprintf(text, "n: rozpoczecie nowej gry");
	DrawString(screen, screen->w - strlen(text) * 16, 670, text, charset);


}
// rysowanie tekstu informacyjnego
void drawInfo(SDL_Surface* screen, int* zielony, int* czarny, char* text, SDL_Surface* charset, double* score, double* worldTime, double* fps) {

	drawData(screen, zielony, czarny, text, worldTime, fps, charset, score);
	drawTutorial(screen, zielony, czarny, text, worldTime, fps, charset, score);

}


// zapisywanie gry
void saveGame(double* worldTime, double* score, bool* load1) {

	if (*load1 == false) *load1 = true;

	// czas, wynik
	double gameTime = *worldTime, result = *score;
	//char name = "";

	char buf[0x100];

	/*time_t currentTime;
	char* currentTimeInString;

	currentTime = time(NULL);

	currentTimeInString = ctime(&currentTime);

	printf("time: %s", currentTimeInString);
	*/


	int hours, minutes, seconds, day, month, year;

	// `time_t` is an arithmetic time type
	time_t now;

	// Obtain current time
	// `time()` returns the current time of the system as a `time_t` value
	time(&now);

	// Convert to local time format and print to stdout
	printf("Today is %s", ctime(&now));

	// localtime converts a `time_t` value to calendar time and
	// returns a pointer to a `tm` structure with its members
	// filled with the corresponding values
	struct tm* local = localtime(&now);

	hours = local->tm_hour;         // get hours since midnight (0-23)
	minutes = local->tm_min;        // get minutes passed after the hour (0-59)
	seconds = local->tm_sec;        // get seconds passed after a minute (0-59)

	day = local->tm_mday;            // get day of month (1 to 31)
	month = local->tm_mon + 1;      // get month of year (0 to 11)
	year = local->tm_year + 1900;


	FILE* fp;

	snprintf(buf, sizeof(buf), "./saves/%d.%d.%d__%d-%d-%d.txt", day, month, year, hours, minutes, seconds);

	fp = fopen(buf, "w");

	fprintf(fp, "%1f\n%1f", gameTime, result);

	//printf("%s", buf);

	fclose(fp);

	FILE* fpt;

	fpt = fopen("./saves/savesNames.txt", "w");

	fprintf(fpt, "%s\n", buf);

	fclose(fpt);

}


// wczytanie gry
void loadGame(char* str, double* worldTime, double* score, int* load) {

	FILE* fp = fopen(str, "r");

	char line[100];
	int b = 0;
		while (fgets(line, sizeof(line), fp)) {
			b++;
			if (b == 1) *worldTime = atof(line) * 0.001;
			else if (b == 2) *score = atof(line);
		};

		*load = 0;

}


// jezeli wcisnieto jakis przycisk..
void keyDown(SDL_Event event, int* quit, double* etiSpeed, int* move, int* newGame, bool* pause, int* load, double* worldTime, double* score, bool* load1, char* str) {
	switch (event.key.keysym.sym) {

	case SDLK_ESCAPE:                 // wcisnieto ESC -> wyjdz z gry
		*quit = 1;
		break;

	case SDLK_UP:                     // wcisnieto strzalke w gore -> akceleracja
		*etiSpeed += 50.0;
		break;

	case SDLK_DOWN:                   // wcisnieto strzalke w dol -> zwolnienie
		*etiSpeed -= 45.0;
		break;

	case SDLK_LEFT:                   // wcisnieto strzalke w lewo -> przesunac spritea w lewo o 10 jednostek
		if (SCREEN_WIDTH / 2 + (*move) > LEFT_BORDER) {
			if (*pause != false) {
				*move = 0;
			}
			else {
				*move -= 10;
			}
		}
		else { *newGame = 1; }
		break;

	case SDLK_RIGHT:                  // wcisnieto strzalke w prawo -> przesunac spritea w prawo o 10 jednostek
		if (SCREEN_WIDTH / 2 + (*move) < RIGHT_BORDER) {
			if (*pause != false) {
				*move = 0;
			}
			else {
				*move += 10;
			}
		}
		else { *newGame = 1; }
		break;

	case SDLK_n:                      // wcisnieto litere "n"
		*newGame = 1;
		*load = 0;
		break;

	case SDLK_p:
		*pause = 1;
		break;

	case SDLK_r:
		*pause = 0;
		break;

	case SDLK_s:
		saveGame(worldTime, score, load1);
		*quit = 1;

		//*pause = 1;
		break;

	case SDLK_l:

		//*load = 1;
		//*load1 = true;
		loadGame(str, worldTime, score, load);
		break;
	};
}


// jezeli zaszly dzialania od uzytkownika..
void keyPressed(SDL_Event event, int* quit, double* etiSpeed, int* move, int* newGame, SDL_Surface* screen, double* distance2, double* worldTime, bool* pause, int* t1, int* load, double* score, bool* load1, char* str) {

	switch (event.type) {
		// jezeli wcisnieto jakis przycisk..
	case SDL_KEYDOWN:
		keyDown(event, quit, etiSpeed, move, newGame, pause, load, worldTime, score, load1, str);
		break;

		// jezeli ten przycisk juz nie jest wcisniety..
	case SDL_KEYUP:
		break;

	case SDL_QUIT:
		*quit = 1;
		break;
	};

}


// zwolnienie powierzchni
void quitGame(SDL_Surface* charset, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window) {
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}


// wczytywanie znakow
int loadCharset(SDL_Surface* charset, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		quitGame(charset, screen, scrtex, renderer, window);
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);
};


// wczytywanie mapy
int loadBackground(SDL_Surface* background, SDL_Surface* charset, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	if (background == NULL) {
		printf("SDL_LoadBMP(map4.bmp) error: %s\n", SDL_GetError());
		quitGame(charset, screen, scrtex, renderer, window);
		return 1;
	};
}


// wczytywanie przeszkody
int loadObstacle(SDL_Surface* obstacle, SDL_Surface* charset, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	if (obstacle == NULL) {
		printf("SDL_LoadBMP(obstacle.bmp) error: %s\n", SDL_GetError());
		quitGame(charset, screen, scrtex, renderer, window);
		return 1;
	};
}


// wczytywanie spritea
int loadSprite(SDL_Surface* eti, SDL_Surface* charset, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	if (eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		quitGame(charset, screen, scrtex, renderer, window);
		return 1;
	};
}


// wczytanie menu
int loadMenu(SDL_Surface* menu, SDL_Surface* charset, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	if (menu == NULL) {
		printf("SDL_LoadBMP(menu.bmp) error: %s\n", SDL_GetError());
		quitGame(charset, screen, scrtex, renderer, window);
		return 1;
	};
}


// wczytywanie obrazkow
void loadImages(SDL_Surface* charset, SDL_Surface* background, SDL_Surface* obstacle, SDL_Surface* eti, SDL_Surface* menu, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {

	// wczytanie znakow
	loadCharset(charset, screen, scrtex, window, renderer);

	// wczytanie mapy
	loadBackground(background, charset, screen, scrtex, window, renderer);

	// wczytanie przeszkody
	loadObstacle(obstacle, charset, screen, scrtex, window, renderer);

	// wczytanie spritea
	loadSprite(eti, charset, screen, scrtex, window, renderer);

	// wczytanie menu
	loadMenu(menu, charset, screen, scrtex, window, renderer);

}


// sprawdz czy okno i renderer byly stworzone
int checkRC(int* rc) {

	if ((*rc) != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}

}


// ustawienia SDL: ustawic rozmiar okna, renderer, tytul okna, ukryc kursor 
void createSettings(SDL_Renderer* renderer, SDL_Window* window) {

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "Spy Hunter | Taras Shuliakevych | 196615");
	SDL_ShowCursor(SDL_DISABLE);

}


// liczenie czasu
void countTime(int* t2, int* t1, double* delta, double* worldTime, bool pause) {

	*t2 = SDL_GetTicks();
	*delta = (*t2 - *t1) * 0.001;
	*t1 = *t2;
	*worldTime += *delta * !pause;

}


// zwolnic sprite pgdy jedzie przy granicy
void slowDown(double* etiSpeed) {

	if (*etiSpeed < 0) {
		*etiSpeed = 0;
	}
	else if (*etiSpeed > 0) {
		*etiSpeed -= 5;
	}

}


// kolizja
void collapse(int* move, int* randomX, int* newGame, double* distance2, int CarL, int CarP, int ObsL, int ObsP) {

	if ((WSPOLRZEDNA_SPRITEA_X - CAR_HEIGHT / 2 <= COORDS + (int)*distance2 + ROZMIAR_PRZESZKODY / 2) && (WSPOLRZEDNA_SPRITEA_Y - CAR_HEIGHT / 2 >= COORDS + (int)*distance2 - ROZMIAR_PRZESZKODY / 2) && (CarP > ObsL && CarL < ObsP)) {
		*newGame = 1;
	}

}


// oblicz granice spritea
void countBordersOfSprite(int* move, int* CarL, int* CarP) {
	*CarL = (WSPOLRZEDNA_SPRITEA_X + *move) - CAR_WIDTH / 2;
	*CarP = (WSPOLRZEDNA_SPRITEA_X + *move) + CAR_WIDTH / 2;
}


// oblicz granice przeszkody
void countBordersOfObstacle(int* randomX, int* ObsL, int* ObsP) {
	*ObsL = *randomX - (ROZMIAR_PRZESZKODY / 2);
	*ObsP = *randomX + (ROZMIAR_PRZESZKODY / 2);
}


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {

	int t1, t2, quit, frames, rc, newGame = 0;
	bool pause = false;
	double delta{}, worldTime, fpsTimer, fps, distance, distance2 = 0.0, etiSpeed, copyEtiSpeed = 0, copyWorldTime = 0;
	char text[128], text1[128];

	SDL_Event    event;
	SDL_Surface* screen, * charset;
	SDL_Surface* eti;
	SDL_Surface* background, * obstacle, * menu;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer); // tryb okienny
	checkRC(&rc);

	createSettings(renderer, window);
	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// wczytanie obrazkow 
	charset = SDL_LoadBMP("./cs8x8.bmp");
	background = SDL_LoadBMP("./map4.bmp");
	obstacle = SDL_LoadBMP("./obstacle.bmp");
	eti = SDL_LoadBMP("./carr.bmp");
	menu = SDL_LoadBMP("./menu.bmp");

	loadImages(charset, background, obstacle, eti, menu, screen, scrtex, window, renderer);

	// deklaracja kolorow
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00), zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00), niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int move = 1;
	double score = 0, wyn = 0;
	int randomX = 0;
	bool spawn = false;
	int CarL = 0, CarP = 0, ObsL = 0, ObsP = 0;
	frames = 0, fpsTimer = 0, quit = 0, worldTime = 0, distance = 0, etiSpeed = 0;
	fps = 60;

	bool load1 = false, load2 = false, load3 = false;
	int dataLoad1 = 0, dataLoad2 = 0, dataLoad3 = 0;
	int load = 1;
	FILE* fpt;
	char str[256];
	fpt = fopen("./saves/savesNames.txt", "r");

	int charNum = fscanf(fpt, "%s", str);
	fseek(fpt, 0, SEEK_END);
	int stream = 0;
	if (fpt != NULL) stream = ftell(fpt);

	if (stream != 0) {
		sprintf(text1, "Last save: %s", str);
		fclose(fpt);
	}
	else {
		sprintf(text1, "Last Save: None");
		fclose(fpt);
	}


	while (!quit) {

		if (load == 1) {

			//worldTime = false;

			DrawSurface(screen, menu, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			DrawRectangle(screen, 10, 400, SCREEN_WIDTH - 20, 150, zielony, czarny);

			sprintf(text, "Choose your saved game or start a new one:");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 420, text, charset);

			sprintf(text, "(Press the number of a game you want to load)");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 440, text, charset);

			DrawString(screen, screen->w / 2 - strlen(text1) * 8 / 2, 455, text1, charset);

			sprintf(text, "New game: 'n'");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 500, text, charset);

			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);
			while (SDL_PollEvent(&event)) keyPressed(event, &quit, &etiSpeed, &move, &newGame, screen, &distance2, &worldTime, &pause, &t1, &load, &score, &load1, str);
		}
		else {

			load = 0;

			srand(time(NULL));

			if (newGame == 1) {
				t1 = SDL_GetTicks();
				srand(time(NULL));
				randomX = rand() % ((SCREEN_HEIGHT / 2 + 100) - SCREEN_HEIGHT + 1) - (SCREEN_HEIGHT / 2 + 100);
				resetGame(&worldTime, &score, &etiSpeed, &distance2, &distance, &newGame, &move);
			}

			countTime(&t2, &t1, &delta, &worldTime, pause);
			SDL_FillRect(screen, NULL, czarny);

			if (etiSpeed - 0.5 >= 0) etiSpeed -= 0.5;

			distance += etiSpeed * delta * !pause;
			distance2 += etiSpeed * delta * !pause;

			if (WSPOLRZEDNA_SPRITEA_X + (move) < LEFT_SLOW || WSPOLRZEDNA_SPRITEA_X + (move) > RIGHT_SLOW) slowDown(&etiSpeed);

			countPoints(&etiSpeed, &score, &delta, &pause, move); // obliczenie wyniku gry:
			drawMap(distance2, screen, background, spawn); // rysowanie mapy

			if (COORDS + distance2 >= COORDS + PIC_HEIGHT / 2 + 5 && !spawn) {
				srand(time(NULL));
				spawn = true;
				randomX = LEFT_BORDER + ((rand() * 1234) % (RIGHT_BORDER - LEFT_BORDER));
			}
			else { drawObstacles(screen, obstacle, distance2, &randomX); }

			countBordersOfSprite(&move, &CarL, &CarP);
			countBordersOfObstacle(&randomX, &ObsL, &ObsP);
			collapse(&move, &randomX, &newGame, &distance2, CarL, CarP, ObsL, ObsP);
			srand(time(NULL));
			drawSprite(screen, eti, &move); // rysowanie spritea
			countFPS(&fpsTimer, &delta, &fps, &frames); // obliczenie FPS
			drawInfo(screen, &zielony, &czarny, text, charset, &score, &worldTime, &fps); // rysowanie tekstu informacyjnego

			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

			// obsluga zdarzen (o ile jakies zaszly)
			while (SDL_PollEvent(&event)) keyPressed(event, &quit, &etiSpeed, &move, &newGame, screen, &distance2, &worldTime, &pause, &t1, &load, &score, &load1, str);
			frames++;
		}

	}
	quitGame(charset, screen, scrtex, renderer, window); // zwolnienie powierzchni

	return 0;
};



