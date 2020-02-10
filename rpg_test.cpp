// [C Game] Simple RPG
// made by "PrintedLove"
// https://printed.tistory.com/
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#define FALSE 0
#define TRUE 1
#define MAP_X_MAX 96
#define MAP_Y_MAX 32
#define FLOOR_Y 22

typedef struct _Character {
    short x = MAP_X_MAX / 2 + 1, y = MAP_Y_MAX / 2;
    float x_accel = 0, y_accel = 0;
    short size_x = 3, size_y = 3;
    float flyTime = 0;
    bool direction = TRUE;	//true=right, false=left
    			//character stat
    char name[16];
    int lv = 1;
    unsigned long int exp = 0, expi = 100;
    int hpm = 100, hp = hpm;
    int mpm = 50, mp = mpm;
    int power = 10;
    short weapon = 0;
    			//animation control
    short leg_m = 1, atk_m[3] = {FALSE, 0, 1};	//motion
    unsigned int gen_tick, leg_tick = 0, atk_tick = 0;
}Character;

typedef struct _Enemy {
    short x, y;
    float x_accel = 0, y_accel = 0;
    short size_x, size_y;
    float flyTime = 0;
    bool direction = TRUE;
    
    bool live = FALSE;
    short kinds;
    int hpm, hp;
    int dam;
}Enemy;

Character character;

void SetConsole();
void StartGame();
void UpdateGame(unsigned int sistem_tick);
void ControlUI(unsigned int sistem_tick);
void ControlCharacter(unsigned int sistem_tick);

void FillMap(char str[], char str_s, int max_value);
void EditMap(int x, int y, char str);
void DrawSprite(int x, int y, int size_x, int size_y, char spr[]);
void DrawNumber(int x, int y, int num);
void DrawBox(int x, int y, int size_x, int size_y);
void MotionControl(short *x, short *y, float *x_accel, float *y_accel, short size_x, short size_y, float *flyTime);

int NumLen(int num);
int StrLen(char str[]);

short stat_weapon[] = {5, 10, 15};
int list_object[16];

char sprite_floor[MAP_X_MAX];
char sprite_character[10] = " 0  | _^_";
char sprite_character_leg[2][3][4] = 
{{"-^.", "_^\'", "_^."},
 {".^-", "\'^_", ".^_"}};
char sprite_invenWeapon[][11] = {"   /   /  ", "   /  '+. ", "  |   \"+\" "};
char sprite_Weapon[][3][4] = 
{{"---", "--+", "<=+"},
 {"---", "+--", "+=>"}};
char sprite_normalAttack[][3][16] = 
{{".-  o          ", " .   (   o \'   ", "         o \'-  "},
 {"o -.           ", "   . o   )   \' ", "     o      -\' "}};

char mapData[MAP_X_MAX * MAP_Y_MAX];	//array for graphics

int main() {
	StartGame();
	
	unsigned int system_tick = GetTickCount();
	
	while (TRUE) {
		if (system_tick + 30 < GetTickCount()) {
			system_tick = GetTickCount();
			
			UpdateGame(system_tick);
		}
	}
	
	return 0;
}

void SetConsole() {
	system("mode con:cols=96 lines=32");
	system("title RPG test");
	
	HANDLE hConsole;
    CONSOLE_CURSOR_INFO ConsoleCursor;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    ConsoleCursor.bVisible = 0;
    ConsoleCursor.dwSize = 1;
    SetConsoleCursorInfo(hConsole , &ConsoleCursor);
    
    srand((unsigned int)time(NULL));
}

void StartGame() {
	SetConsole();
	
	printf("Enter your name: ");
	scanf("%[^\n]s", character.name);
	
	FillMap(sprite_floor, '=', MAP_X_MAX);
}

void UpdateGame(unsigned int sistem_tick) {
	unsigned int tick = sistem_tick;
	
	FillMap(mapData, ' ', MAP_X_MAX * MAP_Y_MAX);	//initialize mapdata
	
	ControlUI(tick);
	ControlCharacter(tick);
	
	printf("%s", mapData);	//update mapdata
}

