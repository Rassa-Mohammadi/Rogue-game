// Name: Rassa Mohammadi
// Student: 403106657

#include "essentials.h"
#include "designer.h"
#include "file_processor.h"

void init_mark();
void set_colors();
void create_game_menu();
void create_login_page();
void password_recovery_page();
void create_register_page();
void pregame_menu();
void go_to_settings();
void difficulty_menu();
void hero_color_menu();
void music_menu();
void play_game();
int move_player(int, char***);
void move_in_one_direction();
void play_trap(struct Point);
void appear_map(struct Point, int);
void appear_nightmare(struct Point, int);
void appear_trap_secret();
void disappear_nightmare(struct Point, int);
void hunger_menu();
void consume_food(int, time_t);
void add_weapon(char);
void add_potion(char);
void weapon_menu();
void potion_menu();
void attack(char***, int);
void change_weapon(int, time_t);
void init_enemy(struct Point);
int check_health();
void game_over();
void quit_game();

struct User user;
int is_guest, reveal, speed = 1;
int hero_color = 1;
int speed_boost, health_boost, power_boost;
int mark[MAX_SIZE][MAX_SIZE];
int last_dir = -1;
int music_on = 0;
char genre[MAX_SIZE];

int main() {
    setlocale(LC_ALL, "");
    initscr();
    keypad(stdscr, TRUE);
    srand(time(NULL));
    set_colors();
    create_game_menu();
}

void init_mark() {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            mark[i][j] = 0;
}

void set_colors() {
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_WHITE, COLOR_BLACK);
    init_pair(7, COLOR_YELLOW, COLOR_BLACK);
    int gold_color = 8;
    init_color(gold_color, 1000, 840, 0);
    init_pair(8, gold_color, COLOR_BLACK);
    attron(COLOR_PAIR(1));
}

void create_game_menu() {
    clear();
    curs_set(FALSE);
    noecho();
    int x = LINES / 3, y = COLS / 3;
    int width = COLS / 3, height = LINES / 3;
    draw_box("Welcome to Rogue", create_point(x, y), height, width, '+');
    x += 3, y += 1;
    int choice = 0, key = -1;
    char *options[] = {"Login", "Register", "Guest", "Quit"};
    do {
        if (key == KEY_UP && choice > 0)
            choice--;
        if (key == KEY_DOWN && choice < 3)
            choice++;
        for (int i = 0; i < 4; i++) {
            if (i == choice) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(x + i, y + width / 2 - 3, "%s", options[i]);
                attroff(A_REVERSE | A_BOLD);
            }
            else
                mvprintw(x + i, y + width / 2 - 3, "%s", options[i]);
        }
        key = getch();
    } while (key != '\n');
    if (choice == 0) {
        create_login_page();
        pregame_menu();
    }
    else if (choice == 1) {
        create_register_page();
        pregame_menu();
    }
    else if (choice == 2) {
        is_guest = 1;
        pregame_menu();
    }
    else // quit
        quit_game();
    play_game();
}

void successful(int is_register) {
    clear();
    curs_set(FALSE);
    if (!is_register)
        print_message_with_color(LINES / 3, COLS / 3 - 10, "You have successfully logged in!", 3);
    else
        print_message_with_color(LINES / 3, COLS / 3 - 10, "You have successfully registered!", 3);
    print_message_with_color(LINES / 3 + 2, COLS / 3 - 10, "Press any key to continue ...", 1);
    getch();
}

