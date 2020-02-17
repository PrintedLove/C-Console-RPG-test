// [C Game] Simple RPG
// made by "PrintedLove"
// https://printed.tistory.com/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#define FALSE 0
#define TRUE 1
#define MAP_X_MAX 96
#define MAP_Y_MAX 32
#define FLOOR_Y 26
#define OBJECT_MAX 32

typedef struct _Character {
    short coord[2] = {MAP_X_MAX / 2, MAP_Y_MAX / 2}, size[2] = {3, 3};	//coordinate value and size
    float accel[2] = {0, }, flyTime = 0;	//acceleration value and flotation time
    bool direction = TRUE;	//true=right, false=left
    			//character stat
    char name[16];
    int lv = 1;
    unsigned int exp[2] = {100, 0};	//0=exp required, 1=current exp
    int hp[2] = {100, 0}, mp[2] = {50, 0};	//0=max value, 1=current value
    short power = 10, weapon = 0;
    unsigned int score = 0;
    			//animation control
    short motion[4] = {1, FALSE, 0, 1};	//motion value		//leg_motion, attack_motion(1, 2, 3)
    unsigned int tick[4] = {0, };	//tick 		//gen_tick, leg_tick, atk_tick, dash_tick
}Character;

typedef struct _Object {	//enemys, projectiles, particles, etc.
    short coord[2], size[2];
    float accel[2], flyTime = 0;
    bool direction = TRUE;
    			//object stat
    short kinds;
    int hp[2];
    int dam;
    			//animation control
    short motion[3];	//motion
    unsigned int tick[4];
}Object;

Character character;
Object **objects;

void StartGame();	//initialize
void UpdateGame();
void SetConsole();
void ControlUI();
void ControlCharacter();
void ControlEnemy();
void CreateObject();
void MotionControl(short coord[], float accel[], short size[], float *flyTime);	// motion control
void DrawBox(short x, short y, short size_x, short size_y);	//draw box of size_x, size_y at x, y coordinates
void DrawNumber(short x, short y, int num);	//draw numbers at x, y coordinates (align left)
void DrawSprite(short x, short y, short size_x, short size_y, char spr[]);	//draw sprite of size_x, size_y at x, y coordinates
void FillMap(char str[], char str_s, int max_value);	//array initialization
void EditMap(short x, short y, char str);	// edit x, y coordinate mapdata
int NumLen(int num);
int StrLen(char str[]);

short stat_weapon[] = {5, 10, 15};
unsigned int tick = 0;

char sprite_floor[MAP_X_MAX];
char sprite_character[10] = " 0  | _^_";
char sprite_character_leg[2][3][4] = 
{{"-^.", "_^\'", "_^."},
 {".^-", "\'^_", ".^_"}};
char sprite_Weapon[2][3][4] = 
{{"---", "--+", "<=+"},
 {"---", "+--", "+=>"}};
char sprite_normalAttack[2][3][16] = 
{{" .- o          ", " .   (   o \'   ", "         o \'-  "},
 {"o -.           ", "   . o   )   \' ", "     o      -\' "}};
char sprite_invenWeapon[3][11] = {"   /   /  ", "   /  '*. ", "  |   \"+\" "};

char mapData[MAP_X_MAX * MAP_Y_MAX];	//array for graphics

int main() {
	StartGame();
	
	while (TRUE) {
		if (tick + 30 < GetTickCount()) {
			tick = GetTickCount();
			
			UpdateGame();
		}
	}
	
	return 0;
}

void StartGame() {
	SetConsole();
	
	printf("Enter your name: ");
	scanf("%[^\n]s", character.name);
	
	FillMap(sprite_floor, '=', MAP_X_MAX);
	
	objects = (Object **)malloc(sizeof(Object *) * OBJECT_MAX);	//dynamic memory allocation
	memset(objects, 0, sizeof(Object *) * OBJECT_MAX);	//memory initialization
}