void ControlUI(unsigned int sistem_tick) {
	unsigned int tick = sistem_tick;
	int len;	//length of previous sprite
	
	DrawSprite(1, FLOOR_Y, MAP_X_MAX, 1, sprite_floor);	//draw floor
	
	DrawBox(1, 1, 35, 8); DrawBox(27, 4, 7, 4);
	DrawSprite(28, 5, 5, 2, sprite_invenWeapon[character.weapon]);
	DrawSprite(28, 3, 6, 1, "Weapon");	//draw weaponinven
	
	EditMap(3, 2, '\"');	//draw name, lv, exp
	DrawSprite(4, 2, StrLen(character.name), 1, character.name); len = 4 + StrLen(character.name);
	DrawSprite(len, 2, 7, 1, "\" LV."); len += 5;
	DrawNumber(len, 2, character.lv); len += NumLen(character.lv);
	DrawSprite(len, 2, 2, 1, " ("); len += 2;
	
	int expPer = roundf(character.exp * 100 / character.expi);
	
	if (!expPer) {
		EditMap(len, 2, '0'); len ++;
	} else {
		DrawNumber(len, 2, expPer); len += NumLen(expPer);
	}
	
	DrawSprite(len, 2, 2, 1, "%)");
	
	DrawSprite(4, 4, 3, 1, "HP:");	//draw HP
	DrawNumber(8, 4, character.hp);
	EditMap(9 + NumLen(character.hp), 4, '/');
	DrawNumber(11 + NumLen(character.hp), 4, character.hpm);
	
	DrawSprite(4, 5, 3, 1, "MP:");	//draw MP
	DrawNumber(8, 5, character.mp);
	EditMap(9 + NumLen(character.mp), 5, '/');
	DrawNumber(11 + NumLen(character.mp), 5, character.mpm);
	
	DrawSprite(4, 7, 6, 1, "Power:");	//draw power
	DrawNumber(11, 7, character.power);
}

