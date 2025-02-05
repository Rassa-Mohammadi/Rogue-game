#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define MAX_SIZE 150

struct Point {
    int x;
    int y;
};

struct Door {
    int exist;
    struct Point pos;
    int is_opened;
    int is_old;
    int has_password;
    char password[MAX_SIZE];
};

struct Trap {
    int exist;
    int damage;
};

struct Gold {
    char type; // n: normal, b: balck
    int cnt; // value
};

struct Room {
    struct Point p;
    int height, width;
    char type; // r: regular, e: enchanted, t: treasure, n: nightmare
};

struct Bag {
    int number_of_food;
    char food[5];
    time_t production_date[5];
    int weapon[5];
    // potions
    int health_potion;
    int speed_potion;
    int damage_potion;
};

struct Enemy {
    int health;
    int moves;
};

struct User {
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    char email[MAX_SIZE];
    int number_of_games;
    time_t first_game;
    int number_of_floor;
    int level;
    char **map[5];
    int mask[5][MAX_SIZE][MAX_SIZE];
    int theme[5][MAX_SIZE][MAX_SIZE]; // r: regular, t: treasure, e: enchant, n: nightmare | 1: normal 2: aala 3: jadoee 4: fased
    struct Point pos;
    struct Bag bag;
    int cur_weapon;
    int score;
    int golds;
    int health;
    int hunger;
    struct Gold gold[5][MAX_SIZE][MAX_SIZE];
    struct Door door[5][MAX_SIZE][MAX_SIZE];
    struct Trap trap[5][MAX_SIZE][MAX_SIZE];
    struct Enemy enemy[5][MAX_SIZE][MAX_SIZE];
    int info[5][MAX_SIZE][MAX_SIZE];
};

struct miniUser {
    char username[MAX_SIZE];
    int number_of_games;
    time_t first_game;
    int gold;
    int score;
};

time_t LAST_MESSAGE_REFRESH[3], LAST_EAT, LAST_RECOVERY;
int USERS;
int DELAY = 40000, RECOVERY = 2, POWER = 1;
int DIFFICULTY = 0; // 0: Easy, 1: Medium, 2: HARD
int GAME_X = 30, GAME_Y = 120;
int ST_X = 3, ST_Y = 4;
// up, right, down, left, topleft, topright, bottomright, bottomleft
int D_X[] = {-1, 0, 1, 0, -1, -1, 1, 1};
int D_Y[] = {0, 1, 0, -1, -1, 1, 1, -1};
int number_of_rooms;
struct Point* corners;
char WEAPON[][20] = {"Mace", "Dagger", "Magic Wand", "Normal Arrow", "Sword"};
char wEAPON[] = {'m', 'a', 'M', 'n', 'o'};
int DAMAGE[] = {5, 12, 15, 5, 10};

struct Point create_point(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

int are_equal(struct Point p1, struct Point p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

int get_dist(struct Point p1, struct Point p2) {
    int delta_x = p1.x - p2.x, delta_y = p1.y - p2.y;
    if (delta_x < 0)
        delta_x *= -1;
    if (delta_y < 0)
        delta_y *= -1;
    return delta_x * delta_x + delta_y * delta_y;
}

void change_color(int color) {
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(color));
}

void undo_color(int color) {
    attroff(COLOR_PAIR(color));
    attron(COLOR_PAIR(1));
}

void print_message_with_color(int x, int y, char message[], int color) {
    change_color(color);
    mvprintw(x, y, "%s", message);
    undo_color(color);
}

void print_number_with_color(int x, int y, int n, int color) {
    change_color(color);
    mvprintw(x, y, "%d", n);
    undo_color(color);
}

void print_character_with_color(int x, int y, char c, int color) {
    change_color(color);
    mvprintw(x, y, "%c", c);
    undo_color(color);
}

void init_user(struct User* user, int level) {
    user->map[level] = malloc(sizeof(char*) * GAME_X);
    for (int i = 0; i < GAME_X; i++)
        (user->map)[level][i] = malloc(sizeof(char) * GAME_Y);
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            (user->mask)[level][i][j] = 0;
            (user->theme)[level][i][j] = 'r';
        }
    // user->golds = 0;
    user->bag.number_of_food = 0;
    user->bag.weapon[0] = 1;
    for (int i = 1; i < 5; i++)
        user->bag.weapon[i] = 0;
    user->cur_weapon = 0;
    user->bag.speed_potion = user->bag.health_potion = user->bag.damage_potion = 0;
    user->health = 10;
    user->hunger = 10;
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            (user->gold)[level][i][j].type = 'N'; // Not available
            (user->door)[level][i][j].exist = (user->door)[level][i][j].has_password = 0;
            (user->trap)[level][i][j].exist = 0;
            (user->theme)[level][i][j] = '.';
        }
}

