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
#define SPAWN_TIME 15000

typedef struct _Character {
    short coord[2], size[2];	//coordinate value and size
    float accel[2], flyTime;	//acceleration value and flotation time
    bool direction;	//true=right, false=left
    			//stat
    char name[16];
    int lv, exp[2], score, hp[2], mp[2];	//0=exp required, 1=current exp// 0=max value, 1=current value
    short power, weapon;
    			//animation control
    short motion[4];	//motion value		//leg_motion, attack_motion(1, 2, 3), invincibility motion
    unsigned int tick[5];	//tick 		//gen_tick, leg_tick, atk_tick, dash_tick, invincibility tick
}Character;

typedef struct _Object {	//enemies, projectiles, particles, etc.
    short coord[2], size[2];
    float accel[2], flyTime;
    bool direction;

    short kind;	//1~99: items, 100~199: enemies, 200~: projectiles, particles
    int hp[2], exp;	//hp: this value is used randomly for item or particle object
    short dam;
    
    short motion[3];	//motion
    unsigned int tick[4];	//0: hpshow time(enemy) or active time(projecticles, particles)
}Object;

Character character = {{MAP_X_MAX / 2, MAP_Y_MAX / 2}, {3, 3}, {0, 0}, 0, 1, "", 1, {100, 0}, 0, {100, 100}, {50, 50}, 10, 0, {0, 1, 0, 0}, {0, 0, 0, 0, 0}};
Object **objects;

unsigned int tick = 0;
unsigned int spon_tick = 0;
char sprite_floor[MAP_X_MAX];
char mapData[MAP_X_MAX * MAP_Y_MAX];	//array for graphics

const short stat_enemy[2][7] =
 {{150, 30, 4, 3, 0, 1000, 0},
  {300, 50, 5, 5, 0, 500, 0}};	//hp, exp, size(x y), tick (2 3 4)
const short stat_weapon[3] = {5, 10, 15};
const char sprite_character[10] = " 0  | _^_";
const char sprite_character_leg[2][3][4] = 
{{"-^.", "_^\'", "_^."},
 {".^-", "\'^_", ".^_"}};
const char sprite_normalAttack[2][3][16] = 
{{" .- o          ", " .   (   o \'   ", "         o \'-  "},
 {"o -.           ", "   . o   )   \' ", "     o      -\' "}};
const char sprite_weapon[2][3][4] = 
{{"---", "--+", "<=+"},
 {"---", "+--", "+=>"}};
const char sprite_invenWeapon[3][11] = {"   /   /  ", "   /  '*. ", "  |   \"+\" "};  
const char sprite_enemy1[2][13] = {" __ (  )----", " __ [  ]\'--\'"};

void StartGame();	//=initialize
void UpdateGame();
void ExitGame();
void SetConsole();
void ControlUI();
void ControlCharacter();
void ControlObject();
void ControlItem(int index);
void ControlEnemy(int index);
void ControlParticle(int index);
void CreateObject(short x, short y, short kind);
void RemoveObject(int index);
bool EnemyPosition(short x, short size_x);	//direction the enemy looks at the character
bool CollisionCheck(short coord1[], short coord2[], short size1[], short size2[]);	//check collision
void MoveControl(short coord[], float accel[], short size[], float *flyTime);	// motion control
void DrawBox(short x, short y, short size_x, short size_y);	//draw box of size_x, size_y at x, y coordinates
void DrawNumber(short x, short y, int num);	//draw numbers at x, y coordinates (align left)
void DrawSprite(short x, short y, short size_x, short size_y, const char spr[]);	//draw sprite of size_x, size_y at x, y coordinates
void FillMap(char str[], char str_s, int max_value);	//array initialization
void EditMap(short x, short y, char str);	// edit x, y coordinate mapdata
int NumLen(int num);	//return length of number

int main() {
	StartGame();
	
	while (TRUE) {
		if (tick + 30 < GetTickCount()) {
			tick = GetTickCount();
			
			UpdateGame();
			
			if (tick == 0)
				break;
		}
	}
	
	ExitGame();
	return 0;
}