void UpdateGame() {
	
	FillMap(mapData, ' ', MAP_X_MAX * MAP_Y_MAX);	//initialize mapdata
	
	ControlUI();
	ControlCharacter();
	ControlEnemy();
	
	fputs(mapData, stdout);	//update mapdata
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

void ControlUI() {
	int expPer = roundf(character.exp[1] * 100 / character.exp[0]);
	int len;	//length of previous sprite
	
	DrawSprite(1, FLOOR_Y, MAP_X_MAX, 1, sprite_floor);	//draw floor
	
	DrawBox(1, 1, 35, 8); DrawBox(27, 4, 7, 4);	//draw weaponinven
	DrawSprite(28, 5, 5, 2, sprite_invenWeapon[character.weapon]);
	DrawSprite(28, 3, 6, 1, "Weapon");
	
	EditMap(3, 2, '\"');	//draw name, lv, exp
	DrawSprite(4, 2, StrLen(character.name), 1, character.name); len = 4 + StrLen(character.name);
	DrawSprite(len, 2, 7, 1, "\" LV."); len += 5;
	DrawNumber(len, 2, character.lv); len += NumLen(character.lv);
	DrawSprite(len, 2, 2, 1, " ("); len += 2;
	if (!expPer) {
		EditMap(len, 2, '0'); len ++;
	} else {
		DrawNumber(len, 2, expPer); len += NumLen(expPer);
	}
	DrawSprite(len, 2, 2, 1, "%)");
	
	DrawSprite(MAP_X_MAX - NumLen(character.score) - 7, 2, 6, 1, "SCORE:");	//draw score
	DrawNumber(MAP_X_MAX - NumLen(character.score), 2, character.score);
	
	DrawSprite(4, 4, 3, 1, "HP:");	//draw HP
	DrawNumber(8, 4, character.hp[1]);
	EditMap(9 + NumLen(character.hp[1]), 4, '/');
	DrawNumber(11 + NumLen(character.hp[1]), 4, character.hp[0]);
	
	DrawSprite(4, 5, 3, 1, "MP:");	//draw MP
	DrawNumber(8, 5, character.mp[1]);
	EditMap(9 + NumLen(character.mp[1]), 5, '/');
	DrawNumber(11 + NumLen(character.mp[1]), 5, character.mp[0]);
	
	DrawSprite(4, 7, 6, 1, "Power:");	//draw power
	DrawNumber(11, 7, character.power);
}

void ControlCharacter() {
	bool move = FALSE, attack = FALSE;
	
	if (character.exp[1] >= character.exp[0]) {	//LV up
		character.lv ++; character.hp[0] += 10; character.mp[0] += 5; character.power ++;
		character.exp[1] = 0; character.exp[0] += character.lv * 10;
	}
	
	if (character.tick[0] + 900 < tick) {	//hp, mp
		character.tick[0] = tick;
		character.hp[1] += roundf(character.hp[0] * 0.01);
		character.mp[1] += roundf(character.mp[0] * 0.05);
	}
	if (character.hp[1] > character.hp[0])
		character.hp[1] = character.hp[0];
	if (character.mp[1] > character.mp[0])
		character.mp[1] = character.mp[0];
	
	if (GetAsyncKeyState(0x5A) & 0x8000 && character.flyTime == 0) {	//attack
		attack = TRUE;
		character.motion[1] = TRUE;
	}
	if (character.motion[1]) {
		if (tick > character.tick[2] + 150) {	//attack motion calculation
			character.tick[2] = tick;
			character.motion[2]++;
		}
		
		if (character.motion[2] > 3) {
			if (attack) {
				character.motion[2] = 1; character.motion[3]++;
			} else {
				character.motion[1] = FALSE; character.motion[2] = 0; character.motion[3] = 1;
			}
			
			if (character.motion[3] > 3)
				character.motion[3] = 1;	
		}
	} else {
		if (GetAsyncKeyState(VK_LEFT) & 0x8000 && character.coord[0] > 1) {	//move left
			if (character.accel[0] > -1)
				character.accel[0] = -1;
				
			character.direction = FALSE;
			move = TRUE;
		}
		
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && character.coord[0] < MAP_X_MAX - 2) {	//move right
			if (character.accel[0] < 1)
				character.accel[0] = 1;
				
			character.direction = TRUE;
			move = TRUE;
		}
			
		if (GetAsyncKeyState(0x58) & 0x8000 && character.tick[3] + 1200 <= tick) {	//dash
			character.accel[0] = character.direction * 8 - 4;
			character.tick[3] = tick;
		}
	}
	
	if (GetAsyncKeyState(VK_UP) & 0x8000 && character.coord[1] + 3 == FLOOR_Y)	//jump
			character.accel[1] = -1.75;
	
	MotionControl(character.coord, character.accel, character.size, &character.flyTime);
	
	if (tick > character.tick[1] + 90) {	//leg tick	
		character.tick[1] = tick;
		
		if (move == TRUE)
			character.motion[0]++;
		else
			character.motion[0] = 0;
			 
		if (character.motion[0] > 3)
			character.motion[0] = 1;
	}
	
	DrawSprite(character.coord[0], character.coord[1], character.size[0], character.size[1], sprite_character);	//draw character sprite
	
	if (character.direction) {
		EditMap(character.coord[0], character.coord[1] + 1, '(');
	} else {
		EditMap(character.coord[0] + 2, character.coord[1] + 1, ')');
	}
	
	if (character.accel[0] > 1)
		DrawSprite(character.coord[0] - 3, character.coord[1], 1, 3, "===");
	if (character.accel[0] < -1)
		DrawSprite(character.coord[0] + 5, character.coord[1], 1, 3, "===");
		
	if (character.motion[1] && character.motion[2] > 0) {	//draw attack motion
		if (character.motion[3] == 3) {
			DrawSprite(character.coord[0] - 5 + 8 * character.direction, character.coord[1], 5, 3, sprite_normalAttack[character.direction][character.motion[2] - 1]);
		} else {
			if (character.motion[2] == 2) {
				EditMap(character.coord[0] - 2 + 6 * character.direction, character.coord[1] + 1, 'o');
				DrawSprite(character.coord[0] - 5 + 10 * character.direction, character.coord[1] + 1, 3, 1, sprite_Weapon[character.direction][character.weapon]);
			} else {
				EditMap(character.coord[0] + 2 * character.direction, character.coord[1] + 1, 'o');
				DrawSprite(character.coord[0] - 3 + 6 * character.direction, character.coord[1] + 1, 3, 1, sprite_Weapon[character.direction][character.weapon]);
			}
		}
	} else {
		EditMap(character.coord[0] + character.direction * 2, character.coord[1] + 1, 'o');
		DrawSprite(character.coord[0] - 3 + 6 * character.direction, character.coord[1] + 1, 3, 1, sprite_Weapon[character.direction][character.weapon]);
		
		if (character.motion[0] == 3)
			EditMap(character.coord[0] + 1, character.coord[1]+ 1, 'l');
	}
	
	if (move)
		DrawSprite(character.coord[0], character.coord[1] + 2, 3, 1, sprite_character_leg[character.direction][character.motion[0] - 1]);	//draw leg motion
}

