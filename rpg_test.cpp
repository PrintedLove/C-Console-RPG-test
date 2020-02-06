// [C Game] Simple RPG
// made by "PrintedLove"
// https://printed.tistory.com/
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#define FALSE 0
#define TRUE 1
#define MAP_X_MAX 96
#define MAP_Y_MAX 32

typedef struct _Character {
    short x = MAP_X_MAX / 2 + 1, y = 19;
    short size_x = 3, size_y = 3;
    char sprite[10] = " 0 (|)_^_";
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

Character character;

void SetConsole();
void ControlUI();
void ControlCharacter();

void FillMap(char str[], char str_s, int max_value);
void EditMap(int x, int y, char str);
void DrawSprite(int x, int y, int size_x, int size_y, char spr[]);
void DrawNumber(int x, int y, int num);
void DrawBox(int x, int y, int size_x, int size_y);

int NumLen(int num);
int StrLen(char str[]);

char wall_floor[MAP_X_MAX];
char sprite_invenWeapon[][11] = {"   /   /  ", "  |   \"+\" ", "   /  '+. "};
char sprite_rightWeapon[][4] = {"---", "+--", "+=>"};
char sprite_leftWeapon[][4] = {"---", "--+", "<=+"};
short weapon_stat[] = {5, 10, 15};
char mapData[MAP_X_MAX * MAP_Y_MAX];

int main() {
	srand((unsigned int)time(NULL));
	SetConsole();
	
	printf("Enter your name: ");
	scanf("%[^\n]s", character.name);
	
	FillMap(wall_floor, '=', MAP_X_MAX);
	
	unsigned int system_tick = GetTickCount();
	
	while (TRUE) {
		if (system_tick + 30 < GetTickCount()) {
			system_tick = GetTickCount();
			
			FillMap(mapData, ' ', MAP_X_MAX * MAP_Y_MAX);
		
			ControlCharacter();
			ControlUI();
			
			printf("%s", mapData);				//update mapdata
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
}

void ControlUI() {
	int len;	//length of previous sprite
	DrawSprite(1, 22, MAP_X_MAX, 1, wall_floor);	//draw floor
	
	DrawBox(1, 1, 35, 8); DrawBox(27, 4, 7, 4);
	DrawSprite(28, 5, 5, 2, sprite_invenWeapon[character.weapon]);	//draw weaponinven
	
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

void ControlCharacter() {
	unsigned int tick = GetTickCount();
	bool move = FALSE, attack = FALSE;
				//LV up
	if (character.exp >= character.expi) {
		character.exp = 0;
		character.lv ++; character.hpm += 10; character.mpm += 5; character.power ++;
		character.expi += character.lv * 10;
	}
				//hp, mp
	if (character.gen_tick + 1000 < tick) {
		character.gen_tick = tick;
		character.hp += roundf(character.hpm * 0.01);
		character.mp += roundf(character.mpm * 0.05);
	}
	if (character.hp > character.hpm)
		character.hp = character.hpm;
	if (character.mp > character.mpm)
		character.mp = character.mpm;
				//keyboard
	if (GetAsyncKeyState(0x5A) & 0x8000) {
		attack = TRUE;
		character.atk_m[0] = TRUE;
	}
	if (character.atk_m[0]) {
		if (tick > character.atk_tick + 150) {	//attack
			character.atk_tick = tick;
			character.atk_m[1]++;
		}
		
		if (character.atk_m[1] > 3) {
			if (attack) {
				character.atk_m[1] = 1;
				character.atk_m[2]++;	
			} else {
				character.atk_m[0] = FALSE;
				character.atk_m[1] = 0;
				character.atk_m[2] = 1;
			}
			
			if (character.atk_m[2] > 3) {
				character.atk_m[2] = 1;
				if (character.direction) {
					if (character.x < MAP_X_MAX - 2)
						character.x++;
				} else {
					if (character.x > 1)
						character.x--;
				}
			}	
		}
	} else {
		if (GetAsyncKeyState(VK_LEFT) & 0x8000 && character.x > 1) {
			character.x--;
			character.direction = FALSE;
			move = TRUE;
		}
		
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && character.x < MAP_X_MAX - 2) {
			character.x++;
			character.direction = TRUE;
			move = TRUE;
		}
	}
	
	if (GetAsyncKeyState(0x31) & 0x8000)	//1
		character.weapon = 1;
		
	if (GetAsyncKeyState(0x32) & 0x8000)	//2
		character.weapon = 2;
		
	if (GetAsyncKeyState(0x4C) & 0x8000)	//L
		character.exp += 25;
	
	if (tick > character.leg_tick + 90) {	//leg tick	
		character.leg_tick = tick;
		
		if (move == TRUE)
			character.leg_m++;
		else
			character.leg_m = 0;
			 
		if (character.leg_m > 3)
			character.leg_m = 1;
	}
				//sprite
	character.sprite[4] = '|'; character.sprite[6] = '_'; character.sprite[8] = '_';
	
	if (character.direction) {
		character.sprite[3] = '(';
		
		if (character.atk_m[0]) {
			switch (character.atk_m[2]) {
				case 1:
					character.sprite[5] = ' ';
					switch (character.atk_m[1]) {
						case 1:
							character.sprite[6] = '_'; character.sprite[8] = '_';
							EditMap(character.x + 3, character.y, 'o');
							break;
						case 2:
							character.leg_m = 1;
							EditMap(character.x + 3, character.y + 1, 'o');
							EditMap(character.x + 5, character.y, '.');
							EditMap(character.x + 6, character.y + 1, ')');
							EditMap(character.x + 5, character.y + 2, '\'');
							break;
						case 3:
							character.sprite[6] = '_'; character.sprite[8] = '_';
							break;
						default:
							break;
					} break;
				case 2:
					switch (character.atk_m[1]) {
						case 1:
							character.sprite[5] = 'o'; character.sprite[6] = '_'; character.sprite[8] = '_';
							DrawSprite(character.x + 3, character.y + 1, 3, 1, sprite_rightWeapon[character.weapon]);
							break;
						case 2:
							character.sprite[5] = ' ';
							character.leg_m = 1;
							EditMap(character.x + 4, character.y + 1, 'o');
							DrawSprite(character.x + 5, character.y + 1, 3, 1, sprite_rightWeapon[character.weapon]);
							break;
						case 3:
							character.sprite[5] = 'o'; character.sprite[6] = '_'; character.sprite[8] = '_';
							DrawSprite(character.x + 3, character.y + 1, 3, 1, sprite_rightWeapon[character.weapon]);
							break;
						default:
							break;
					} break;
				case 3:
					character.sprite[5] = ' ';
					switch (character.atk_m[1]) {
						case 1:
							character.leg_m = 1;
							EditMap(character.x + 3, character.y + 2, 'o');
							DrawSprite(character.x + 4, character.y + 2, 3, 1, sprite_rightWeapon[character.weapon]);
							DrawSprite(character.x + 4, character.y, 3, 1, sprite_rightWeapon[character.weapon]);
							
							break;
						case 2:
							character.leg_m = 1;
							EditMap(character.x + 4, character.y + 1, 'o');
							DrawSprite(character.x + 5, character.y + 1, 3, 1, sprite_rightWeapon[character.weapon]);
							break;
						case 3:
							character.sprite[6] = '_'; character.sprite[8] = '_';
							EditMap(character.x + 3, character.y, 'o');
							DrawSprite(character.x + 4, character.y + 2, 3, 1, sprite_rightWeapon[character.weapon]);
							DrawSprite(character.x + 4, character.y, 3, 1, sprite_rightWeapon[character.weapon]);
							break;
						default:
							break;
					} break;
				default:
					break;
			}
		} else {
			character.sprite[5] = 'o';
			DrawSprite(character.x + 3, character.y + 1, 3, 1, sprite_rightWeapon[character.weapon]);
		}
		
		switch (character.leg_m) {
			case 1:
				character.sprite[4] = 'l'; character.sprite[6] = '.'; character.sprite[8] = '-';
				break;
			case 2:
				character.sprite[6] = '\''; character.sprite[8] = '_';
				break;
			case 3:
				character.sprite[6] = '.'; character.sprite[8] = '_';
				break;
			default:
				break;
		}
	} else {
		character.sprite[5] = ')';
		
		if (character.atk_m[0]) {
			switch (character.atk_m[2]) {
				case 1:
					character.sprite[3] = ' ';
					switch (character.atk_m[1]) {
						case 1:
							character.sprite[6] = '_'; character.sprite[8] = '_';
							EditMap(character.x - 1, character.y, 'o');
							break;
						case 2:
							character.leg_m = 1;
							EditMap(character.x - 1, character.y + 1, 'o');
							EditMap(character.x - 3, character.y, '.');
							EditMap(character.x - 4, character.y + 1, '(');
							EditMap(character.x - 3, character.y + 2, '\'');
							break;
						case 3:
							character.sprite[6] = '_'; character.sprite[8] = '_';
							break;
						default:
							break;
					} break;
				case 2:
					switch (character.atk_m[1]) {
						case 1:
							character.sprite[3] = 'o'; character.sprite[6] = '_'; character.sprite[8] = '_';
							DrawSprite(character.x - 3, character.y + 1, 3, 1, sprite_leftWeapon[character.weapon]);
							break;
						case 2:
							character.sprite[3] = ' ';
							character.leg_m = 1;
							EditMap(character.x - 2, character.y + 1, 'o');
							DrawSprite(character.x - 5, character.y + 1, 3, 1, sprite_leftWeapon[character.weapon]);
							break;
						case 3:
							character.sprite[3] = 'o'; character.sprite[6] = '_'; character.sprite[8] = '_';
							DrawSprite(character.x - 3, character.y + 1, 3, 1, sprite_leftWeapon[character.weapon]);
							break;
						default:
							break;
					} break;
				case 3:
					character.sprite[3] = ' ';
					switch (character.atk_m[1]) {
						case 1:
							character.leg_m = 1;
							EditMap(character.x - 1, character.y + 2, 'o');
							DrawSprite(character.x - 4, character.y + 2, 3, 1, sprite_leftWeapon[character.weapon]);
							DrawSprite(character.x - 4, character.y, 3, 1, sprite_leftWeapon[character.weapon]);
							
							break;
						case 2:
							character.leg_m = 1;
							EditMap(character.x - 2, character.y + 1, 'o');
							DrawSprite(character.x - 5, character.y + 1, 3, 1, sprite_leftWeapon[character.weapon]);
							break;
						case 3:
							character.sprite[6] = '_'; character.sprite[8] = '_';
							EditMap(character.x - 1, character.y, 'o');
							DrawSprite(character.x - 4, character.y + 2, 3, 1, sprite_leftWeapon[character.weapon]);
							DrawSprite(character.x - 4, character.y, 3, 1, sprite_leftWeapon[character.weapon]);
							break;
						default:
							break;
					} break;
				default:
					break;
			}
		} else {
			character.sprite[3] = 'o'; 
			DrawSprite(character.x - 3, character.y + 1, 3, 1, sprite_leftWeapon[character.weapon]);
		}
		
		switch (character.leg_m) {
			case 1:
				character.sprite[4] = 'l'; character.sprite[6] = '-'; character.sprite[8] = '.';
				break;
			case 2:
				character.sprite[6] = '_'; character.sprite[8] = '\'';
				break;
			case 3:
				character.sprite[6] = '_'; character.sprite[8] = '.';
				break;
			default:
				break;
		}
	}
	DrawSprite(character.x, character.y, character.size_x, character.size_y, character.sprite);
}

void FillMap(char str[], char str_s, int max_value) {
	for (int i = 0; i < max_value; i++)
		str[i] = str_s;
}

void EditMap(int x, int y, char str) {
	if (x > 0 && y > 0 && x - 1 < MAP_X_MAX && y - 1 < MAP_Y_MAX)
		mapData[(y - 1) * MAP_X_MAX + x - 1] = str;
}

void DrawSprite(int x, int y, int size_x, int size_y, char spr[]) {
	for (int i = 0; i < size_y; i++) {
		for (int n = 0; n < size_x; n++)
			EditMap(x + n, y + i, spr[i * size_x + n]);
	}
}

void DrawNumber(int x, int y, int num) {
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

void DrawBox(int x, int y, int size_x, int size_y) {
	EditMap(x, y, '.'); EditMap(x + size_x - 1, y, '.');
	EditMap(x, y + size_y - 1, '\''); EditMap(x + size_x - 1, y + size_y - 1, '\'');
	
	for (int i = 1; i < size_x - 1; i++) {
		EditMap(x + i, y, '-'); EditMap(x + i, y + size_y - 1, '-');
	}
	
	for (int i =  1; i < size_y - 1; i++) {
		EditMap(x, y + i, '|'); EditMap(x + size_x - 1, y + i, '|');
	}
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