void StartGame() {
	SetConsole();
	srand((unsigned int)time(NULL));
	
	printf("Enter your name: ");
	scanf("%[^\n]s", character.name);
	
	FillMap(sprite_floor, '=', MAP_X_MAX);
	
	objects = (Object **)malloc(sizeof(Object *) * OBJECT_MAX);
	memset(objects, 0, sizeof(Object *) * OBJECT_MAX);
}

void UpdateGame() {
	FillMap(mapData, ' ', MAP_X_MAX * MAP_Y_MAX);	//initialize mapData
	
	ControlCharacter();	//update mapData(character)
	ControlObject();	//update mapData(enemy, projecticles, particles, etc...)
	ControlUI();	//update mapData(UI)
	
	if (spon_tick + SPAWN_TIME < tick) {
		spon_tick = tick;
		CreateObject(rand() % 90, 10, 100);
		CreateObject(rand() % 90, 10, 100);
		CreateObject(rand() % 90, 10, 100);
	}
	
	puts(mapData);	//draw mapData
}

void ExitGame() {
    for (int i = 0; i < OBJECT_MAX; i++) {
        if (objects[i])
            free(objects[i]);
    }
    
    free(objects);
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
	int expPer = roundf(character.exp[1] * 100 / character.exp[0]);
	int len;	//length of previous sprite
	
	DrawSprite(1, FLOOR_Y, MAP_X_MAX, 1, sprite_floor);	//draw floor
	
	DrawBox(1, 2, 35, 8); DrawBox(27, 5, 7, 4);	//draw weaponinven
	DrawSprite(28, 6, 5, 2, sprite_invenWeapon[character.weapon]);
	DrawSprite(28, 4, 6, 1, "Weapon");
	
	EditMap(3, 3, '\"');	//draw name, lv, exp
	DrawSprite(4, 3, strlen(character.name), 1, character.name);	len = 4 + strlen(character.name);
	DrawSprite(len, 3, 7, 1, "\" LV.");	len += 5;
	DrawNumber(len, 3, character.lv);	len += NumLen(character.lv);
	DrawSprite(len, 3, 2, 1, " (");	len += 2;
	if (!expPer) {
		EditMap(len, 3, '0');	len ++;
	} else {
		DrawNumber(len, 3, expPer);	len += NumLen(expPer);
	}
	DrawSprite(len, 3, 2, 1, "%)");
	
	DrawSprite(MAP_X_MAX - NumLen(character.score) - 7, 3, 6, 1, "SCORE:");	//draw score
	DrawNumber(MAP_X_MAX - NumLen(character.score), 3, character.score);
	
	DrawSprite(4, 5, 3, 1, "HP:");	//draw HP
	DrawNumber(8, 5, character.hp[1]);
	EditMap(9 + NumLen(character.hp[1]), 5, '/');
	DrawNumber(11 + NumLen(character.hp[1]), 5, character.hp[0]);
	
	DrawSprite(4, 6, 3, 1, "MP:");	//draw MP
	DrawNumber(8, 6, character.mp[1]);
	EditMap(9 + NumLen(character.mp[1]), 6, '/');
	DrawNumber(11 + NumLen(character.mp[1]), 6, character.mp[0]);
	
	DrawSprite(4, 8, 6, 1, "Power:");	//draw power
	DrawNumber(11, 8, character.power);
}