struct Point next_point(struct Point p, int dir) {
    p.x += D_X[dir];
    p.y += D_Y[dir];
    return p;
}

void trim(char s[]) {
    s[strlen(s) - 1] = '\0';
}

int get_num(char s[]) {
    int len = strlen(s), res = 0;
    for (int i = 0; i < len; i++)
        res = res * 10 + s[i] - '0';
    return res;
}

char* get_str(int n) {
    char* res = malloc(sizeof(char) * 20);
    int len = 0;
    while (n > 0) {
        res[len++] = '0' + n % 10;
        n /= 10;
    }
    for (int l = 0, r = len - 1; l < r; l++, r--) {
        char tmp = res[l];
        res[l] = res[r];
        res[r] = tmp;
    }
    if (len == 0)
        res[len++] = '0';
    res[len] = '\0';
    return res;
}

int is_in_map(struct Point p) {
    return p.x >= 0 && p.y >= 0 && p.x < GAME_X && p.y < GAME_Y;
}

int is_in_corridor(char c) {
    return c == '+' || c == '#' || c == '?';
}

int is_door(char c) {
    return c == '+' || c == '?';
}

int is_corner(struct Point p) {
    for (int i = 0; i < 4 * number_of_rooms; i++)
        if (p.x == corners[i].x && p.y == corners[i].y)
            return 1;
    return 0;
}

int is_food(struct User* user) {
    return user->map[user->level][user->pos.x][user->pos.y] == 'f';
}

int is_wall(char c) {
    return c == '|' || c == '_';
}

int is_weapon(char c) {
    return c == 'm' || c == 'a' || c == 'M' || c == 'n' || c == 'o'; // mace dagger Magic arrow sword
}

int is_potion(char c) {
    return c == 'h' || c == 's' || c == 'd'; 
}

int is_enemy(char c) {
    return c == 'D' || c == 'F' || c == 'G' || c == 'S' || c == 'U';
}

int is_in_room(char ***map, struct Point p) {
    if (!is_in_map(p))
        return 0;
    return (*map)[p.x][p.y] == '.' || (*map)[p.x][p.y] == 'O' || (*map)[p.x][p.y] == '<' || (*map)[p.x][p.y] == '>' || (*map)[p.x][p.y] == '^' || (*map)[p.x][p.y] == 'g' || (*map)[p.x][p.y] == 'f' || is_weapon((*map)[p.x][p.y]) || is_potion((*map)[p.x][p.y]) || is_enemy((*map)[p.x][p.y]) || (*map)[p.x][p.y] == 'T';
}

int is_new_room(struct User* user) {
    for (int dir = 0; dir < 4; dir++) {
        struct Point nxt = next_point(user->pos, dir);
        if (is_in_map(nxt)  && is_in_room(&(user->map[user->level]), nxt))
            return !user->mask[user->level][nxt.x][nxt.y];
    }
    return 0;
}

int not_restricted(struct User* user, char ***map, struct Point p) {
    if (user->door[user->level][p.x][p.y].exist)
        return 1;
    return (*map)[p.x][p.y] != '_' && (*map)[p.x][p.y] != '|' && (*map)[p.x][p.y] != 'O' && (*map)[p.x][p.y] != ' ' && (*map)[p.x][p.y] != '=';
}

int has_enemy(char ***map) {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            if (is_enemy((*map)[i][j]))
                return 1;
    return 0;
}

int get_dir(int key) {
    if (key == KEY_UP || key == 'j')
        return 0;
    else if (key == KEY_RIGHT || key == 'l')
        return 1;
    else if (key == KEY_DOWN || key == 'k')
        return 2;
    else if (key == KEY_LEFT || key == 'h')
        return 3;
    else if (key == 'y')
        return 4;
    else if (key == 'u')
        return 5;
    else if (key == 'n')
        return 6;
    else if (key == 'b')
        return 7;
    return -1;
}

int get_weapon_id(char type) {
    if (type == 'm')
        return 0;
    if (type == 'a');
        return 1;
    if (type == 'M')
        return 2;
    if (type == 'n')
        return 3;
    return 4;
}