void ControlEnemy() {
	bool move = FALSE, attack = FALSE;
	
}

void CreateObject() {
	int num = 0;
	Object *obj = 0;
	
	while(num < OBJECT_MAX) {
		if (! objects[num - 1]) {
        	obj = (Object *)malloc(sizeof(Object));
    		objects[num - 1] = obj;  

        	return;
    	}
    	
    	num ++;
	}
}

void MotionControl(short coord[], float accel[], short size[], float *flyTime) {
	float x_value = accel[0], y_value = accel[1];
	
	if (coord[1] + size[1] < FLOOR_Y) {
		*flyTime += 0.05;
		accel[1] += *flyTime;
	} else {
		*flyTime = 0;
	}
	
	if (x_value != 0 || y_value != 0) {
		if (coord[0] + x_value < 1)
			x_value = 1 - coord[0];
		if (coord[0] + size[0] + x_value > MAP_X_MAX)
			x_value = MAP_X_MAX - size[0] - coord[0];
		if (coord[1] + size[1] + y_value > FLOOR_Y)
			y_value = FLOOR_Y - coord[1] - size[1];
	}
	
	coord[0] += floor(x_value + 0.5); coord[1] += floor(y_value + 0.5);
	
	if (accel[0] > 0) accel[0] -= 0.5; if (accel[0] < 0) accel[0] += 0.5;
	if (accel[1] > 0) accel[1] -= 0.1; if (accel[1] < 0) accel[1] += 0.1;
}

void DrawBox(short x, short y, short size_x, short size_y) {
	EditMap(x, y, '.'); EditMap(x + size_x - 1, y, '.');
	EditMap(x, y + size_y - 1, '\''); EditMap(x + size_x - 1, y + size_y - 1, '\'');
	
	for (int i = 1; i < size_x - 1; i++) {
		EditMap(x + i, y, '-'); EditMap(x + i, y + size_y - 1, '-');
	}
	
	for (int i =  1; i < size_y - 1; i++) {
		EditMap(x, y + i, '|'); EditMap(x + size_x - 1, y + i, '|');
	}
}

void DrawNumber(short x, short y, int num) {
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

void DrawSprite(short x, short y, short size_x, short size_y, char spr[]) {
	for (int i = 0; i < size_y; i++) {
		for (int n = 0; n < size_x; n++)
			EditMap(x + n, y + i, spr[i * size_x + n]);
	}
}

void FillMap(char str[], char str_s, int max_value) {
	for (int i = 0; i < max_value; i++)
		str[i] = str_s;
}

void EditMap(short x, short y, char str) {
	if (x > 0 && y > 0 && x - 1 < MAP_X_MAX && y - 1 < MAP_Y_MAX)
		mapData[(y - 1) * MAP_X_MAX + x - 1] = str;
}

int NumLen(int num) {
	int tmp = num;
	short len = 0;
	
	if (num == 0) {
		return 1;
	} else {
		while(tmp != 0) {
        	tmp /= 10;
        	len++;
    	}
	}
	
    return len;
}

int StrLen(char str[]) {
	short len = 0;
	
	for(int i = 0; str[i]; i++)
        len ++;
    
    return len;
}