void ControlCharacter() {
	bool move = FALSE, attack = FALSE;
	int x = character.coord[0], y = character.coord[1];
	
	if (character.exp[1] >= character.exp[0]) {	//LV up
		character.hp[0] += 10; character.mp[0] += 5; character.power ++;
		character.lv ++; character.exp[1] = 0; character.exp[0] += character.lv * 10;
	}
	
	if (character.tick[0] + 900 < tick) {	//hp,mp gen & control
		character.tick[0] = tick;
		character.hp[1] += roundf(character.hp[0] * 0.01);
		character.mp[1] += roundf(character.mp[0] * 0.05);
	}
	if (character.hp[1] > character.hp[0])
		character.hp[1] = character.hp[0];
	if (character.mp[1] > character.mp[0])
		character.mp[1] = character.mp[0];
	
	if (character.hp[1] < 1)
		tick = 0;
	
	if (character.tick[5] > 0)
		character.tick[5] -= 1;
	
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
		if (GetAsyncKeyState(VK_LEFT) & 0x8000 && x > 1) {	//move left
			if (character.accel[0] > -1)
				character.accel[0] = -1;
				
			character.direction = FALSE;
			move = TRUE;
		}
		
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && x < MAP_X_MAX - 2) {	//move right
			if (character.accel[0] < 1)
				character.accel[0] = 1;
				
			character.direction = TRUE;
			move = TRUE;
		}
			
		if (GetAsyncKeyState(0x58) & 0x8000 && character.tick[3] + 1200 <= tick) {	//dash
			character.accel[0] = character.direction * 6 - 3;
			character.tick[3] = tick;
		}
	}
	
	if (GetAsyncKeyState(VK_UP) & 0x8000 && y + 3 == FLOOR_Y)	//jump
			character.accel[1] = -1.75;
	
	if (tick > character.tick[1] + 90) {	//leg tick	
		character.tick[1] = tick;
		
		if (move == TRUE)
			character.motion[0]++;
		else
			character.motion[0] = 0;
			 
		if (character.motion[0] > 3)
			character.motion[0] = 1;
	}
	
	MoveControl(character.coord, character.accel, character.size, &character.flyTime);	// control character movement
	
	if (character.tick[5] % 2 == 0) {
		DrawSprite(x, y, character.size[0], character.size[1], sprite_character);	//draw character sprite
		
		if (character.direction) {
			EditMap(x, y + 1, '(');
		} else {
			EditMap(x + 2, y + 1, ')');
		}
		
		if (character.accel[0] > 1)
			DrawSprite(x - 2, y, 1, 3, "===");
		if (character.accel[0] < -1)
			DrawSprite(x + 4, y, 1, 3, "===");
			
		if (character.motion[1] && character.motion[2] > 0) {	//draw attack motion
			if (character.motion[3] == 3) {
				DrawSprite(x - 5 + 8 * character.direction, y, 5, 3, sprite_normalAttack[character.direction][character.motion[2] - 1]);
			} else {
				if (character.motion[2] == 2) {
					EditMap(x - 2 + 6 * character.direction, y + 1, 'o');
					DrawSprite(x - 5 + 10 * character.direction, y + 1, 3, 1, sprite_weapon[character.direction][character.weapon]);
				} else {
					EditMap(x + 2 * character.direction, y + 1, 'o');
					DrawSprite(x - 3 + 6 * character.direction, y + 1, 3, 1, sprite_weapon[character.direction][character.weapon]);
				}
			}
		} else {
			EditMap(x + character.direction * 2, y + 1, 'o');
			DrawSprite(x - 3 + 6 * character.direction, y + 1, 3, 1, sprite_weapon[character.direction][character.weapon]);
			
			if (character.motion[0] == 3)
				EditMap(x + 1, y + 1, 'l');
		}
		
		if (character.motion[0] > 0)
			DrawSprite(x, y + 2, 3, 1, sprite_character_leg[character.direction][character.motion[0] - 1]);	//draw leg motion
	}		
}

void ControlItem(int index) {
	short x = objects[index]->coord[0], y = objects[index]->coord[1];
	short item_coord[2] = {x, y - 2};
	short item_size[2] = {5, 2};
	
	if (objects[index]->tick[1] < tick) {
		objects[index]->tick[1] = tick * 2;
		objects[index]->accel[0] =  1 - 2 * objects[index]->hp[0] / (float)RAND_MAX;
		objects[index]->accel[1] =  - 2 * objects[index]->hp[1] / (float)RAND_MAX;
	}
	
	if (CollisionCheck(item_coord, character.coord, item_size, character.size)) {
		DrawSprite(x + 1, y - 5, 3, 1, "[E]");
		
		if (GetAsyncKeyState(0x45) & 0x8000) {
			character.weapon = objects[index]->kind;
		
			RemoveObject(index);
			return;
		}	
	}
	
	DrawSprite(x, y - 2, 5, 2, sprite_invenWeapon[objects[index]->kind]);

	
	MoveControl(objects[index]->coord, objects[index]->accel, objects[index]->size, &objects[index]->flyTime);
}