int best_dir(char ***map, struct Point p1, struct Point p2) {
    int rasta = rand() % 2; // 0: aval x baad y, 1: baraaks
    if (rasta == 0) {
        if (p1.x < p2.x && !are_equal(create_point(p1.x + 1, p1.y), p2)) { // down
            struct Point nxt = next_point(p1, 2);
            if (is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 2;
        }
        if (p1.x > p2.x && !are_equal(create_point(p1.x - 1, p1.y), p2)) { // up
            struct Point nxt = next_point(p1, 0);
            if (is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 0;
        }
        if (p1.y < p2.y && !are_equal(create_point(p1.x, p1.y + 1), p2)) { // right
            struct Point nxt = next_point(p1, 1);
            if (is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 1;
        }
        if (p1.y > p2.y && !are_equal(create_point(p1.x, p1.y - 1), p2)) { // left
            struct Point nxt = next_point(p1, 3);
            if (is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 3;
        }
        if (p1.x == p2.x) {
            struct Point nxt = next_point(p1, 0);
            if (!are_equal(nxt, p2) && is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 0;
            nxt = next_point(p1, 2);
            if (!are_equal(nxt, p2) && is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 2;
        }
        if (p1.y == p2.y) {
            struct Point nxt = next_point(p1, 1);
            if (!are_equal(nxt, p2) && is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 1;
            nxt = next_point(p1, 3);
            if (!are_equal(nxt, p2) && is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 3;
        }
    }
    else {
        if (p1.y < p2.y && !are_equal(create_point(p1.x, p1.y + 1), p2)) { // right
            struct Point nxt = next_point(p1, 1);
            if (is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 1;
        }
        if (p1.y > p2.y && !are_equal(create_point(p1.x, p1.y - 1), p2)) { // left
            struct Point nxt = next_point(p1, 3);
            if (is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 3;
        }
        if (p1.x < p2.x && !are_equal(create_point(p1.x + 1, p1.y), p2)) { // down
            struct Point nxt = next_point(p1, 2);
            if (is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 2;
        }
        if (p1.x > p2.x && !are_equal(create_point(p1.x - 1, p1.y), p2)) { // up
            struct Point nxt = next_point(p1, 0);
            if (is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 0;
        }
        if (p1.x == p2.x) {
            struct Point nxt = next_point(p1, 0);
            if (!are_equal(nxt, p2) && is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 0;
            nxt = next_point(p1, 2);
            if (!are_equal(nxt, p2) && is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 2;
        }
        if (p1.y == p2.y) {
            struct Point nxt = next_point(p1, 1);
            if (!are_equal(nxt, p2) && is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 1;
            nxt = next_point(p1, 3);
            if (!are_equal(nxt, p2) && is_in_room(map, nxt) && (*map)[nxt.x][nxt.y] != '<' && (*map)[nxt.x][nxt.y] != '>' && (*map)[nxt.x][nxt.y] != 'O' && (*map)[nxt.x][nxt.y] != '^' && !is_enemy((*map)[nxt.x][nxt.y]))
                return 3;
        }
    }
    return -1;
}

int best_dir_trap(char ***map, struct Point p1, struct Point p2) {
    int rasta = rand() % 2; // 0: aval x baad y, 1: baraaks
    if (rasta == 0) {
        if (p1.x < p2.x && !are_equal(create_point(p1.x + 1, p1.y), p2)) { // down
            struct Point nxt = next_point(p1, 2);
            if (!is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 2;
        }
        if (p1.x > p2.x && !are_equal(create_point(p1.x - 1, p1.y), p2)) { // up
            struct Point nxt = next_point(p1, 0);
            if (!is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 0;
        }
        if (p1.y < p2.y && !are_equal(create_point(p1.x, p1.y + 1), p2)) { // right
            struct Point nxt = next_point(p1, 1);
            if (!is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 1;
        }
        if (p1.y > p2.y && !are_equal(create_point(p1.x, p1.y - 1), p2)) { // left
            struct Point nxt = next_point(p1, 3);
            if (!is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 3;
        }
        if (p1.x == p2.x) {
            struct Point nxt = next_point(p1, 0);
            if (!are_equal(nxt, p2) && !is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 0;
            nxt = next_point(p1, 2);
            if (!are_equal(nxt, p2) && !is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 2;
        }
        if (p1.y == p2.y) {
            struct Point nxt = next_point(p1, 1);
            if (!are_equal(nxt, p2) && !is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 1;
            nxt = next_point(p1, 3);
            if (!are_equal(nxt, p2) && !is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 3;
        }
    }
    else {
        if (p1.y < p2.y && !are_equal(create_point(p1.x, p1.y + 1), p2)) { // right
            struct Point nxt = next_point(p1, 1);
            if (!is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 1;
        }
        if (p1.y > p2.y && !are_equal(create_point(p1.x, p1.y - 1), p2)) { // left
            struct Point nxt = next_point(p1, 3);
            if (!is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 3;
        }
        if (p1.x < p2.x && !are_equal(create_point(p1.x + 1, p1.y), p2)) { // down
            struct Point nxt = next_point(p1, 2);
            if (!is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 2;
        }
        if (p1.x > p2.x && !are_equal(create_point(p1.x - 1, p1.y), p2)) { // up
            struct Point nxt = next_point(p1, 0);
            if (!is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 0;
        }
        if (p1.x == p2.x) {
            struct Point nxt = next_point(p1, 0);
            if (!are_equal(nxt, p2) && !is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 0;
            nxt = next_point(p1, 2);
            if (!are_equal(nxt, p2) && !is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 2;
        }
        if (p1.y == p2.y) {
            struct Point nxt = next_point(p1, 1);
            if (!are_equal(nxt, p2) && !is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 1;
            nxt = next_point(p1, 3);
            if (!are_equal(nxt, p2) && !is_wall((*map)[nxt.x][nxt.y]) && !is_enemy((*map)[nxt.x][nxt.y]))
                return 3;
        }
    }
    return -1;
}

int best_dir_snake(char ***map, struct Point p1, struct Point p2) {
    int cur_dist = get_dist(p1, p2), res = -1;
    for (int dir = 4; dir < 8; dir++) {
        struct Point new_p = next_point(p1, dir);
        int new_dist = get_dist(new_p, p2);
        if (new_dist < cur_dist && !are_equal(new_p, p2) && is_in_room(map, new_p) && (*map)[new_p.x][new_p.y] != '<' && (*map)[new_p.x][new_p.y] != '>' && (*map)[new_p.x][new_p.y] != 'O' && (*map)[new_p.x][new_p.y] != '^' && !is_enemy((*map)[new_p.x][new_p.y])) {
            res = dir;
            cur_dist = new_dist;
        }
    }
    return res == -1? best_dir(map, p1, p2): res;
}

int best_dir_snake_trap(char ***map, struct Point p1, struct Point p2) {
    int cur_dist = get_dist(p1, p2), res = -1;
    for (int dir = 4; dir < 8; dir++) {
        struct Point new_p = next_point(p1, dir);
        int new_dist = get_dist(new_p, p2);
        if (new_dist < cur_dist && !are_equal(new_p, p2) && !is_wall((*map)[new_p.x][new_p.y]) && !is_enemy((*map)[new_p.x][new_p.y])) {
            res = dir;
            cur_dist = new_dist;
        }
    }
    return res == -1? best_dir_trap(map, p1, p2): res;
}

int get_range(int id) {
    if (id == 0)
        return 1;
    else if (id == 1)
        return 5;
    else if (id == 2)
        return 10;
    else if (id == 3)
        return 5;
    else
        return 1;
}

int get_enemy_health(char c) {
    if (c == 'D')
        return 5;
    else if (c == 'F')
        return 10;
    else if (c == 'G')
        return 15;
    else if (c == 'S')
        return 20;
    else if (c == 'U')
        return 30;
}

int get_enemy_moves(char c) {
    if (c == 'D')
        return 0;
    else if (c == 'F')
        return 0;
    else if (c == 'G')
        return 5;
    else if (c == 'S')
        return 1000; // unlimited
    else if (c == 'U')
        return 10;
}

char* get_enemy_name(char c) {
    if (c == 'D')
        return "Deamon";
    if (c == 'F')
        return "Fire Breathing Monster";
    if (c == 'G')
        return "Giant";
    if (c == 'S')
        return "Snake";
    if (c == 'U')
        return "Undeed";
}

int check_corners(char ***map) {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            if ((*map)[i][j] == '+' && is_corner(create_point(i, j)))
                return 0;
    return 1;
}

int vertical_neighbor(char ***map, struct Point p) {
    if (p.x > 0 && (*map)[p.x - 1][p.y] == '.') {
        if (p.x + 2 < GAME_X && (*map)[p.x + 1][p.y] == '+' && (*map)[p.x + 2][p.y] == '.')
            return 1;        
    }
    if (p.x + 1 < GAME_X && (*map)[p.x + 1][p.y] == '.') {
        if (p.x - 2 >= 0 && (*map)[p.x - 1][p.y] == '+' && (*map)[p.x - 2][p.y] == '.')
            return 1;
    }
    return 0;
}

int horizontal_neighbor(char ***map, struct Point p) {
    if (p.y > 0 && (*map)[p.x][p.y - 1] == '.') {
        if (p.y + 2 < GAME_Y && (*map)[p.x][p.y + 1] == '+' && (*map)[p.x][p.y + 2] == '.')
            return 1;
    }
    if (p.y + 1 < GAME_Y && (*map)[p.x][p.y + 1] == '.') {
        if (p.y - 2 >= 0 && (*map)[p.x][p.y - 1] == '+' && (*map)[p.x][p.y - 2] == '.')
            return 1;
    }
    return 0;
}


void sort_miniusers(struct miniUser **user_list) {
    for (int i = 0; i < USERS; i++)
        for (int j = i + 1; j < USERS; j++)
            if (user_list[i]->gold < user_list[j]->gold) {
                struct miniUser* tmp = user_list[i];
                user_list[i] = user_list[j];
                user_list[j] = tmp;
            }
}

char* get_time(time_t t) {
    time_t now;
    time(&now);
    long long dif = (long long) difftime(now, t);
    int d = dif / 86400;
    dif %= 86400;
    int h = dif / 3600;
    dif %= 3600;
    int m = dif / 60;
    int s = dif % 60;
    char* res = malloc(sizeof(char) * MAX_SIZE);
    res[0] = '\0';
    strcat(res, get_str(d));
    strcat(res, " Days, ");
    strcat(res, get_str(h));
    strcat(res, " Hours, ");
    strcat(res, get_str(m));
    strcat(res, " Minutes, ");
    strcat(res, get_str(s));
    strcat(res, " Seconds.");
    return res;
}

void init_time(struct User *user, time_t t) {
    for (int line = 0; line < 3; line++)
        LAST_MESSAGE_REFRESH[line] = t;
    LAST_EAT = t;
    LAST_RECOVERY = t;
    for (int i = 0; i < user->bag.number_of_food; i++)
        user->bag.production_date[i] = t;
}

int refresh_message(time_t t, int line) {
    if (difftime(t, LAST_MESSAGE_REFRESH[line]) > 5) {
        LAST_MESSAGE_REFRESH[line] = t;
        return 1;
    }
    return 0;
}

void reduce_hunger(struct User* user) {
    if (user->hunger > 0)
        --user->hunger;
    else
        --user->health;
}

void check_hunger(struct User* user, time_t t) {
    if (difftime(t, LAST_EAT) > 7 + (2 - DIFFICULTY)) {
        reduce_hunger(user);
        LAST_EAT = t;
    }
}

void refresh_food(struct User* user, time_t now) {
    for (int i = 0; i < user->bag.number_of_food; i++) {
        if (difftime(now, user->bag.production_date[i]) > 60 - 5 * DIFFICULTY) {
            if (user->bag.food[i] == 1)
                user->bag.food[i] = 4;
            else if (user->bag.food[i] == 2 || user->bag.food[i] == 3)
                user->bag.food[i] = 1;
        }
    }
}

void recover_health(struct User* user, time_t now) {
    if (user->hunger != 10)
        return;
    if (difftime(now, LAST_RECOVERY) > RECOVERY) {
        user->health++;
        if (user->health > 10)
            user->health = 10;
        LAST_RECOVERY = now;
    }
}

char guess_char(char ***map, struct Point p) {
    struct Point nxt1, nxt2;
    nxt1 = next_point(p, 0), nxt2 = next_point(p, 2);
    if (is_wall((*map)[nxt1.x][nxt1.y]) && is_wall((*map)[nxt2.x][nxt2.y]))
        return '+';
    nxt1 = next_point(p, 1), nxt2 = next_point(p, 3);
    if (is_wall((*map)[nxt1.x][nxt1.y]) && is_wall((*map)[nxt2.x][nxt2.y]))
        return '+';
    for (int dir = 0; dir < 4; dir++) {
        struct Point nxt = next_point(p, dir);
        if ((*map)[nxt.x][nxt.y] == '#')
            return '#';
    }
    return '.';
}

struct Point get_window(char ***map, struct Point p) {
    for (int dir = 0; dir < 4; dir++) {
        struct Point nxt = next_point(p, dir);
        if ((*map)[nxt.x][nxt.y] == '=')
            return nxt;
    }
    return create_point(-1, -1);
}

int get_window_dir(char ***map, struct Point p) {
    for (int dir = 0; dir < 4; dir++) {
        struct Point nxt = next_point(p, dir);
        if (is_in_map(nxt), is_in_room(map, nxt))
            return (dir + 2) % 4;
    }
    return -1;
}