void password_recovery_page() {
    clear();
    echo();
    int x = LINES / 3, y = COLS / 3;
    char email[MAX_SIZE];
    mvprintw(x, y, "Enter Your Email:");
    while (true) {
        move(x + 1, y);
        getstr(email);
        if (!strcmp(email, user.email)) {
            clean_area(create_point(x, y + 20), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message_with_color(x, y + 20, "Email not correct!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    x += 3;
    mvprintw(x, y, "Enter New Password:");
    while (true) {
        print_message_with_color(x + 2, y - 30, "Password must contain at least 7 characters, 1 number character, 1 capital letter and 1 small letter.", 2);
        print_message_with_color(x + 3, y, "To generate password press space button", 2);
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        while (key != '\n') {
            if (key == ' ') {
                len = MAX_SIZE;
                do {
                    strcpy(user.password, generate_password());
                } while (!valid_password(user.password));
                print_message_with_color(x + 1, y, user.password, 2);
                refresh();
                usleep(2000000);
                break;
            }
            if (key == KEY_BACKSPACE && len > 0) {
                move(x + 1, y + len - 1);
                printw(" ");
                len--;
                move(x + 1, y + len);
            }
            else if (key != KEY_BACKSPACE) {
                user.password[len++] = key;
                printw("*");
            }
            key = getch();
        }
        user.password[len] = '\0';
        if (valid_password(user.password)) {
            clean_area(create_point(x, y + 20), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message_with_color(x, y + 20, "Invalid Password!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    strcpy(user.email, email);
}

void create_login_page() {
    clear();
    curs_set(TRUE);
    int x = LINES / 3, y = COLS / 3;
    while (true) {
        mvprintw(x, y, "Enter Username:");
        move(x + 1, y);
        echo();
        getstr(user.username);
        if (exist_username(user.username)) {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message_with_color(x, y + 16, "Invalid Username!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    load_user(&user);
    char password[MAX_SIZE];
    x += 3;
    while (true) {
        mvprintw(x, y, "Enter Password:");
        print_message_with_color(x + 2, y - 15, "It you have forgotten your password press space button", 2);
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        int recovery = 0;
        while (key != '\n') {
            if (key == ' ') {
                password_recovery_page();
                recovery = 1;
                break;
            }
            else if (key == KEY_BACKSPACE && len > 0) {
                move(x + 1, y + len - 1);
                printw(" ");
                len--;
                move(x + 1, y + len);
            }
            else if (key != KEY_BACKSPACE) {
                password[len++] = key;
                printw("*");
            }
            key = getch();
        }
        password[len] = '\0';
        if (recovery)
            break;
        if (!strcmp(password, user.password)) {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message_with_color(x, y + 16, "Incorrect Password!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    successful(0);
}

void create_register_page() {
    clear();
    curs_set(TRUE);
    int x = LINES / 3, y = COLS / 3;
    // get username
    while (true) {
        mvprintw(x, y, "Enter Username:");
        move(x + 1, y);
        echo();
        getstr(user.username);
        if (exist_username(user.username)) {
            print_message_with_color(x, y + 16, "Username is already used!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
        else {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
    }
    // get password
    x += 3;
    while (true) {
        mvprintw(x, y, "Enter Password:");
        print_message_with_color(x + 2, y - 30, "Password must contain at least 7 characters, 1 number character, 1 capital letter and 1 small letter.", 2);
        print_message_with_color(x + 3, y, "To generate password press space button", 2);
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        while (key != '\n') {
            if (key == ' ') {
                len = MAX_SIZE;
                do {
                    strcpy(user.password, generate_password());
                } while (!valid_password(user.password));
                print_message_with_color(x + 1, y, user.password, 2);
                break;
            }
            if (key == KEY_BACKSPACE && len > 0) {
                move(x + 1, y + len - 1);
                printw(" ");
                len--;
                move(x + 1, y + len);
            }
            else if (key != KEY_BACKSPACE) {
                user.password[len++] = key;
                printw("*");
            }
            key = getch();
        }
        user.password[len] = '\0';
        if (valid_password(user.password)) {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message_with_color(x, y + 16, "Invalid Password!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    // get email
    x += 5;
    while (true) {
        mvprintw(x, y, "Enter Email:");
        move(x + 1, y);
        echo();
        getstr(user.email);
        if (valid_email(user.email)) {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message_with_color(x, y + 16, "Invalid Email!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    create_user(&user);
    successful(1);
}

void difficulty_menu() {
    clear();
    char *options[] = {"Easy (default)", "Medium", "Hard"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 3, 1);
    DIFFICULTY = choice;
    go_to_settings();
}

void hero_color_menu() {
    clear();
    char *options[] = {"Blue (default)", "Red", "Green"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 3, 1);
    hero_color = choice + 1;
    go_to_settings();
}

void music_menu() {
    clear();
    char *options[] = {"No Music (default)", "Plants_vs_Zombies", "Angry_Birds"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 3, 1);
    init_music();
    if (choice == 0) {
        terminate_music();
        music_on = 0;
    }
    else {
        strcpy(genre, options[choice]);
        play_song(options[choice], "main");
        music_on = 1;
    }
    go_to_settings();
}

void go_to_settings() {
    clear();
    char *options[] = {"Set Difficulty", "Hero Color", "Music", "Back"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 4, 1);
    if (choice == 0)
        difficulty_menu();
    else if (choice == 1)
        hero_color_menu();
    else if (choice == 2)
        music_menu();
    else
        pregame_menu();
}

void scoreboard_menu() {
    clear();
    curs_set(FALSE);
    struct miniUser** user_list;
    get_users(&user_list);
    if (user_list == NULL) {
        print_message_with_color(2, COLS / 3, "There are no users!", 2);
        print_message_with_color(4, COLS / 3, "Press any key to return ...", 2);
        getch();
        return;
    }
    sort_miniusers(user_list);
    print_message_with_color(LINES - 1, 0, "Press (Q) to return ...", 2);
    // Titles
    // Rank, Username, Golds, Games, Time
    print_message_with_color(ST_X, ST_Y, "Rank", 4);
    print_message_with_color(ST_X, ST_Y + 10, "Username", 4);
    print_message_with_color(ST_X, ST_Y + 30, "Score", 4);
    print_message_with_color(ST_X, ST_Y + 40, "Golds", 4);
    print_message_with_color(ST_X, ST_Y + 50, "Games", 4);
    print_message_with_color(ST_X, ST_Y + 60, "Time", 4);
    // Users
    int user_pos;
    if (!is_guest) {
        for (int i = 0; i < USERS; i++)
            if (!strcmp(user_list[i]->username, user.username))
                user_pos = i;
    }
    else
        user_pos = -1;
    int cur = 0, key = -1, sz = (USERS >= 4? 4: USERS), st = 0;
    timeout(0);
    do {
        if (key == KEY_UP) {
            if (cur > 0)
                --cur;
            else if (st > 0)
                --st;
        }
        if (key == KEY_DOWN) {
            if (cur + 1 < sz)
                ++cur;
            else if (st + sz < USERS)
                ++st;
        }
        print_user_list(user_list, st, sz, cur, user_pos);
        key = getch();
    } while (key != 'Q');
    timeout(-1);
    pregame_menu();
}

void profile_menu() {
    clear();
    curs_set(FALSE);
    print_message_with_color(LINES / 3, COLS / 3, "User's Profile: ", 3);
    print_message_with_color(LINES / 3 + 2, COLS / 3, "Username: ", 1);
    if (is_guest)
        print_message_with_color(LINES / 3 + 2, COLS / 3 + 11, "Guest", 6);
    else
        print_message_with_color(LINES / 3 + 2, COLS / 3 + 11, user.username, 6);
    print_message_with_color(LINES / 3 + 4, COLS / 3, "Password: ", 1);
    if (is_guest)
        print_message_with_color(LINES / 3 + 4, COLS / 3 + 10, "-", 6);
    else
        print_message_with_color(LINES / 3 + 4, COLS / 3 + 10, user.password, 6);
    print_message_with_color(LINES / 3 + 6, COLS / 3, "Email: ", 1);
    if (is_guest)
        print_message_with_color(LINES / 3 + 6, COLS / 3 + 7, "-", 6);
    else
        print_message_with_color(LINES / 3 + 6, COLS / 3 + 7, user.email, 6);
    print_message_with_color(LINES / 3 + 8, COLS / 3, "Games Played: ", 1);
    if (is_guest)
        print_message_with_color(LINES / 3 + 8, COLS / 3 + 15, "-", 6);
    else
        print_number_with_color(LINES / 3 + 8, COLS / 3 + 15, user.number_of_games, 6);
    print_message_with_color(LINES / 3 + 10, COLS / 3, "Score: ", 1);
    if (is_guest)
        print_number_with_color(LINES / 3 + 10, COLS / 3 + 8, 0, 6);
    else
        print_number_with_color(LINES / 3 + 10, COLS / 3 + 8, user.score, 6);
    print_message_with_color(LINES / 3 + 12, COLS / 3, "Golds: ", 1);
    if (is_guest)
        print_number_with_color(LINES / 3 + 12, COLS / 3 + 8, 0, 6);
    else
        print_number_with_color(LINES / 3 + 12, COLS / 3 + 8, user.golds, 6);
    print_message_with_color(LINES - 1, 0, "Press any key to return ...", 2);
    getch(); 
    pregame_menu();
}

void pregame_menu() {
    clear();
    curs_set(FALSE);
    noecho();
    char *options[] = {"Load previous game", "Create new game", "Profile", "Scoreboard", "Settings", "Back"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 6, 4);
    if (choice == 0) { // load game
        if (is_guest || !has_map(&user) || user.health <= 0 || user.theme[user.level][user.pos.x][user.pos.y] == 't') {
            clear();
            print_message_with_color(LINES / 3, COLS / 3, "There is no previous map for this account!", 2);
            print_message_with_color(LINES / 3 + 1, COLS / 3, "Press any key to return to the previous menu.", 2);
            getch();
            pregame_menu();
        }
    }
    else if (choice == 1) { // create new game
        generate_map(&user);
        update_user(&user);
    }
    else if (choice == 2) {
        profile_menu();
    }
    else if (choice == 3) {
        scoreboard_menu();
    }
    else if (choice == 4) { // settings
        go_to_settings();
    }
    else {
        create_game_menu();
    }
}

void hunger_menu() {
    clear();
    timeout(-1);
    curs_set(FALSE);
    int x = LINES / 3 - 2, y = COLS / 3;
    print_hunger(x, y, &user);
    print_message_with_color(x + 1, y - 15, "The following foods are available in your bag. choose one to recover hunger:", 2);
    refresh();
    char *options[user.bag.number_of_food];
    for (int i = 0; i < user.bag.number_of_food; i++) {
        options[i] = malloc(sizeof(char) * 12);
        if (user.bag.food[i] == 1)
            strcpy(options[i], "Normal food");
        else if (user.bag.food[i] == 2)
            strcpy(options[i], "Aala food");
        else if (user.bag.food[i] == 3)
            strcpy(options[i], "Magical food");
        else
            strcpy(options[i], "Rotten food");
    }
    // create_list
    x += 3;
    curs_set(FALSE);
    noecho();
    change_color(3);
    int choice = 0, key = -1, cnt = user.bag.number_of_food;
    do {
        if (key == KEY_UP && choice > 0)
            choice--;
        if (key == KEY_DOWN && choice < cnt - 1)
            choice++;
        for (int i = 0; i < cnt; i++) {
            if (choice == i) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(x + 2 * i, y, "%s", options[i]);
                attroff(A_REVERSE | A_BOLD);
            }
            else
                mvprintw(x + 2 * i, y, "%s", options[i]);
        }
        key = getch();
    } while (key != '\n' && key != KEY_DC);
    undo_color(3);
    if (!user.bag.number_of_food)
        return;
    if (key == '\n') {
        time_t now;
        time(&now);
        switch (user.bag.food[choice]) {
            case 1:
                consume_food(3, now);
                break;
            case 4:
                consume_food(-1, now);
                break;
            case 2:
                consume_food(3, now);
                power_boost = 5;
                POWER = 2;
                break;
            case 3:
                consume_food(3, now);
                speed_boost = 5;
                speed = 2;
            default:
                break;
        }
    }
    --user.bag.number_of_food;
    for (int i = choice; i < user.bag.number_of_food; i++)
        user.bag.food[i] = user.bag.food[i + 1];
    timeout(0);
}

void consume_food(int x, time_t now) {
    user.hunger += x;
    if (user.hunger >= 10) {
        user.hunger = 10;
        LAST_RECOVERY = now;
    }
    if (user.hunger < 0) {
        user.health += user.hunger;
        user.hunger = 0;
    }
    LAST_EAT = now;
}

void add_weapon(char type) {
    switch (type) {
        case 'm':
            user.bag.weapon[0] += user.info[user.level][user.pos.x][user.pos.y];
            break;
        case 'a':
            user.bag.weapon[1] += user.info[user.level][user.pos.x][user.pos.y];
            break;
        case 'M':
            user.bag.weapon[2] += user.info[user.level][user.pos.x][user.pos.y];
            break;
        case 'n':
            user.bag.weapon[3] += user.info[user.level][user.pos.x][user.pos.y];
            break;
        case 'o':
            user.bag.weapon[4] += user.info[user.level][user.pos.x][user.pos.y];
            break;
    }
}

void change_weapon(int id, time_t now) {
    if (user.cur_weapon != -1) {
        clean_area(create_point(0, 0), create_point(0, COLS - 1));
        print_message_with_color(0, 0, "First, put your current weapon in the backpack", 2);
        LAST_MESSAGE_REFRESH[0] = now;
        return;
    }
    if (user.bag.weapon[id] == 0) {
        clean_area(create_point(0, 0), create_point(0, COLS - 1));
        print_message_with_color(0, 0, "This weapon is not available in your backpack", 2);
        LAST_MESSAGE_REFRESH[0] = now;
        return;
    }
    user.cur_weapon = id;
    clean_area(create_point(0, 0), create_point(0, COLS - 1));
    print_message_with_color(0, 0, "Your current weapon is now: ", 2);
    print_message_with_color(0, 29, WEAPON[id], 2);
    LAST_MESSAGE_REFRESH[0] = now;
}

void add_potion(char type) {
    switch(type) {
        case 'h':
            ++user.bag.health_potion;
            break;
        case 's':
            ++user.bag.speed_potion;
            break;
        case 'd':
            ++user.bag.damage_potion;
            break;
    }
}

void weapon_menu() {
    clear();
    timeout(-1);
    curs_set(FALSE);
    int x = LINES / 3 - 2, y = COLS / 3;
    print_message_with_color(1, 0, "Choose your weapon to fight!", 2);
    // Titles
    // Name, type, range, availabe quantity, damage
    print_message_with_color(ST_X, ST_Y, "Name", 4);
    print_message_with_color(ST_X, ST_Y + 15, "Type", 4);
    print_message_with_color(ST_X, ST_Y + 30, "Range", 4);
    print_message_with_color(ST_X, ST_Y + 40, "Availabe quantity", 4);
    print_message_with_color(ST_X, ST_Y + 60, "Damage", 4);
    print_message_with_color(ST_X, ST_Y + 70, "Symbol", 4);
    char name[][20] = {"Mace", "Dagger", "Magic Wand", "Normal Arrow", "Sword"};
    char type[][20] = {"Short-Range", "Long-Range", "Long-Range", "Long-Range", "Short-Range"};
    int range[] = {1, 5, 10, 5, 1};
    int damage[] = {5, 12, 15, 5, 10};
    char symbol[][20] = {"⚒", "🗡", "\u269A", "➳", "⚔"};
    int key = -1, choice = 0;
    do {
        if (key == KEY_UP && choice > 0)
            --choice;
        if (key == KEY_DOWN && choice < 4)
            ++choice;
        for (int i = 0; i < 5; i++) {
            if (choice == i)
                attron(A_REVERSE | A_BOLD);
            int color = 7;
            if (i == 0 || i == 4)
                color = 6;
            if (i == user.cur_weapon)
                print_message_with_color(ST_X + 2 * i + 2, 0, "-->", color);
            print_message_with_color(ST_X + 2 * i + 2, ST_Y, name[i], color);
            print_message_with_color(ST_X + 2 * i + 2, ST_Y + 15, type[i], color);
            print_number_with_color(ST_X + 2 * i + 2, ST_Y + 32, range[i], color);
            print_number_with_color(ST_X + 2 * i + 2, ST_Y + 45, user.bag.weapon[i], color);
            print_number_with_color(ST_X + 2 * i + 2, ST_Y + 62, damage[i], color);
            print_message_with_color(ST_X + 2 * i + 2, ST_Y + 72, symbol[i], color);
            if (choice == i)
                attroff(A_REVERSE | A_BOLD);
        }
        refresh();
        key = getch();
    } while (key != '\n');
    if (user.bag.weapon[choice] == 0) {
        print_message_with_color(0, 0, "This weapon is not available in your bag!", 2);
        LAST_MESSAGE_REFRESH[0] = time(NULL);
    }
    else
        user.cur_weapon = choice;
    clear();
    timeout(0);
}

void potion_menu() {
    clear();
    timeout(-1);
    int x = LINES / 3 - 2, y = COLS / 3;
    print_message_with_color(x + 1, y - 10, "The following potions are available in your bag:", 2);
    char *options[3];
    for (int i = 0; i < 3; i++)
        options[i] = malloc(sizeof(char) * 20);
    // health
    strcpy(options[0], "Health (");
    strcat(options[0], get_str(user.bag.health_potion));
    strcat(options[0], ")");
    // speed
    strcpy(options[1], "Speed (");
    strcat(options[1], get_str(user.bag.speed_potion));
    strcat(options[1], ")");
    // damage
    strcpy(options[2], "Damage (");
    strcat(options[2], get_str(user.bag.damage_potion));
    strcat(options[2], ")");

    int choice = create_list(create_point(x + 3, y), options, 3, 5);

    time_t now;
    time(&now);    
    if (choice == 0) {
        if (user.bag.health_potion == 0) {
            clean_area(create_point(0, 0), create_point(0, COLS - 1));
            print_message_with_color(0, 0, "There is no health potion in your bag!", 2);
            LAST_MESSAGE_REFRESH[0] = now;
        }
        health_boost = 10;
        RECOVERY = 1;
        --user.bag.health_potion;
    }
    else if (choice == 1) {
        if (user.bag.speed_potion == 0) {
            clean_area(create_point(0, 0), create_point(0, COLS - 1));
            print_message_with_color(0, 0, "There is no speed potion in your bag!", 2);
            LAST_MESSAGE_REFRESH[0] = now;
        }
        speed_boost = 10;
        speed = 2;
        --user.bag.speed_potion;
    }
    else {
        if (user.bag.damage_potion == 0) {
            clean_area(create_point(0, 0), create_point(0, COLS - 1));
            print_message_with_color(0, 0, "There is no damage potion in your bag!", 2);
            LAST_MESSAGE_REFRESH[0] = now;
        }
        power_boost = 10;
        POWER = 2;
        --user.bag.damage_potion;
    }
    timeout(0);
}

int move_player2x(int key, char ***map) {
    int dir = get_dir(key);
    if (dir == -1)
        return 0;
    struct Point nxt = next_point(user.pos, dir);
    if (!is_in_map(nxt) || !not_restricted(&user, map, nxt) || is_enemy((*map)[nxt.x][nxt.y]))
        return 0;
    user.pos.x = nxt.x, user.pos.y = nxt.y;
    if ((*map)[user.pos.x][user.pos.y] == '_' || (*map)[user.pos.x][user.pos.y] == '|')
            (*map)[user.pos.x][user.pos.y] = '?';
    struct Point nxt2 = next_point(nxt, dir);
    if (!is_in_map(nxt2) || !not_restricted(&user, map, nxt2) || is_enemy((*map)[nxt.x][nxt.y]))
        return 1;
    user.pos.x = nxt2.x, user.pos.y = nxt2.y;
    if ((*map)[user.pos.x][user.pos.y] == '_' || (*map)[user.pos.x][user.pos.y] == '|')
            (*map)[user.pos.x][user.pos.y] = '?';
    return 2;
}

int move_player(int key, char ***map) {
    if (speed == 2)
        return move_player2x(key, map);
    struct Point nxt;
    int dir = get_dir(key);
    if (dir == -1)
        return 0;
    nxt = next_point(user.pos, dir);
    if (is_in_map(nxt) && not_restricted(&user, map, nxt) && !is_enemy((*map)[nxt.x][nxt.y])) {
        user.pos.x = nxt.x, user.pos.y = nxt.y;
        if ((*map)[user.pos.x][user.pos.y] == '_' || (*map)[user.pos.x][user.pos.y] == '|')
            (*map)[user.pos.x][user.pos.y] = '?';
        return 1;
    }
    return 0;
}

void move_in_one_direction() {
    timeout(-1);
    int key = getch();
    while (true) {
        int dir = get_dir(key);
        if (dir == -1)
            break;
        struct Point nxt = next_point(user.pos, dir);
        char c = user.map[user.level][nxt.x][nxt.y];
        if (c == '.' || c == '#' || c == '+' || c == '?') {
            move_player(key, &(user.map[user.level]));
            if (user.theme[user.level][user.pos.x][user.pos.y] != 'n')
                appear_map(user.pos, 5);
        }
        else
            break;
    }
    timeout(0);
}

void appear_map(struct Point p, int depth) {
    int level = user.level;
    (user.mask)[level][p.x][p.y] = 1;
    if (!depth)
        return;
    if (is_in_room(&(user.map[level]), p)) {
        for (int dir = 0; dir < 8; dir++) {
            struct Point nxt = next_point(p, dir);
            if (is_in_room(&(user.map[level]), nxt) && !(user.mask)[level][nxt.x][nxt.y])
                appear_map(nxt, 5);
            else
                (user.mask)[level][nxt.x][nxt.y] = 1;
        }
    }
    else { // +, #
        for (int dir = 0; dir < 4; dir++) {
            struct Point nxt = next_point(p, dir);
            if (is_in_map(nxt)) {
                if (is_in_corridor(user.map[level][nxt.x][nxt.y]))
                    appear_map(nxt, depth - 1);
                if (is_in_room(&(user.map[level]), nxt) && depth == 5)
                    appear_map(nxt, 5);
            }
        }
    }
}

void appear_trap_secret() {
    struct Point nxt;
    for (int dir = 0; dir < 8; dir++) {
        nxt = next_point(user.pos, dir);
        if (user.trap[user.level][nxt.x][nxt.y].exist)
            user.map[user.level][nxt.x][nxt.y] = '^';
        if (user.door[user.level][nxt.x][nxt.y].exist && (user.map[user.level][nxt.x][nxt.y] == '|' || user.map[user.level][nxt.x][nxt.y] == '_')) {
            user.map[user.level][nxt.x][nxt.y] = '?';
            user.mask[user.level][nxt.x][nxt.y] = 1;
        }
    }
}

void appear_nightmare(struct Point p, int depth) {
    user.mask[user.level][p.x][p.y] = 1;
    if (!depth)
        return;
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (!is_in_map(nxt))
            continue;
        int c = user.theme[user.level][user.pos.x][user.pos.y];
        if ((is_in_room(&user.map[user.level], nxt) || is_wall(user.map[user.level][nxt.x][nxt.y]) || is_door(user.map[user.level][nxt.x][nxt.y]) || user.map[user.level][nxt.x][nxt.y] == '=') && c == 'n')
            appear_nightmare(nxt, depth - 1);
    }
}

void disappear_nightmare(struct Point p, int depth) {
    user.mask[user.level][p.x][p.y] = 0;
    if (!depth)
        return;
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (!is_in_map(nxt))
            continue;
        int c = user.theme[user.level][user.pos.x][user.pos.y];
        if ((is_in_room(&user.map[user.level], nxt) || is_wall(user.map[user.level][nxt.x][nxt.y]) || is_door(user.map[user.level][nxt.x][nxt.y]) || user.map[user.level][nxt.x][nxt.y] == '=') && c == 'n')
            disappear_nightmare(nxt, depth - 1);
    }
}

void appear_room(struct Point p) {
    if (mark[p.x][p.y])
        return;
    mark[p.x][p.y] = 1;
    ++user.mask[user.level][p.x][p.y];
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (mark[nxt.x][nxt.y])
            continue;
        if (is_in_room(&user.map[user.level], nxt))
            appear_room(nxt);
        else {
            ++user.mask[user.level][nxt.x][nxt.y];
            mark[nxt.x][nxt.y] = 1;
        }
    }
}

void disappear_room(struct Point p) {
    if (mark[p.x][p.y])
        return;
    mark[p.x][p.y] = 1;
    --user.mask[user.level][p.x][p.y];
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (mark[nxt.x][nxt.y])
            continue;
        if (is_in_room(&user.map[user.level], nxt))
            disappear_room(nxt);
        else {
            mark[nxt.x][nxt.y] = 1;
            --user.mask[user.level][nxt.x][nxt.y];
        }
    }
}

void appear_window(struct Point p, int dir) {
    struct Point nxt = next_point(p, dir);
    while (is_in_map(nxt)) {
        if (is_in_room(&user.map[user.level], nxt))
            appear_room(nxt);
        nxt = next_point(nxt, dir);
    }
}

void disappear_window(struct Point p, int dir) {
    struct Point nxt = next_point(p, dir);
    while (is_in_map(nxt)) {
        if (is_in_room(&user.map[user.level], nxt))
            disappear_room(nxt);
        nxt = next_point(nxt, dir);
    }
}

int check_health() {
    if (user.health <= 0) {
        clear();
        print_message_with_color(LINES / 3, COLS / 3, "There is no health left for you!", 2);
        update_user(&user);
        timeout(-1);
        print_message_with_color(LINES / 3 + 1, COLS / 3, "Press any key to continue ...", 2);
        refresh();
        getch();
        game_over();
        return 0;
    }
    return 1;
}

void congrat() {
    clear();
    print_message_with_color(LINES / 3, COLS / 3 - 10, "You finished the game and reached the treasure room!", 3);
    print_message_with_color(LINES / 3 + 2, COLS / 3 - 10, "Because of this amazing success, 50 golds will be added to your account!", 8);
    print_message_with_color(LINES / 3 + 4, COLS / 3 - 10, "Press any key to continue ...", 1);
    user.golds += 50;
    user.score += 50;
    getch();
}

void init_enemy(struct Point p) {
    mark[p.x][p.y] = 1;
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (is_in_room(&user.map[user.level], nxt) && !mark[nxt.x][nxt.y]) {
            if (is_enemy(user.map[user.level][nxt.x][nxt.y])) {
                user.enemy[user.level][nxt.x][nxt.y].health = get_enemy_health(user.map[user.level][nxt.x][nxt.y]);
                if (user.map[user.level][nxt.x][nxt.y] == 'S')
                    user.enemy[user.level][nxt.x][nxt.y].moves = get_enemy_moves(user.map[user.level][nxt.x][nxt.y]);
                else
                    user.enemy[user.level][nxt.x][nxt.y].moves = -1;
            }
            init_enemy(nxt);
        }
    }
}

void trigger_enemy(struct Point p) {
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (is_in_room(&user.map[user.level], nxt) && is_enemy(user.map[user.level][nxt.x][nxt.y]) && user.enemy[user.level][nxt.x][nxt.y].moves == -1)
            user.enemy[user.level][nxt.x][nxt.y].moves = get_enemy_moves(user.map[user.level][nxt.x][nxt.y]);
    }
}

void trigger_enemy_trap(char ***map, struct Enemy enemy[GAME_X][GAME_Y]) {
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(user.pos, dir);
        if (is_in_map(nxt) && is_enemy((*map)[nxt.x][nxt.y]) && (*map)[nxt.x][nxt.y] != 'S' && enemy[nxt.x][nxt.y].moves == -1) {
            enemy[nxt.x][nxt.y].moves = get_enemy_moves((*map)[nxt.x][nxt.y]);
        }
    }
}

void move_enemy(struct Point p) {
    mark[p.x][p.y] = 1;
    if (is_enemy(user.map[user.level][p.x][p.y]) && user.enemy[user.level][p.x][p.y].moves > 0) {
        int dir;
        if (user.map[user.level][p.x][p.y] == 'S')
            dir = best_dir_snake(&user.map[user.level], p, user.pos);
        else
            dir = best_dir(&user.map[user.level], p, user.pos);
        if (dir != -1) {
            struct Point nxt = next_point(p, dir);
            user.enemy[user.level][nxt.x][nxt.y] = user.enemy[user.level][p.x][p.y];
            --user.enemy[user.level][nxt.x][nxt.y].moves;
            user.map[user.level][nxt.x][nxt.y] = user.map[user.level][p.x][p.y];
            user.map[user.level][p.x][p.y] = '.';
            mark[nxt.x][nxt.y] = 1;
        }
    }
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (is_in_room(&user.map[user.level], nxt) && !mark[nxt.x][nxt.y])
            move_enemy(nxt);
    }
}

void move_enemy_trap(char ***map, struct Enemy enemy[GAME_X][GAME_Y]) {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            if (mark[i][j])
                continue;
            if (is_enemy((*map)[i][j]) && enemy[i][j].moves > 0) {
                int dir;
                if ((*map)[i][j] == 'S')
                    dir = best_dir_snake_trap(map, create_point(i, j), user.pos);
                else
                    dir = best_dir_trap(map, create_point(i, j), user.pos);
                if (dir != -1) {
                    struct Point nxt = next_point(create_point(i, j), dir);
                    enemy[nxt.x][nxt.y] = enemy[i][j];
                    --enemy[nxt.x][nxt.y].moves;
                    (*map)[nxt.x][nxt.y] = (*map)[i][j];
                    (*map)[i][j] = '.';
                    mark[nxt.x][nxt.y] = 1;
                }
            }
            mark[i][j] = 1;
        }
}

void enemy_attack(struct Point p) {
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (is_enemy(user.map[user.level][nxt.x][nxt.y])) {
            user.health -= DIFFICULTY + 1;
            clean_area(create_point(0, 0), create_point(0, COLS - 1));
            print_message_with_color(0, 0, "You have been hit. ", 2);
            print_number_with_color(0, 20, user.health, 3);
            print_message_with_color(0, 22, "health remains", 2);
            LAST_MESSAGE_REFRESH[0] = time(NULL);
        }
    }
}

void enemy_attack_trap(char ***map) {
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(user.pos, dir);
        if (is_enemy((*map)[nxt.x][nxt.y])) {
            user.health -= DIFFICULTY + 1;
            clean_area(create_point(0, 0), create_point(0, COLS - 1));
            print_message_with_color(0, 0, "You have been hit. ", 2);
            print_number_with_color(0, 20, user.health, 3);
            print_message_with_color(0, 22, "health remains", 2);
            LAST_MESSAGE_REFRESH[0] = time(NULL);
        }
    }
}

int damage(char ***map, struct Point p) {
    if (is_enemy((*map)[p.x][p.y])) {
        user.enemy[user.level][p.x][p.y].health -= DAMAGE[user.cur_weapon] * POWER;
        if (user.enemy[user.level][p.x][p.y].health <= 0)
            user.enemy[user.level][p.x][p.y].health = user.enemy[user.level][p.x][p.y].moves = 0;
        clean_area(create_point(1, 0), create_point(1, COLS - 1));
        print_message_with_color(1, 0, "You hit the ", 3);
        print_message_with_color(1, 13, get_enemy_name((*map)[p.x][p.y]), 2);
        print_message_with_color(1, 35, "You need ", 3);
        print_number_with_color(1, 45, user.enemy[user.level][p.x][p.y].health, 2);
        print_message_with_color(1, 48, "more hits.", 3);
        LAST_MESSAGE_REFRESH[1] = time(NULL);
        if (user.enemy[user.level][p.x][p.y].health <= 0) {
            clean_area(create_point(1, 0), create_point(1, COLS - 1));
            print_message_with_color(1, 0, "You killed the ", 3);
            print_message_with_color(1, 16, get_enemy_name((*map)[p.x][p.y]), 2);
            LAST_MESSAGE_REFRESH[1] = time(NULL);
            (*map)[p.x][p.y] = '.';
        }
        return 1;
    }
    return 0;
}

int damage_trap(char ***map, struct Enemy enemy[GAME_X][GAME_Y], struct Point p) {
    if (is_enemy((*map)[p.x][p.y])) {
        enemy[p.x][p.y].health -= DAMAGE[user.cur_weapon] * POWER;
        if (enemy[p.x][p.y].health <= 0)
            enemy[p.x][p.y].health = enemy[p.x][p.y].moves = 0;
        clean_area(create_point(1, 0), create_point(1, COLS - 1));
        print_message_with_color(1, 0, "You hit the ", 3);
        print_message_with_color(1, 13, get_enemy_name((*map)[p.x][p.y]), 2);
        print_message_with_color(1, 35, "You need ", 3);
        print_number_with_color(1, 45, enemy[p.x][p.y].health, 2);
        print_message_with_color(1, 48, "more hits.", 3);
        LAST_MESSAGE_REFRESH[1] = time(NULL);
        if (enemy[p.x][p.y].health <= 0) {
            clean_area(create_point(1, 0), create_point(1, COLS - 1));
            print_message_with_color(1, 0, "You killed the ", 3);
            print_message_with_color(1, 16, get_enemy_name((*map)[p.x][p.y]), 2);
            LAST_MESSAGE_REFRESH[1] = time(NULL);
            (*map)[p.x][p.y] = '.';
        }
        return 1;
    }
    return 0;
}

void attack(char ***map, int is_a_attack) {
    if (user.cur_weapon != 0 && user.cur_weapon != 4) {
        int dir;
        if (is_a_attack)
            dir = last_dir;
        else {
            timeout(-1);
            int key = getch();
            dir = get_dir(key);
            timeout(0);
        }
        last_dir = dir;
        if (dir == -1)
            return;
        int dist = get_range(user.cur_weapon);
        struct Point cur = user.pos;
        struct Point nxt = next_point(cur, dir);
        while (!is_enemy((*map)[cur.x][cur.y]) && not_restricted(&user, map, nxt) && dist > 0) {
            --dist;
            cur = nxt;
            nxt = next_point(cur, dir);
        }
        if (!damage(map, cur)) {
            clean_area(create_point(0, 0), create_point(0, COLS - 1));
            print_message_with_color(0, 0, "You missed your shot!", 2);
            LAST_MESSAGE_REFRESH[0] = time(NULL);
            (*map)[cur.x][cur.y] = wEAPON[user.cur_weapon];
            user.info[user.level][cur.x][cur.y] = 1;
        }
        --user.bag.weapon[user.cur_weapon];
        if (user.bag.weapon[user.cur_weapon] <= 0)
            user.cur_weapon = -1;
    }
    else if (user.cur_weapon != -1) {
        for (int dir = 0; dir < 8; dir++) {
            struct Point nxt = next_point(user.pos, dir);
            damage(map, nxt);
        }
    }
    else {
        clean_area(create_point(0, 0), create_point(0, COLS - 1));
        print_message_with_color(0, 0, "You don't have a weapon in your hand. Please take a weapon from your bag.", 2);
        LAST_MESSAGE_REFRESH[0] = time(NULL);
    }
}

void attack_trap(char ***map, struct Enemy enemy[GAME_X][GAME_Y], int is_a_attack) {
    if (user.cur_weapon != 0 && user.cur_weapon != 4) {
        int dir;
        if (is_a_attack)
            dir = last_dir;
        else {
            timeout(-1);
            int key = getch();
            dir = get_dir(key);
            timeout(0);
        }
        last_dir = dir;
        if (dir == -1)
            return;
        int dist = get_range(user.cur_weapon);
        struct Point cur = user.pos;
        struct Point nxt = next_point(cur, dir);
        while (!is_enemy((*map)[cur.x][cur.y]) && !is_wall((*map)[nxt.x][nxt.y]) && dist > 0) {
            --dist;
            cur = nxt;
            nxt = next_point(cur, dir);
        }
        if (!damage_trap(map, enemy, cur)) {
            clean_area(create_point(0, 0), create_point(0, COLS - 1));
            print_message_with_color(0, 0, "You missed your shot!", 2);
            LAST_MESSAGE_REFRESH[0] = time(NULL);
            (*map)[cur.x][cur.y] = wEAPON[user.cur_weapon];
        }
        --user.bag.weapon[user.cur_weapon];
        if (user.bag.weapon[user.cur_weapon] <= 0)
            user.cur_weapon = -1;
    }
    else if (user.cur_weapon != -1) {
        for (int dir = 0; dir < 8; dir++) {
            struct Point nxt = next_point(user.pos, dir);
            damage_trap(map, enemy, nxt);
        }
    }
    else {
        clean_area(create_point(0, 0), create_point(0, COLS - 1));
        print_message_with_color(0, 0, "You don't have a weapon in your hand. Please take a weapon from your bag.", 2);
        LAST_MESSAGE_REFRESH[0] = time(NULL);
    }
}

void change_music(char theme) {
    terminate_music();
    switch (theme) {
        case 't':
            play_song(genre, "treasure");
            break;
        case 'n':
            play_song(genre, "nightmare");
            break;
        case 'e':
            play_song(genre, "enchant");
            break;
        default:
            play_song(genre, "main");
    }
}

void play_trap(struct Point p) {
    clear();
    char **map;
    map = malloc(GAME_X * sizeof(char *));
    for (int i = 0; i < GAME_X; i++)
        map[i] = malloc(GAME_Y * sizeof(char));
    struct Enemy enemy[GAME_X][GAME_Y];
    create_battle_room(&user, &map, enemy);
    print_message_with_color(2, COLS / 3, "You fell into a trap!", 2);
    int cycle = 0;
    time_t now;
    while (has_enemy(&map)) {
        time(&now);
        cycle = (cycle + 1) % (5000 / (1 + DIFFICULTY));
        // refresh food
        refresh_food(&user, now);
        // recover health
        recover_health(&user, now);
        // refresh recovery
        if (RECOVERY == 1 && health_boost <= 0)
            RECOVERY = 2;
        // refresh speed
        if (speed == 2 && speed_boost <= 0)
            speed = 1;
        // refresh power
        if (POWER = 2 && power_boost <= 0)
            POWER = 1;
        // messages
        for (int line = 0; line < 3; line++)
            if (refresh_message(now, line))
                clean_area(create_point(line, 0), create_point(line, COLS - 1));
        // hunger
        check_hunger(&user, now);
        timeout(0);
        // item
        if (is_weapon(map[user.pos.x][user.pos.y])) {
            ++user.bag.weapon[get_weapon_id(map[user.pos.x][user.pos.y])];
            map[user.pos.x][user.pos.y] = '.';
        }
        // trigger enemy
        trigger_enemy_trap(&map, enemy);
        if (cycle == 0) {
            --speed_boost;
            --health_boost;
            --power_boost;
            init_mark();
            move_enemy_trap(&map, enemy);
            enemy_attack_trap(&map);
        }
        check_health();
        // print map
        print_map_trap(&map);
        print_message_with_color(user.pos.x + ST_X, user.pos.y + ST_Y, "$", hero_color);
        print_status(&user);
        refresh();
        int key = getch();
        move_player(key, &map);
        if (key == 'E')
            hunger_menu(now);
        if (key == 'i')
            weapon_menu();
        if (key == 'p')
            potion_menu();
        if (key == 'w')
            user.cur_weapon = -1;
        if (key >= '1' && key <= '5')
            change_weapon(key - '1', now);
        if (key == ' ')
            attack_trap(&map, enemy, 0);
        if (key == 'a')
            attack_trap(&map, enemy, 1);
    }
    user.map[user.level][p.x][p.y] = '^';
    timeout(-1);
    clear();
}

void play_game() {
    clear();
    int key;
    time_t now, st_enchant;
    int is_in_enchant = 0, is_gmove = 0, num_gmove = 0;
    time(&now);
    init_time(&user, now);
    init_mark();
    init_enemy(user.pos);
    int last_theme = 'r';
    struct Point last_window = create_point(-1, -1);
    int cycle = 0;
    do {
        cycle = (cycle + 1) % (5000 / (1 + DIFFICULTY));
        time(&now);
        // refresh food
        refresh_food(&user, now);
        // recover health
        recover_health(&user, now);
        // refresh recovery
        if (RECOVERY == 1 && health_boost <= 0)
            RECOVERY = 2;
        // refresh speed
        if (speed == 2 && speed_boost <= 0)
            speed = 1;
        // refresh power
        if (POWER = 2 && power_boost <= 0)
            POWER = 1;
        // messages
        for (int line = 0; line < 3; line++)
            if (refresh_message(now, line))
                clean_area(create_point(line, 0), create_point(line, COLS - 1));
        // hunger
        check_hunger(&user, now);
        timeout(0);
        // reduce health --> enchanted room
        if (user.theme[user.level][user.pos.x][user.pos.y] == 'e') {
            if (!is_in_enchant) {
                is_in_enchant = 1;
                st_enchant = now;
            }
            else if (difftime(now, st_enchant) > (2 - DIFFICULTY) + 4) {
                st_enchant = now;
                --user.health;
            }
        }
        else
            is_in_enchant = 0;
        check_health();
        // treasure
        if (user.map[user.level][user.pos.x][user.pos.y] == 'T') {
            create_treasure_room(&user);
            init_mark();
            init_enemy(user.pos);
        }
        // trap
        if (user.trap[user.level][user.pos.x][user.pos.y].exist) {
            struct Point pos_copy = user.pos;
            play_trap(create_point(user.pos.x, user.pos.y));
            user.pos = pos_copy;
        }
        check_health();
        // gold
        if (user.map[user.level][user.pos.x][user.pos.y] == 'g' && !is_gmove) {
            // int cnt = get_gold(user.map[user.level][user.pos.x][user.pos.y]);
            if (user.theme[user.level][user.pos.x][user.pos.y] != 'n') {
                user.golds += user.gold[user.level][user.pos.x][user.pos.y].cnt;
                user.score += user.gold[user.level][user.pos.x][user.pos.y].cnt;
                clean_area(create_point(2, 0), create_point(2, COLS - 1));
                print_message_with_color(2, 0, "You gained ", 8);
                print_number_with_color(2, 11, user.gold[user.level][user.pos.x][user.pos.y].cnt, 8);
                print_message_with_color(2, 13, "golds!", 8);
                LAST_MESSAGE_REFRESH[2] = now;
            }
            user.map[user.level][user.pos.x][user.pos.y] = '.';
        }
        // food
        if (is_food(&user) && !is_gmove) {
            if (user.bag.number_of_food == 5) {
                clean_area(create_point(1, 0), create_point(1, COLS - 1));
                print_message_with_color(1, 0, "Bag is full! Can not pick food!", 2);
                LAST_MESSAGE_REFRESH[1] = now;
            }
            else {
                char c = user.theme[user.level][user.pos.x][user.pos.y];
                if (c != 'n') {
                    clean_area(create_point(1, 0), create_point(1, COLS - 1));
                    print_message_with_color(1, 0, "Food has been added to your bag!", 3);
                    LAST_MESSAGE_REFRESH[1] = now;
                    user.bag.production_date[user.bag.number_of_food] = now;
                    user.bag.food[user.bag.number_of_food] = user.info[user.level][user.pos.x][user.pos.y];
                    ++user.bag.number_of_food;
                }
                user.map[user.level][user.pos.x][user.pos.y] = '.';
            }
        }
        // weapon
        if (is_weapon(user.map[user.level][user.pos.x][user.pos.y]) && !is_gmove) {
            char c = user.theme[user.level][user.pos.x][user.pos.y];
            add_weapon(user.map[user.level][user.pos.x][user.pos.y]);
            user.map[user.level][user.pos.x][user.pos.y] = guess_char(&user.map[user.level], user.pos);
        }
        // potion
        if (is_potion(user.map[user.level][user.pos.x][user.pos.y]) && !is_gmove) {
            add_potion(user.map[user.level][user.pos.x][user.pos.y]);
            user.map[user.level][user.pos.x][user.pos.y] = '.';
        }
        // trigger enemy
        trigger_enemy(user.pos);
        // move enemy
        if (cycle == 0) {
            --speed_boost;
            --health_boost;
            --power_boost;

            init_mark();
            move_enemy(user.pos);
            enemy_attack(user.pos);
        }
        // enter new room
        if (user.map[user.level][user.pos.x][user.pos.y] == '?' || user.map[user.level][user.pos.x][user.pos.y] == '+' || user.map[user.level][user.pos.x][user.pos.y] == '_' || user.map[user.level][user.pos.x][user.pos.y] == '|') {
            if (is_new_room(&user)) {
                clean_area(create_point(0, 0), create_point(0, COLS - 1));
                print_message_with_color(0, 0, "You have entered a new room", 2);
                LAST_MESSAGE_REFRESH[0] = now;
                init_mark();
                init_enemy(user.pos);
            }
        }
        // window
        if (is_in_room(&user.map[user.level], user.pos)) {
            struct Point cur_window = get_window(&user.map[user.level], user.pos);
            if (last_window.x == -1 && cur_window.x != -1) {
                init_mark();
                appear_window(cur_window, get_window_dir(&user.map[user.level], cur_window));
            }
            else if (last_window.x != -1 && cur_window.x == -1) {
                init_mark();
                disappear_window(last_window, get_window_dir(&user.map[user.level], last_window));
            }
            last_window = cur_window;
        }
        // change level
        if (user.map[user.level][user.pos.x][user.pos.y] == '<') {
            --user.level;
            timeout(-1);
        }
        else if (user.map[user.level][user.pos.x][user.pos.y] == '>') {
            ++user.level;
            if (!user.mask[user.level][user.pos.x][user.pos.y]) {
                clean_area(create_point(0, 0), create_point(0, COLS - 1));
                print_message_with_color(0, 0, "You have entered a new floor. Floor level is ", 2);
                print_number_with_color(0, 45, user.level + 1, 2);
                LAST_MESSAGE_REFRESH[0] = now;
                init_mark();
                init_enemy(user.pos);
            }
            timeout(-1);
        }
        if (user.theme[user.level][user.pos.x][user.pos.y] == 'n')
            appear_nightmare(user.pos, 2);
        else
            appear_map(user.pos, 5);
        
        print_map(&user, reveal);
        print_message_with_color(user.pos.x + ST_X, user.pos.y + ST_Y, "$", hero_color);
        print_status(&user);
        refresh();
        if (user.theme[user.level][user.pos.x][user.pos.y] == 'n')
            disappear_nightmare(user.pos, 2);
        key = getch();
        int has_moved = move_player(key, &(user.map[user.level]));
        // if (has_moved > 0) {
        //     --speed_boost;
        //     --health_boost;
        //     --power_boost;
        // }
        if (is_gmove) {
            num_gmove += has_moved;
            if (num_gmove == 2) {
                num_gmove = 0;
                is_gmove = 0;
            }
        }
        // change music
        int cur_theme = user.theme[user.level][user.pos.x][user.pos.y];
        if (music_on && last_theme != cur_theme) {
            change_music(cur_theme);
            last_theme = cur_theme;
        }
        if (key == 'M')
            reveal = 1 - reveal;
        if (key == 'E')
            hunger_menu(now);
        if (key == 'i')
            weapon_menu();
        if (key == 'p')
            potion_menu();
        if (key == 'f')
            move_in_one_direction();
        if (key == 'g')
            is_gmove = 1;
        if (key == 's')
            appear_trap_secret();
        if (key == 'w')
            user.cur_weapon = -1;
        if (key >= '1' && key <= '5')
            change_weapon(key - '1', now);
        if (key == ' ')
            attack(&user.map[user.level], 0);
        if (key == 'a')
            attack(&user.map[user.level], 1);
    } while (key != 'Q');
    timeout(-1);
    if (user.health > 0 && user.theme[user.level][user.pos.x][user.pos.y] == 't')
        congrat();
    update_user(&user);
    game_over();
}

void game_over() {
    usleep(500000);
    refresh();
    clear();
    curs_set(FALSE);
    int x = LINES / 3, y = COLS / 3;
    print_message_with_color(x, y, "Game over!", 2);
    mvprintw(x + 2, y, "Press any key to return to game menu ...");
    refresh();
    getch();
    is_guest = 0;
    last_dir = -1;
    reveal = 0;
    for (int level = 0; level < 4; level++)
        init_user(&user, level);
    create_game_menu();
}

void quit_game() {
    endwin();
    exit(0);
}