void ControlEnemy(int index) {
	short x = objects[index]->coord[0], y = objects[index]->coord[1];
	short at_coord[2] = {character.coord[0] - 5 + 8 * character.direction, character.coord[1]}, at_size[2] = {5, 3};
	short item_code = rand() % 100;
	
	if (objects[index]->hp[1] < 1) {
		for (int i = 0; i < 3; i++)
			CreateObject(x + objects[index]->size[0] / 2, y + objects[index]->size[1] / 2, objects[index]->kind + 100);
			
		if (item_code >= 90)
			CreateObject(x + objects[index]->size[0] / 2 - 2, y, 1);
			
		if (item_code <= 3)
			CreateObject(x + objects[index]->size[0] / 2 - 2, y, 2);
			
		character.exp[1] += objects[index]->exp;
		
		RemoveObject(index);
		return;
	}
	
	if (objects[index]->tick[0] + 2000 > tick)
		DrawNumber(x + objects[index]->size[0] / 2 - NumLen(objects[index]->hp[1]) / 2, y - 1, objects[index]->hp[1]);
	
	if (character.motion[2] == 1 && CollisionCheck(objects[index]->coord, at_coord, objects[index]->size, at_size)) {
		objects[index]->tick[0] = tick;
		objects[index]->hp[1] -= character.power;
		objects[index]->accel[1] = - 0.55;
		
		if (character.motion[3] == 3)
			objects[index]->hp[1] -= character.power;
		
		if (EnemyPosition(x,  objects[index]->size[0]))
			objects[index]->accel[0] = -0.75;
		else
			objects[index]->accel[0] = 0.75;
	}
	
	if (objects[index]->kind == 100) {
		if (y + objects[index]->size[1] == FLOOR_Y)
			objects[index]->motion[0] = 0;
		else 
			objects[index]->motion[0] = 1;

		if (objects[index]->tick[1] + objects[index]->tick[2] < tick) {
			objects[index]->tick[1] = tick;
			objects[index]->tick[2] = 1000 + rand() % 1000;
			objects[index]->accel[1] = rand() / (float)RAND_MAX / 2 - 1.2;
			
			if (EnemyPosition(x,  objects[index]->size[0]))
				objects[index]->accel[0] = 2.4 - rand() / (float)RAND_MAX;
			else
				objects[index]->accel[0] = rand() / (float)RAND_MAX - 2.4;
		}
		
		if (character.tick[5] == 0 && CollisionCheck(objects[index]->coord,character.coord, objects[index]->size, character.size)) {
			character.tick[5] = 100;
			character.hp[1] -= 10;
		}
		
		DrawSprite(x, y, objects[index]->size[0], objects[index]->size[1], sprite_enemy1[objects[index]->motion[0]]);
	}
	
	MoveControl(objects[index]->coord, objects[index]->accel, objects[index]->size, &objects[index]->flyTime);
}

void ControlParticle(int index) {
	short x = objects[index]->coord[0], y = objects[index]->coord[1];
	short money_size[2] = {2, 2};
	short money_coord[2] = {x, y - 1};
	
	if (objects[index]->kind == 200) {
		if (objects[index]->tick[1] < tick) {
			objects[index]->tick[1] = tick * 2;
			objects[index]->accel[0] =  2 - 4 * objects[index]->hp[0] / (float)RAND_MAX;
			objects[index]->accel[1] =  - 2 * objects[index]->hp[1] / (float)RAND_MAX;
		}
		
		if (CollisionCheck(money_coord, character.coord, money_size, character.size)) {
			character.score += 100;
			
			RemoveObject(index);
			return;
		}
		
		EditMap(x, y - 1, '@');
	}
	
	MoveControl(objects[index]->coord, objects[index]->accel, objects[index]->size, &objects[index]->flyTime);
}

