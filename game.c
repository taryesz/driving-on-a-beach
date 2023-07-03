#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<fstream>


extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}


#define SCREEN_WIDTH            930
#define SCREEN_HEIGHT           720
#define PIC_HEIGHT                      2825
#define COORDS                          SCREEN_HEIGHT - PIC_HEIGHT / 2
#define OBSTACLE_SIZE           125                     

#define LEFT_BORDER                     296
#define RIGHT_BORDER            626
#define RIGHT_SLOW                      576
#define LEFT_SLOW                       346

#define CAR_WIDTH                       92
#define CAR_HEIGHT                      164

#define CAR_X                           SCREEN_WIDTH / 2
#define CAR_Y                           SCREEN_HEIGHT / 2


// narysowanie napisu txt na powierzchni screen, zaczynajïż½c od punktu (x, y)
// charset to bitmapa 128x128 zawierajïż½ca znaki
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
// (x, y) to punkt ïż½rodka obrazka sprite na ekranie
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


// rysowanie linii o dïż½ugoïż½ci l w pionie (gdy dx = 0, dy = 1) 
// bïż½dïż½ poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
        for (int i = 0; i < l; i++) {
                DrawPixel(screen, x, y, color);
                x += dx;
                y += dy;
        };
};


// rysowanie prostokïż½ta o dïż½ugoïż½ci bokïż½w l i k
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
void resetGame(double* worldTime, double* score, double* etiSpeed, double* distance2, double* distance, int* newGame, int* move, int* lives) {
        *worldTime = 0;
        *score = 0;
        *etiSpeed = 0;
        *distance2 = 0;
        *distance = 0;
        *newGame = 0;
        *move = 0;
        *lives = 3;
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


//obliczenie dodatkowych zyc
void countLives(double* score, int* lives, int* i) {
        int liveScore = (int)*score;
        if (liveScore > 1000) {
                if ((liveScore) % 1000 == 0) {
                        printf("+ %d 1 - \n", liveScore);
                        liveScore -= 1000;
                        printf("+ %d 2 - \n", liveScore);
                        *lives += 1;
                        if (*lives > 3) {
                                *lives = 3;
                        }
                }
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


// rysowanie informacji dotyczcej danych gry: czas, wynik, fps
void drawData(SDL_Surface* screen, int* zielony, int* czarny, char* text, double* worldTime, double* fps, SDL_Surface* charset, double* score, int* lives, int* i) {

        DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 46, *zielony, *czarny);

        sprintf(text, "Spy Hunter | Taras Shuliakevych | 196615 | Czas trwania gry: %.1lf s |  %.0lf klatek / s", *worldTime, *fps);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

        sprintf(text, "wynik: %d", (int)(*score));
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

        countLives(score, lives, i);

        sprintf(text, "liczba zyc: %d", *lives);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 36, text, charset);

}


// rysowanie instrukcji w prawym dolnych rogu
void drawTutorial(SDL_Surface* screen, int* zielony, int* czarny, char* text, double* worldTime, double* fps, SDL_Surface* charset, double* score) {

        DrawRectangle(screen, 530, 635, 384, 80, *zielony, *czarny);

        sprintf(text, "strzalki: poruszanie graczem na planszy");
        DrawString(screen, screen->w - (strlen(text) * 16 - 240), 640, text, charset);

        sprintf(text, "esc: zamkniecie programu");
        DrawString(screen, screen->w - strlen(text) * 16, 650, text, charset);

        sprintf(text, "n: rozpoczecie nowej gry");
        DrawString(screen, screen->w - strlen(text) * 16, 660, text, charset);

        sprintf(text, "s: zapisz biezaca gre");
        DrawString(screen, screen->w - (strlen(text) * 16 + 48), 670, text, charset);

        sprintf(text, "p: pauza");
        DrawString(screen, screen->w - (strlen(text) * 48), 680, text, charset);

        sprintf(text, "r: kontynuuj gre");
        DrawString(screen, screen->w - (strlen(text) * 24), 690, text, charset);

        sprintf(text, "l: zaladuj ostatnio zapisana gre");
        DrawString(screen, screen->w - (strlen(text) * 12), 700, text, charset);

}


// rysowanie tekstu informacyjnego
void drawInfo(SDL_Surface* screen, int* zielony, int* czarny, char* text, SDL_Surface* charset, double* score, double* worldTime, double* fps, int* lives, int* i) {

        drawData(screen, zielony, czarny, text, worldTime, fps, charset, score, lives, i);
        drawTutorial(screen, zielony, czarny, text, worldTime, fps, charset, score);

}


// zapisywanie gry
void saveGame(double* worldTime, double* score, bool* load1) {

        // zmienic wyswitlanie sejwu gry w menu z 'none' na nazwe tego sejwu
        if (*load1 == false) *load1 = true;

        double gameTime = *worldTime, result = *score;
        char buf[0x100];
        int hours, minutes, seconds, day, month, year;

        // wczytywanie czasu swiatowego
        time_t now;
        time(&now);
        struct tm* local = localtime(&now);

        hours = local->tm_hour;
        minutes = local->tm_min;      
        seconds = local->tm_sec;        
        day = local->tm_mday;
        month = local->tm_mon + 1;
        year = local->tm_year + 1900;

        // zapisywanie danych gry w plik
        FILE* fp;
        snprintf(buf, sizeof(buf), "./saves/%d.%d.%d__%d-%d-%d.txt", day, month, year, hours, minutes, seconds);
        fp = fopen(buf, "w");
        fprintf(fp, "%1f\n%1f", gameTime, result);
        fclose(fp);


        // zapisywanie naglowkow plikow z danymi s savesNames.txt
        FILE* fpt;
        fpt = fopen("./saves/savesNames.txt", "w");
        fprintf(fpt, "%s\n", buf);
        fclose(fpt);
}


// wczytanie gry
void loadGame(char* str, double* worldTime, double* score, int* load) {

        FILE* fp = fopen(str, "r");

        char line[100];
        int lineIdx = 0;
        while (fgets(line, sizeof(line), fp)) {
                lineIdx++;
                if (lineIdx == 1) *worldTime = atof(line);
                else if (lineIdx == 2) *score = atof(line);
        };

        *load = 0;

}


// jezeli wcisnieto jakis przycisk..
void keyDown(SDL_Event event, int* quit, double* etiSpeed, int* move, int* newGame, bool* pause, int* load, double* worldTime, double* score, bool* load1, char* str) {
        switch (event.key.keysym.sym) {

        case SDLK_ESCAPE: // wcisnieto ESC -> wyjdz z gry
                *quit = 1;
                break;

        case SDLK_UP: // wcisnieto strzalke w gore -> akceleracja
                *etiSpeed += 50.0;
                break;

        case SDLK_DOWN: // wcisnieto strzalke w dol -> zwolnienie
                *etiSpeed -= 45.0;
                break;

        case SDLK_LEFT: // wcisnieto strzalke w lewo -> przesunac spritea w lewo o 10 jednostek
                if (CAR_X + (*move) > LEFT_BORDER) {
                        if (*pause != false) *move = 0;
                        else { *move -= 10; }
                }
                else { *newGame = 1; }
                break;

        case SDLK_RIGHT: // wcisnieto strzalke w prawo -> przesunac spritea w prawo o 10 jednostek
                if (CAR_X + (*move) < RIGHT_BORDER) {
                        if (*pause != false) *move = 0;
                        else { *move += 10; }
                }
                else { *newGame = 1; }
                break;

        case SDLK_n: // wcisnieto litere "n" -> nowa gra
                *newGame = 1;
                *load = 0;
                break;

        case SDLK_p: // wcisnieto litere "p" -> pauza
                *pause = 1;
                break;

        case SDLK_r: // wcisnieto litere "r" -> kontynuuj gre
                *pause = 0;
                break;

        case SDLK_s: // wciesnieto litere "s" -> zapisz biezaca gre
                if (*load != true) {
                        saveGame(worldTime, score, load1);
                        *quit = 1;
                }
                break;

        case SDLK_l: // wcisnieto litere "l" -> zaladuj zapisana gre
                loadGame(str, worldTime, score, load);
                *newGame = 2;
                break;
        };
}


// jezeli zaszly dzialania od uzytkownika..
void keyPressed(SDL_Event event, int* quit, double* etiSpeed, int* move, int* newGame, SDL_Surface* screen, double* distance2, double* worldTime, bool* pause, int* t1, int* load, double* score, bool* load1, char* str) {

        switch (event.type) {

        case SDL_KEYDOWN: // jezeli wcisnieto jakis przycisk
                keyDown(event, quit, etiSpeed, move, newGame, pause, load, worldTime, score, load1, str);
                break;

        case SDL_KEYUP: // jezeli ten przycisk juz nie jest wcisniety
                break;

        case SDL_QUIT: // wyjscie z gry
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
void loadImages(SDL_Surface* charset, SDL_Surface* background, SDL_Surface* obstacle, SDL_Surface* eti, SDL_Surface* menu, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer, SDL_Surface* civil) {

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

        if (*etiSpeed < 0) *etiSpeed = 0;
        else if (*etiSpeed > 0) *etiSpeed -= 5;

}


// kolizja
void collapse(int* move, int* randomX, int* newGame, double* distance2, int CarL, int CarP, int ObsL, int ObsP, double* worldTime, int* lives, double* etiSpeed, char* text, SDL_Surface* screen, SDL_Surface* charset) {

        if (*worldTime > 7) {
                if ((CAR_Y - CAR_HEIGHT / 2 <= COORDS + (int)*distance2 + OBSTACLE_SIZE / 2) && (CAR_Y - CAR_HEIGHT / 2 >= COORDS + (int)*distance2 - OBSTACLE_SIZE / 2) && (CarP > ObsL && CarL < ObsP)) {
                        if (*lives > 0) {
                                *distance2 -= 200;
                                *etiSpeed = 0;
                                *lives -= 1;
                        }
                        else {
                                *newGame = 1;
                        }
                }
        }
        else {
                if ((CAR_Y - CAR_HEIGHT / 2 <= COORDS + (int)*distance2 + OBSTACLE_SIZE / 2) && (CAR_Y - CAR_HEIGHT / 2 >= COORDS + (int)*distance2 - OBSTACLE_SIZE / 2) && (CarP > ObsL && CarL < ObsP)) {
                        *move = 0;
                }
        }

}


// oblicz granice spritea
void countBordersOfSprite(int* move, int* spriteLB, int* spriteRB) {
        *spriteLB = (CAR_X + *move) - CAR_WIDTH / 2;
        *spriteRB = (CAR_X + *move) + CAR_WIDTH / 2;
}


// oblicz granice przeszkody
void countBordersOfObstacle(int* randomX, int* obstacleLB, int* obstacleRB) {
        *obstacleLB = *randomX - (OBSTACLE_SIZE / 2);
        *obstacleRB = *randomX + (OBSTACLE_SIZE / 2);
}


// sprawdz czy plik jest pusty
void isFileEmpty(char* str, int* stream, char* text1) {
        FILE* fpt;
        fpt = fopen("./saves/savesNames.txt", "r");

        int charNum = fscanf(fpt, "%s", str);
        fseek(fpt, 0, SEEK_END);
        if (fpt != NULL) *stream = ftell(fpt);

        if (*stream != 0) {
                sprintf(text1, "Last save: %s", str);
                fclose(fpt);
        }
        else {
                sprintf(text1, "Last Save: None");
                fclose(fpt);
        }
}


// rysuj menu
void drawMenu(SDL_Surface* screen, SDL_Surface* charset, SDL_Surface* menu, int* zielony, int* czarny, char* text, char* text1) {
        DrawSurface(screen, menu, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        DrawRectangle(screen, 10, 400, SCREEN_WIDTH - 20, 150, *zielony, *czarny);

        sprintf(text, "Choose your saved game or start a new one:");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 420, text, charset);

        sprintf(text, "(Press the number of a game you want to load)");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 440, text, charset);

        DrawString(screen, screen->w / 2 - strlen(text1) * 8 / 2, 455, text1, charset);

        sprintf(text, "New game: 'n'");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 500, text, charset);
}


// ustawienia SDLa
void screenSettings(SDL_Texture* scrtex, SDL_Surface* screen, SDL_Renderer* renderer) {
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
}


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {

        int t1, t2, quit, frames, rc, newGame = 0, i = 1;
        bool pause = false;
        double delta, worldTime, fpsTimer, fps, distance, distance2 = 0.0, etiSpeed;
        char text[128], text1[128];

        SDL_Event    event;
        SDL_Surface* screen, *charset;
        SDL_Surface* sprite;
        SDL_Surface* background, *obstacle, *menu, *civil;
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
        sprite = SDL_LoadBMP("./carr.bmp");
        menu = SDL_LoadBMP("./menu.bmp");
        civil = SDL_LoadBMP("./civil.bmp");

        loadImages(charset, background, obstacle, sprite, menu, screen, scrtex, window, renderer, civil);

        // deklaracja kolorow oraz innych zmiennych
        int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00), zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
        int move = 1, randomX = 0, spriteLB = 0, spriteRB = 0, obstacleLB = 0, obstacleRB = 0, CivilCarL = 0, CivilCarP = 0, load = 1, stream = 0, lives = 3, carRandomX = 0;
        double score = 0, wyn = 0;
        bool spawn = false, load1 = false, wasCarSpawned = false;
        char str[256];

        frames = 0, fpsTimer = 0, quit = 0, worldTime = 0, distance = 0, etiSpeed = 0;
        fps = 60;

        // sprawdz czy plik ma jakas zaratosc - odpowiednio rysuj menu
        isFileEmpty(str, &stream, text1);

        // gra
        while (!quit) {

                if (load == 1) { // jezeli aktualnie jest pauza

                        drawMenu(screen, charset, menu, &zielony, &czarny, text, text1);
                        screenSettings(scrtex, screen, renderer);

                        // reaguj na wcisniecie klawiszy
                        while (SDL_PollEvent(&event)) keyPressed(event, &quit, &etiSpeed, &move, &newGame, screen, &distance2, &worldTime, &pause, &t1, &load, &score, &load1, str);

                }
                else {

                        load = 0;
                        srand(time(NULL));

                        if (newGame == 1) {
                                t1 = SDL_GetTicks();
                                srand(time(NULL));
                                randomX = rand() % ((SCREEN_HEIGHT / 2 + 100) - SCREEN_HEIGHT + 1) - (SCREEN_HEIGHT / 2 + 100);
                                resetGame(&worldTime, &score, &etiSpeed, &distance2, &distance, &newGame, &move, &lives);
                        } 
                        else if (newGame == 2) {
                                t1 = SDL_GetTicks();
                                newGame = 0;
                        }

                        countTime(&t2, &t1, &delta, &worldTime, pause); // liczenie czasu w grze
                        SDL_FillRect(screen, NULL, czarny);
                        if (distance2 == 0) wasCarSpawned = false;
                        //countLives(&score, &lives);

                        collapse(&move, &randomX, &newGame, &distance2, spriteLB, spriteRB, obstacleLB, obstacleRB, &worldTime, &lives, &etiSpeed, text, screen, charset); // mechanika kolizji
                        if (etiSpeed - 0.5 >= 0) etiSpeed -= 0.5;
                        distance += etiSpeed * delta * !pause;
                        distance2 += etiSpeed * delta * !pause;

                        if (CAR_X + (move) < LEFT_SLOW || CAR_X + (move) > RIGHT_SLOW) slowDown(&etiSpeed); // zwolnij spritea przy granicach mapy

                        countPoints(&etiSpeed, &score, &delta, &pause, move); // obliczenie wyniku gry
                        drawMap(distance2, screen, background, spawn); // rysowanie mapy

                        if (COORDS + distance2 >= COORDS + PIC_HEIGHT / 2 + 5 && !spawn) {
                                srand(time(NULL));
                                spawn = true;
                                randomX = LEFT_BORDER + ((rand() * 1234) % (RIGHT_BORDER - LEFT_BORDER));
                        }
                        else { drawObstacles(screen, obstacle, distance2, &randomX); }

                        countBordersOfSprite(&move, &spriteLB, &spriteRB); // oliczenie granic spritea
                        countBordersOfObstacle(&randomX, &obstacleLB, &obstacleRB); // obliczenie granic przeszkody
                        srand(time(NULL));
                        drawSprite(screen, sprite, &move); // rysowanie spritea
                        countFPS(&fpsTimer, &delta, &fps, &frames); // obliczenie FPS
                        drawInfo(screen, &zielony, &czarny, text, charset, &score, &worldTime, &fps, &lives, &i); // rysowanie tekstu informacyjnego
                        screenSettings(scrtex, screen, renderer);

                        // obsluga zdarzen (o ile jakies zaszly)
                        while (SDL_PollEvent(&event)) keyPressed(event, &quit, &etiSpeed, &move, &newGame, screen, &distance2, &worldTime, &pause, &t1, &load, &score, &load1, str);
                        frames++;
                }

        }
        quitGame(charset, screen, scrtex, renderer, window); // zwolnienie powierzchni

        return 0;
};