void ControlCharacter(unsigned int sistem_tick) {
	unsigned int tick = sistem_tick;
	bool move = FALSE, attack = FALSE;
	
	if (character.exp >= character.expi) {	//LV up
		character.lv ++; character.hpm += 10; character.mpm += 5; character.power ++;
		character.exp = 0; character.expi += character.lv * 10;
	}
	
	if (character.gen_tick + 900 < tick) {	//hp, mp
		character.gen_tick = tick;
		character.hp += roundf(character.hpm * 0.01);
		character.mp += roundf(character.mpm * 0.05);
	}
	if (character.hp > character.hpm) character.hp = character.hpm;
	if (character.mp > character.mpm) character.mp = character.mpm;
	
	if (GetAsyncKeyState(0x5A) & 0x8000 && character.flyTime == 0) {	//keyboard
		attack = TRUE;
		character.atk_m[0] = TRUE;
	}
	
	if (character.atk_m[0]) {
		if (tick > character.atk_tick + 150) {	//attack motion calculation
			character.atk_tick = tick;
			character.atk_m[1]++;
		}
		
		if (character.atk_m[1] > 3) {
			if (attack) {
				character.atk_m[1] = 1; character.atk_m[2]++;	
			} else {
				character.atk_m[0] = FALSE; character.atk_m[1] = 0; character.atk_m[2] = 1;
			}
			
			if (character.atk_m[2] > 3) {
				character.atk_m[2] = 1;
				
				if (character.direction) {
					if (character.x < MAP_X_MAX - 2) character.x++;
				} else {
					if (character.x > 1) character.x--;
				}
			}	
		}
	} else {
		if (GetAsyncKeyState(VK_LEFT) & 0x8000 && character.x > 1) {	//move left
			character.x_accel = -1;
			character.direction = FALSE;
			move = TRUE;
		}
		
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && character.x < MAP_X_MAX - 2) {	//move right
			character.x_accel = 1;
			character.direction = TRUE;
			move = TRUE;
		}
	}
	
	if (GetAsyncKeyState(VK_UP) & 0x8000 && character.y + 3 == FLOOR_Y)	//jump
			character.y_accel = -1.75;
	
	if (GetAsyncKeyState(0x31) & 0x8000)	//1
		character.weapon = 0;
		
	if (GetAsyncKeyState(0x32) & 0x8000)	//2
		character.weapon = 1;
	
	if (GetAsyncKeyState(0x33) & 0x8000)	//3
		character.weapon = 2;
	
	MotionControl(&character.x, &character.y, &character.x_accel, &character.y_accel, character.size_x, character.size_y, &character.flyTime);
	
	if (tick > character.leg_tick + 90) {	//leg tick	
		character.leg_tick = tick;
		
		if (move == TRUE)
			character.leg_m++;
		else
			character.leg_m = 0;
			 
		if (character.leg_m > 3)
			character.leg_m = 1;
	}
	
	DrawSprite(character.x, character.y, character.size_x, character.size_y, sprite_character);	//draw character sprite
	
	if (character.direction) {
		EditMap(character.x, character.y + 1, '(');
	} else {
		EditMap(character.x + 2, character.y + 1, ')');
	}
		
	if (character.atk_m[0] && character.atk_m[1] > 0) {	//draw attack motion
		switch (character.atk_m[2]) {
			case 1:
				DrawSprite(character.x - 5 + 8 * character.direction, character.y, 5, 3, sprite_normalAttack[character.direction][character.atk_m[1] - 1]);
				break;
			case 2:
				switch (character.atk_m[1]) {
					case 1:
						EditMap(character.x + 2 * character.direction, character.y + 1, 'o');
						DrawSprite(character.x - 3 + 6 * character.direction, character.y + 1, 3, 1, sprite_Weapon[character.direction][character.weapon]);
						break;
					case 2:
						EditMap(character.x - 2 + 6 * character.direction, character.y + 1, 'o');
						DrawSprite(character.x - 5 + 10 * character.direction, character.y + 1, 3, 1, sprite_Weapon[character.direction][character.weapon]);
						break;
					case 3:
						EditMap(character.x + 2 * character.direction, character.y + 1, 'o');
						DrawSprite(character.x - 3 + 6 * character.direction, character.y + 1, 3, 1, sprite_Weapon[character.direction][character.weapon]);
						break;
				} break;
			case 3:
				switch (character.atk_m[1]) {
					case 1:
						EditMap(character.x - 1 + 4 * character.direction, character.y + 2, 'o');
						DrawSprite(character.x - 4 + 8 * character.direction, character.y + 2, 3, 1, sprite_Weapon[character.direction][character.weapon]);
						DrawSprite(character.x - 4 + 8 * character.direction, character.y, 3, 1, sprite_Weapon[character.direction][character.weapon]);
						break;
					case 2:
						EditMap(character.x - 2 + 6 * character.direction, character.y + 1, 'o');
						DrawSprite(character.x - 5 + 10 * character.direction, character.y + 1, 3, 1, sprite_Weapon[character.direction][character.weapon]);
						break;
					case 3:
						EditMap(character.x - 1 + 4 * character.direction, character.y, 'o');
						DrawSprite(character.x - 4 + 8 * character.direction, character.y + 2, 3, 1, sprite_Weapon[character.direction][character.weapon]);
						DrawSprite(character.x - 4 + 8 * character.direction, character.y, 3, 1, sprite_Weapon[character.direction][character.weapon]);
						break;
				} break;
		}
		
		if ((character.atk_m[2] < 3 && character.atk_m[1] == 2) || (character.atk_m[2] == 3 && character.atk_m[1] > 1)) {
			EditMap(character.x + 2 - 2 * character.direction, character.y + 2, '.');
			EditMap(character.x + 2 * character.direction, character.y + 2, '-');
			EditMap(character.x + 1, character.y + 1, 'l');
		}
	} else {
		EditMap(character.x + character.direction * 2, character.y + 1, 'o');
		DrawSprite(character.x - 3 + 6 * character.direction, character.y + 1, 3, 1, sprite_Weapon[character.direction][character.weapon]);
		
		if (character.leg_m == 3)
			EditMap(character.x + 1, character.y + 1, 'l');
	}
	
	if (move)
		DrawSprite(character.x, character.y + 2, 3, 1, sprite_character_leg[character.direction][character.leg_m - 1]);	//draw leg motion
}