void ControlObject() {
	for(int i = 0; i < OBJECT_MAX; i++) {
		if (objects[i]) {
			if (objects[i]->kind < 100)
				ControlItem(i);
			else if (objects[i]->kind > 99 && objects[i]->kind < 200)
				ControlEnemy(i);
			else
				ControlParticle(i);
		}
	}
}

void CreateObject(short x, short y, short kind) {
	int index = 0;
	Object *obj = 0;
	
	while(TRUE) {
		if (! objects[index])
			break;
			
		if (index == OBJECT_MAX)
			return;

    	index ++;
	}
	
	obj = (Object *)malloc(sizeof(Object));
    objects[index] = obj;
    memset(obj, 0, sizeof(Object));
    
    obj->kind = kind;
    obj->coord[0] = x; obj->coord[1] = y;
    obj->tick[0] = 0;
    
    if (kind < 100 || kind > 199) {
    	obj->hp[0] = rand();
		obj->hp[1] = rand();
		obj->tick[1] = 0;
    	obj->tick[2] = 0;
    	obj->tick[3] = 0;
	}
	
    if (kind > 99 && kind < 200) {
    	obj->hp[0] = stat_enemy[kind - 100][0];
		obj->hp[1] = obj->hp[0];
    	obj->exp = stat_enemy[kind - 100][1];
    	obj->size[0] = stat_enemy[kind - 100][2];
		obj->size[1] = stat_enemy[kind - 100][3];
    	obj->tick[1] = stat_enemy[kind - 100][4];
    	obj->tick[2] = stat_enemy[kind - 100][5];
    	obj->tick[3] = stat_enemy[kind - 100][6];
	}
}

void RemoveObject(int index) {
	free(objects[index]);
    objects[index] = 0;
}

bool CollisionCheck(short coord1[], short coord2[], short size1[], short size2[]) {
	if (coord1[0] > coord2[0] - size1[0] && coord1[0] < coord2[0] + size2[0]
	 && coord1[1] > coord2[1] - size1[1] && coord1[1] < coord2[1] + size2[1])
		return TRUE;
	else
		return FALSE;
}

bool EnemyPosition(short x, short size_x) {
	if (character.coord[0] + 1 < x + floor(size_x / 2 + 0.5))
		return FALSE;
	else
		return TRUE;
}

void MoveControl(short coord[], float accel[], short size[], float *flyTime) {
	float x_value = accel[0], y_value = accel[1];
	
	if (coord[1] + size[1] == FLOOR_Y) {
		*flyTime = 0;
	} else {
		*flyTime += 0.05;
		accel[1] += *flyTime;
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
	
	if (accel[0] > 0) accel[0] -= 0.2; if (accel[0] < 0) accel[0] += 0.2;
	if (accel[1] > 0) accel[1] -= 0.1; if (accel[1] < 0) accel[1] += 0.1;
}

void DrawBox(short x, short y, short size_x, short size_y) {
	EditMap(x, y, '.'); EditMap(x + size_x - 1, y, '.');
	EditMap(x, y + size_y - 1, '\''); EditMap(x + size_x - 1, y + size_y - 1, '\'');
	
	for (int i = 1; i < size_x - 1; i++) {
		EditMap(x + i, y, '-'); EditMap(x + i, y + size_y - 1, '-');
	}
	for (int i = 1; i < size_y - 1; i++) {
		EditMap(x, y + i, '|'); EditMap(x + size_x - 1, y + i, '|');
	}
}

void DrawNumber(short x, short y, int num) {
	int tmp = num, len = NumLen(tmp), cnt = len;
    char str[len];
    
    do {
        cnt--;
        str[cnt] = (char)(tmp % 10 + 48);
        tmp /= 10;
    } while(tmp != 0);
    
    DrawSprite(x, y, len, 1, str);
}

void DrawSprite(short x, short y, short size_x, short size_y, const char spr[]) {
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
	int tmp = num, len = 0;
	
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