void FillMap(char str[], char str_s, int max_value) {	//array initialization
	for (int i = 0; i < max_value; i++)
		str[i] = str_s;
}

void EditMap(int x, int y, char str) {	// edit x, y coordinate mapdata
	if (x > 0 && y > 0 && x - 1 < MAP_X_MAX && y - 1 < MAP_Y_MAX)
		mapData[(y - 1) * MAP_X_MAX + x - 1] = str;
}

void DrawSprite(int x, int y, int size_x, int size_y, char spr[]) {	//draw sprite of size_x, size_y at x, y coordinates
	for (int i = 0; i < size_y; i++) {
		for (int n = 0; n < size_x; n++)
			EditMap(x + n, y + i, spr[i * size_x + n]);
	}
}

void DrawNumber(int x, int y, int num) {	//draw numbers at x, y coordinates (align left)
	int tmp = num;
	short len = NumLen(tmp), cnt = len;
    char str[len];
    
    do {
        cnt--;
        str[cnt] = (char)(tmp % 10 + 48);
        tmp /= 10;
    } while(tmp != 0);
    
    DrawSprite(x, y, len, 1, str);
}

void DrawBox(int x, int y, int size_x, int size_y) {	//draw box of size_x, size_y at x, y coordinates
	EditMap(x, y, '.'); EditMap(x + size_x - 1, y, '.');
	EditMap(x, y + size_y - 1, '\''); EditMap(x + size_x - 1, y + size_y - 1, '\'');
	
	for (int i = 1; i < size_x - 1; i++) {
		EditMap(x + i, y, '-'); EditMap(x + i, y + size_y - 1, '-');
	}
	
	for (int i =  1; i < size_y - 1; i++) {
		EditMap(x, y + i, '|'); EditMap(x + size_x - 1, y + i, '|');
	}
}

void MotionControl(short *x, short *y, float *x_accel, float *y_accel, short size_x, short size_y, float *flyTime) {	// Object motion control
	
	float x_value = *x_accel, y_value = *y_accel;
	
	if (*y + size_y < FLOOR_Y) {
		*flyTime += 0.05;
		*y_accel += *flyTime;
	} else {
		*flyTime = 0;
	}
	
	if (x_value != 0 || y_value != 0) {
		if (*x + x_value < 1)
			x_value = 1 - *x;
		if (*x + size_x + x_value > MAP_X_MAX)
			x_value = MAP_X_MAX - size_x - *x;
		if (*y + size_y + y_value > FLOOR_Y)
			y_value = FLOOR_Y - *y - size_y;
	}
	
	*x += floor(x_value + 0.5); *y += floor(y_value + 0.5);
	
	if (*x_accel > 0) *x_accel -= 0.1; if (*x_accel < 0) *x_accel += 0.1;
	if (*y_accel > 0) *y_accel -= 0.1; if (*y_accel < 0) *y_accel += 0.1;
}

int NumLen(int num) {
	int tmp = num;
	short len = 0;
	
	while(tmp != 0) {
        tmp /= 10;
        len++;
    }
    
    return len;
}

int StrLen(char str[]) {
	short len = 0;
	
	for(int i = 0; str[i]; i++)
        len ++;
    
    return len;
}
