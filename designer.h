void draw_horizontal_line(struct Point p, int width, char c) {
    move(p.x, p.y);
    for (int i = 0; i < width; i++)
        printw("%c", c);
}

void draw_vertical_line(struct Point p, int height, char c) {
    for (int i = 0; i < height; i++)
        mvprintw(p.x + i, p.y, "%c", c);
}

void draw_border(struct Point p, int height, int width, char c_hor, char c_ver) {
    draw_horizontal_line(p, width, c_hor);
    p.x++;
    draw_vertical_line(p, height - 2, c_ver);
    p.y += width - 1;
    draw_vertical_line(p, height - 2, c_ver);
    p = create_point(p.x + height - 2, p.y - width + 1);
    draw_horizontal_line(p, width, c_hor);
}

void draw_box(char title[], struct Point p, int height, int width, char c) {
    height += 2;
    width += 2;
    draw_border(p, height, width, c, c);
    if (title[0] != '\0') {
        attron(A_BOLD | A_UNDERLINE);
        mvprintw(p.x + 1, p.y + width / 2 - strlen(title) / 2, "%s", title);
        attroff(A_BOLD | A_UNDERLINE);
        draw_horizontal_line(create_point(p.x + 2, p.y), width, c);
    }
}

void clean_area(struct Point top_left, struct Point bottom_right) {
    for (int i = top_left.x; i <= bottom_right.x; i++)
        for (int j = top_left.y; j <= bottom_right.y; j++)
            mvprintw(i, j, " ");
}

int create_list(struct Point st, char *options[], int cnt, int color) {
    curs_set(FALSE);
    noecho();
    change_color(color);
    int choice = 0, key = -1;
    do {
        if (key == KEY_UP && choice > 0)
            choice--;
        if (key == KEY_DOWN && choice < cnt - 1)
            choice++;
        for (int i = 0; i < cnt; i++) {
            if (choice == i) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(st.x + 2 * i, st.y, "%s", options[i]);
                attroff(A_REVERSE | A_BOLD);
            }
            else
                mvprintw(st.x + 2 * i, st.y, "%s", options[i]);
        }
        key = getch();
    } while (key != '\n');
    undo_color(color);
    return choice;
}

void update_theme(struct User* user, int level, struct Room* room) {
    for (int i = room->p.x; i < room->p.x + room->height + 2; i++)
        for (int j = room->p.y; j < room->p.y + room->width + 2; j++)
            user->theme[level][i][j] = room->type;
}

int create_room(struct User* user, char ***map, struct Point p, int height, int width, struct Room** rooms, int index, int level) {
    (*rooms)[index].p = create_point(p.x, p.y);
    (*rooms)[index].height = height;
    (*rooms)[index].width = width;
    // determine room type
    if ((rand() % 10) < 7)
        (*rooms)[index].type = 'r';
    else if (rand() % 2) // enchanted room
        (*rooms)[index].type = 'e';
    else // nightmare room
        (*rooms)[index].type = 'n';
    corners[4 * index] = create_point(p.x, p.y);
    corners[4 * index + 1] = create_point(p.x + height + 1, p.y);
    corners[4 * index + 2] = create_point(p.x, p.y + width + 1);
    corners[4 * index + 3] = create_point(p.x + height + 1, p.y + width + 1);
    return 1;
}

void draw_room(char ***map, struct Room* room) {
    for (int j = 0; j < room->width + 2; j++)
        (*map)[room->p.x][room->p.y + j] = '_';

    for (int j = 1; j < room->width + 1; j++)
        (*map)[room->p.x + room->height + 1][room->p.y + j] = '_';
    
    for (int i = 1; i < room->height + 2; i++) {
        (*map)[room->p.x + i][room->p.y] = '|';
        (*map)[room->p.x + i][room->p.y + room->width + 1] = '|';
    }
    for (int i = 0; i < room->height; i++)
        for (int j = 0; j < room->width; j++)
            (*map)[room->p.x + i + 1][room->p.y + j + 1] = '.';
}

int check_rooms_dist(struct Room** rooms, int index) {
    int X_1 = (*rooms)[index].p.x - 3, X_2 = (*rooms)[index].p.x + (*rooms)[index].height + 1 + 3;
    int Y_1 = (*rooms)[index].p.y - 3, Y_2 = (*rooms)[index].p.y + (*rooms)[index].width + 1 + 3;
    for (int i = 0; i < index; i++) {
        int x_1 = (*rooms)[i].p.x - 3, x_2 = (*rooms)[i].p.x + (*rooms)[i].height + 1 + 3;
        int y_1 = (*rooms)[i].p.y - 3, y_2 = (*rooms)[i].p.y + (*rooms)[i].width + 1 + 3;
        if (X_1 <= x_2 && Y_1 <= y_2 && x_1 <= X_2 && y_1 <= Y_2)
            return 0;
    }
    return 1;
}

void corridor_at_point(char ***map, int theme[MAX_SIZE][MAX_SIZE], struct Point p) {
    if ((*map)[p.x][p.y] == ' ') {
        (*map)[p.x][p.y] = '#';
        theme[p.x][p.y] = 'r';
    }
    else if ((*map)[p.x][p.y] == '_' || (*map)[p.x][p.y] == '|')
        (*map)[p.x][p.y] = '+';
}

void create_corridor(struct User* user, struct Point st, struct Point en, int level) {
    while (st.x != en.x || st.y != en.y) {
        int dir = rand() % 4; // 0: up, 1: right, 2: down, 3: left
        struct Point nxt = next_point(st, dir);
        if ((dir == 0 && st.x > en.x) || (dir == 1 && st.y < en.y) || (dir == 2 && st.x < en.x) || (dir == 3 && st.y > en.y)) {
            if (is_in_map(nxt)) {
                corridor_at_point(&user->map[level], user->theme[level], nxt);
                st.x = nxt.x, st.y = nxt.y;
            }
        }
    }
}

void destroy_door(char ***map, struct Point p) {
    if ((p.x > 0 && is_in_room(map, create_point(p.x - 1, p.y))) || (p.x + 1 < GAME_X && is_in_room(map, create_point(p.x + 1, p.y))))
        (*map)[p.x][p.y] = '_';
    else
        (*map)[p.x][p.y] = '|';
}

void trim_rooms(char ***map) {
    // remove multiple doors
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            if ((*map)[i][j] == '+') {
                int valid = 0;
                for (int dir = 0; dir < 4; dir++) {
                    struct Point nxt = next_point(create_point(i, j), dir);
                    if (is_in_map(nxt) && (*map)[nxt.x][nxt.y] == '#')
                        valid = 1;
                }
                if (vertical_neighbor(map, create_point(i, j)) || horizontal_neighbor(map, create_point(i, j)))
                    valid = 1;
                if (!valid)
                    destroy_door(map, create_point(i, j));
            }
}

void clear_map(char ***map) {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            (*map)[i][j] = ' ';
}

void determine_initial_position(struct User* user, char ***map) {
    do {
        user->pos.x = rand() % GAME_X;
        user->pos.y = rand() % GAME_Y;
    } while ((*map)[user->pos.x][user->pos.y] != '.');
}

void generate_pillar(char ***map, struct Room* room) {
    int number_of_pillars = rand() % (2 + DIFFICULTY);
    while (number_of_pillars) {
        while (true) {
            int x = room->p.x + 2 + rand() % (room->height - 2);
            int y = room->p.y + 2 + rand() % (room->width - 2);
            if ((*map)[x][y] == '.') {
                (*map)[x][y] = 'O';
                --number_of_pillars;
                break;
            }
        }
    }
}

void generate_traps(struct User* user, struct Room* room, int level) {
    char ***map = &(user->map[level]);
    int number_of_traps;
    if (room->type != 't')
        number_of_traps = DIFFICULTY + rand() % 2;
    else
        number_of_traps = 10 + DIFFICULTY;
    while (number_of_traps) {
        struct Point pos = create_point(room->p.x + rand() % room->height, room->p.y + rand() % room->width);
        if ((*map)[pos.x][pos.y] == '.' && !user->trap[level][pos.x][pos.y].exist) {
            user->trap[level][pos.x][pos.y].exist = 1;
            user->trap[level][pos.x][pos.y].damage = 1 + rand() % 3;
            --number_of_traps;
        } 
    }
}

void generate_staircase(char ***map, struct Point* p) {
    struct Point tmp;
    do {
        tmp.x = rand() % GAME_X;
        tmp.y = rand() % GAME_Y;
    } while ((*map)[tmp.x][tmp.y] != '.');
    (*map)[tmp.x][tmp.y] = '>';
    p->x = tmp.x, p->y = tmp.y;
}

void generate_gold(struct User* user, char ***map, struct Room* room, int level) {
    if (room->type == 'r' || room->type == 'n') {
        int has_gold = rand() % (2 + DIFFICULTY);
        while (!has_gold) {
            int x = room->p.x + 1 + rand() % room->height;
            int y = room->p.y + 1 + rand() % room->width;
            if ((*map)[x][y] == '.') {
                (*map)[x][y] = 'g';
                --has_gold;
                int num = 1 + rand() % 4;
                int p = rand() % 5; // 0: black gold, else: normal
                if (p == 0) {
                    (user->gold)[level][x][y].type = 'b';
                    (user->gold)[level][x][y].cnt = 3 * num;
                }
                else {
                    (user->gold)[level][x][y].type = 'n';
                    (user->gold)[level][x][y].cnt = num;
                }
            }
        }
    }
    else if (room->type == 't') {
        int number_of_golds = 20 - 3 * DIFFICULTY;
        for (int i = 0; i < number_of_golds; i++) {
            while (true) {
                int x = room->p.x + 1 + rand() % room->height;
                int y = room->p.y + 1 + rand() % room->width;
                if ((*map)[x][y] == '.') {
                    (*map)[x][y] = 'g';
                    int num = 1 + rand() % 4;
                    int p = rand() % 5; // 0: black gold, else: normal
                    if (p == 0) {
                        (user->gold)[level][x][y].type = 'b';
                        (user->gold)[level][x][y].cnt = 3 * num;
                    }
                    else {
                        (user->gold)[level][x][y].type = 'n';
                        (user->gold)[level][x][y].cnt = num;
                    }
                    break;
                }
            }
        }
    }
}

int get_doors(struct Point **p, char ***map, struct Room* room) {
    int num = 0;
    for (int j = 0; j < room->width + 2; j++) {
        if ((*map)[room->p.x][room->p.y + j] == '+') {
            (*p)[num].x = room->p.x;
            (*p)[num].y = room->p.y + j;
            ++num;
        }
        if ((*map)[room->p.x + room->height + 1][room->p.y + j] == '+') {
            (*p)[num].x = room->p.x + room->height + 1;
            (*p)[num].y = room->p.y + j;
            ++num;
        }
    }
    for (int i = 0; i < room->height + 2; i++) {
        if ((*map)[room->p.x + i][room->p.y] == '+') {
            (*p)[num].x = room->p.x + i;
            (*p)[num].y = room->p.y;
            ++num;
        }
        if ((*map)[room->p.x + i][room->p.y + room->width + 1] == '+') {
            (*p)[num].x = room->p.x + i;
            (*p)[num].y = room->p.y + room->width + 1;
            ++num;
        }
    }
    return num;
}

void create_secret_doors(struct User* user, char ***map, struct Room* room, int level) {
    struct Point* p = malloc(sizeof(struct Point) * 10);
    int num = get_doors(&p, map, room);
    if (room->type == 'e') { // enchanted room
        for (int i = 0; i < num; i++) {
            user->door[level][p[i].x][p[i].y].exist = 1;
            destroy_door(map, p[i]);
        }
    }
    else if (num == 1) { // (Bon Bast)
        user->door[level][p[0].x][p[0].y].exist = 1;
        destroy_door(map, p[0]);
    }
}

void generate_food(struct User* user, char ***map, struct Room* room, int level) {
    if (room->type != 't' && room->type != 'e') {
        int number_of_food = rand() % (5 - DIFFICULTY);
        while (number_of_food) {
            int x = room->p.x + 1 + rand() % room->height;
            int y = room->p.y + 1 + rand() % room->width;
            if ((*map)[x][y] == '.') {
                (*map)[x][y] = 'f';
                --number_of_food;
                int type = rand() % 11;
                if (type <= 4) // normal
                    user->info[level][x][y] = 1;
                else if (type <= 6) // aala
                    user->info[level][x][y] = 2;
                else if (type <= 8) // jadooee
                    user->info[level][x][y] = 3;
                else // fased
                    user->info[level][x][y] = 4;
                
            }
        }
    }
}

void generate_weapon(struct User *user, char ***map, struct Room* room, int level) {
    if (room->type == 'e')
        return;
    int cnt[] = {1, 10, 8, 20, 1};
    for (int i = 0; i < 5; i++) {
        if (rand() % 5 == 0) {
            while (true) {
                int x = room->p.x + 1 + rand() % room->height;
                int y = room->p.y + 1 + rand() % room->width;
                if ((*map)[x][y] == '.') {
                    (*map)[x][y] = wEAPON[i];
                    user->info[level][x][y] = cnt[i];
                    break;
                }
            }
        }
    }
}

void generate_potion(char ***map, struct Room* room) {
    if (room->type == 'e') { // enchant
        int number_of_potion = 4 + rand() % 3;
        while (number_of_potion) {
            int x = room->p.x + 1 + rand() % room->height;
            int y = room->p.y + 1 + rand() % room->width;
            if ((*map)[x][y] == '.') {
                int type = rand() % 3;
                switch(type) {
                    case 0: // health
                        (*map)[x][y] = 'h';
                        break;
                    case 1: // speed
                        (*map)[x][y] = 's';
                        break;
                    case 2: // damage
                        (*map)[x][y] = 'd';
                        break;
                }
                --number_of_potion;
            }
        }
    }
    else if (room->type != 't') { // normal and nightmare
        if (rand() % 5 == 0) {
            while (true) {
                int x = room->p.x + 1 + rand() % room->height;
                int y = room->p.y + 1 + rand() % room->width;
                if ((*map)[x][y] == '.') {
                    int type = rand() % 3;
                    switch(type) {
                        case 0: // health
                            (*map)[x][y] = 'h';
                            break;
                        case 1: // speed
                            (*map)[x][y] = 's';
                            break;
                        case 2: // damage
                            (*map)[x][y] = 'd';
                            break;
                    }
                }
                break;
            }
        }
    }
}

void generate_enemy(struct User* user, struct Room* room, int level) {
    char enemy_name[] = {'D', 'F', 'G', 'S', 'U'};
    if (room->type == 't') {
        for (int i = 0; i < 5; i++) {
            int num;
            if (i == 3)
                num = 2 + rand() % 2;
            else
                num = 1 + rand() % 3;
            while (num) {
                int x = room->p.x + 1 + rand() % room->height;
                int y = room->p.y + 1 + rand() % room->width;
                if (user->map[level][x][y] == '.' && (user->pos.x != x || user->pos.y != y)) {
                    user->map[level][x][y] = enemy_name[i];
                    --num;
                }
            }
        }
    }
    else {
        for (int i = 0; i < 5; i++) {
            if (rand() % (3 - DIFFICULTY + 2 * i) == 0) {
                while (true) {
                    int x = room->p.x + 1 + rand() % room->height;
                    int y = room->p.y + 1 + rand() % room->width;
                    if (user->map[level][x][y] == '.') {
                        user->map[level][x][y] = enemy_name[i];
                        break;
                    }
                }
            }
        }
    }
}

void create_window(char ***map, struct Room* room) {
    int side = rand() % 4;
    if (side == 0) { // up
        for (int j = 0; j < room->width; j++)
            if (is_wall((*map)[room->p.x][room->p.y + j + 1])) {
                (*map)[room->p.x][room->p.y + 1 + j] = '=';
                break;
            }
    }
    else if (side == 1) { // right
        for (int i = 0; i < room->height; i++)
            if (is_wall((*map)[room->p.x + 1 + i][room->p.y + room->width + 1])) {
                (*map)[room->p.x + 1 + i][room->p.y + room->width + 1] = '=';
                break;
            }
    }
    else if (side == 2) { // down
        for (int j = 0; j < room->width; j++)
            if (is_wall((*map)[room->p.x + room->height + 1][room->p.y + 1 + j])) {
                (*map)[room->p.x + room->height + 1][room->p.y + 1 + j] = '=';
                break;
            }
    }
    else { // left
        for (int i = 0; i < room->height; i++)
            if (is_wall((*map)[room->p.x + 1 + i][room->p.y])) {
                (*map)[room->p.x + 1 + i][room->p.y] = '=';
                break;
            }
    }
}

void add_items(struct User* user, char ***map, struct Room* room, int level) { // add items to room
    generate_pillar(map, room);
    generate_traps(user, room, level);
    generate_gold(user, map, room, level);
    generate_food(user, map, room, level);
    generate_weapon(user, map, room, level);
    generate_potion(map, room);
    generate_enemy(user, room, level);
    create_window(map, room);
    create_secret_doors(user, map, room, level);
}

void add_treasure(char ***map, struct Room *room) {
    while (true) {
        int x = room->p.x + 1 + rand() % room->height;
        int y = room->p.y + 1 + rand() % room->width;
        if ((*map)[x][y] == '.') {
            (*map)[x][y] = 'T';
            break;
        }
    }
}

void create_battle_room(struct User* user, char ***map, struct Enemy enemy[GAME_X][GAME_Y]) {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            (*map)[i][j] = ' ';
    int height = 4 + rand() % 3, width = 10 + rand() % 15;
    int st_x = rand() % (GAME_X - height - 2), st_y = rand() % (GAME_Y - width - 2);
    user->pos = create_point(st_x + 1, st_y + 1);
    for (int j = 0; j < width + 2; j++)
        (*map)[st_x][st_y + j] = '_';
    for (int j = 1; j < width + 1; j++)
        (*map)[st_x + height + 1][st_y + j] = '_';
    for (int i = 1; i < height + 2; i++)
        (*map)[st_x + i][st_y] = (*map)[st_x + i][st_y + width + 1] = '|';
    for (int i = 1; i < height + 1; i++)
        for (int j = 1; j < width + 1; j++)
            (*map)[st_x + i][st_y + j] = '.';
    char enemy_name[] = {'D', 'F', 'G', 'S', 'U'};
    for (int i = 0; i < 5; i++) {
        if (rand() % (5 - DIFFICULTY) == 0) {
            int num = 1 + rand() % 3;
            while (num) {
                int x = st_x + 1 + rand() % height;
                int y = st_y + 1 + rand() % width;
                if ((*map)[x][y] == '.' && (user->pos.x != x || user->pos.y != y)) {
                    (*map)[x][y] = enemy_name[i];
                    enemy[x][y].health = get_enemy_health(enemy_name[i]);
                    enemy[x][y].moves = (i != 3? -1: get_enemy_moves(enemy_name[i]));
                    --num;
                }
            }
        }
    }
}

void generate_map(struct User* user) {
    clear();
    print_message_with_color(LINES / 3, COLS / 3 - 10, "It may take up to 90 seconds. Thank you for your patience.", 3);
    refresh();
    time_t st;
    time(&st);
    user->number_of_floor = 4 + rand() % 2;
    ++user->number_of_games;
    user->level = 0;
    struct Point staircaise;
    for (int level = 0; level < user->number_of_floor; level++) {
        init_user(user, level);
        char ***map = &(user->map[level]);
        int valid_map = 1;
        number_of_rooms = 6 + rand() % 2;
        struct Room* rooms = malloc(sizeof(struct Room) * number_of_rooms);
        corners = malloc(sizeof(struct Point) * number_of_rooms * 4);
        do {
            time_t now;
            time(&now);
            if (difftime(now, st) > 90) {
                clear();
                print_message_with_color(LINES / 3, COLS / 3, "Failed to generate map!", 2);
                print_message_with_color(LINES / 3 + 2, COLS / 3, "Press any key to regenerate ...", 3);
                refresh();
                getch();
                clear();
                refresh();
                generate_map(user);
                return;
            }
            valid_map = 1;
            clear_map(map);
            for (int i = 0; i < number_of_rooms && valid_map; i++) {
                int height = 4 + rand() % 3, width = 4 + rand() % 15;
                int x = rand() % (GAME_X - height - 1), y = rand() % (GAME_Y - width - 1);
                create_room(user, map, create_point(x, y), height, width, &rooms, i, level);
                valid_map &= check_rooms_dist(&rooms, i);
            }
            if (valid_map) {
                for (int i = 0; i < number_of_rooms; i++)
                    draw_room(map, &rooms[i]);
            }
            for (int i = 0; i < number_of_rooms - 1 && valid_map; i++) {
                int delta_x = rand() % (rooms[i].height - 1), delta_y = rand() % (rooms[i].width - 1);
                struct Point p1 = create_point(rooms[i].p.x + 1 + delta_x, rooms[i].p.y + 1 + delta_y);
                delta_x = rand() % (rooms[i + 1].height - 1), delta_y = rand() % (rooms[i + 1].width - 1);
                struct Point p2 = create_point(rooms[i + 1].p.x + 1 + delta_x, rooms[i + 1].p.y + 1 + delta_y);
                create_corridor(user, p1, p2, level);
            }
            if (level > 0) {
                valid_map &= (*map)[staircaise.x][staircaise.y] == '.';
                (*map)[staircaise.x][staircaise.y] = '<';
            }
            valid_map &= check_corners(map);
        } while (!valid_map);
        trim_rooms(map);
        if (level == user->number_of_floor - 1) { // add treasure
            int index = rand() % number_of_rooms;
            add_treasure(map, &rooms[index]);
        }
        if (level != user->number_of_floor - 1)
            generate_staircase(map, &staircaise);
        for (int i = 0; i < number_of_rooms; i++) {
            update_theme(user, level, &rooms[i]);
            add_items(user, map, &rooms[i], level);
        }
        if (level == 0)
            determine_initial_position(user, map);
        // determine_doors_type(user, map);
    }
}

void create_treasure_room(struct User* user) {
    // init map
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            user->map[user->level][i][j] = ' ';
            user->mask[user->level][i][j] = 0;
        }
    // create map
    struct Room room;
    room.type = 't';
    room.p = create_point(GAME_X / 4,GAME_Y / 4);
    room.height = GAME_X / 2; 
    room.width = GAME_Y / 2;
    draw_room(&user->map[user->level], &room);
    update_theme(user, user->level, &room);
    user->pos = create_point(room.p.x + room.height / 2, room.p.y + room.width / 2);
    generate_traps(user, &room, user->level);
    generate_gold(user, &user->map[user->level], &room, user->level);
    generate_enemy(user, &room, user->level);
}

void print_map(struct User* user, int reveal) {
    print_message_with_color(LINES - 1, 0, "Press (Q) to exit game.", 2);
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            int c = user->theme[user->level][i][j];
            int color;
            switch (c) {
            case 'r':
                color = 1;
                break;
            case 'e':
                color = 5;
                break;
            case 'n':
                color = 6;
                break;
            case 't':
                color = 7;
                break;
            default:
                color = 1;
                break;
            }
            if ((user->mask)[user->level][i][j] || reveal) {
                if ((user->map)[user->level][i][j] == 'g') {
                    if ((user->gold)[user->level][i][j].type == 'n')
                        print_character_with_color(i + ST_X, j + ST_Y, 'G', 8); // 🜚
                    else if ((user->gold)[user->level][i][j].type == 'b')
                        print_character_with_color(i + ST_X, j + ST_Y, 'G', 5);
                }
                else if ((user->map)[user->level][i][j] == 'f') {
                    if (user->info[user->level][i][j] == 1)
                        print_character_with_color(i + ST_X, j + ST_Y, 'f', 3);
                    else if (user->info[user->level][i][j] == 2)
                        print_character_with_color(i + ST_X, j + ST_Y, 'f', 7);
                    else if (user->info[user->level][i][j] == 3)
                        print_character_with_color(i + ST_X, j + ST_Y, 'f', 5);
                    else if (user->info[user->level][i][j] == 4)
                        print_character_with_color(i + ST_X, j + ST_Y, 'f', 6);
                }
                else if (is_weapon((user->map)[user->level][i][j])) {
                    char c = (user->map)[user->level][i][j];
                    if (c == 'm') {
                        print_message_with_color(i + ST_X, j + ST_Y, "⚒", 6);
                    }
                    else if (c == 'a') {
                        print_message_with_color(i + ST_X, j + ST_Y, "🗡", 6);
                    }
                    else if (c ==  'M') {
                        print_message_with_color(i + ST_X, j + ST_Y, "\u269A", 6);
                    }
                    else if (c == 'n') {
                        print_message_with_color(i + ST_X, j + ST_Y, "➳", 6);
                    }
                    else {
                        print_message_with_color(i + ST_X, j + ST_Y, "⚔", 6);
                    }
                }
                else if (is_enemy((user->map[user->level][i][j]))) {
                    print_character_with_color(i + ST_X, j + ST_Y, (user->map)[user->level][i][j], 2);
                }
                else if (is_potion((user->map)[user->level][i][j])) {
                    if ((user->map)[user->level][i][j] == 'h') {
                        print_message_with_color(i + ST_X, j + ST_Y, "\u2695", 4);
                    }
                    else if ((user->map)[user->level][i][j] == 's') {
                        print_message_with_color(i + ST_X, j + ST_Y, "\u26f7", 4);
                    }
                    else {
                        print_message_with_color(i + ST_X, j + ST_Y, "\u2620", 4);
                    }
                    // print_character_with_color(i + ST_X, j + ST_Y, (user->map)[user->level][i][j], 4);
                }
                else if ((user->map)[user->level][i][j] == 'T') {
                    print_message_with_color(i + ST_X, j + ST_Y, "🜚", 8);
                }
                else if (user->map[user->level][i][j] == '#') {
                    print_message_with_color(i + ST_X, j + ST_Y, "#", 4);
                }
                else
                    print_character_with_color(i + ST_X, j + ST_Y, (user->map)[user->level][i][j], color);
            }
            else
                mvprintw(i + ST_X, j + ST_Y, " ");
        }
}

void print_map_trap(char ***map) {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            if (is_enemy((*map)[i][j])) {
                print_character_with_color(i + ST_X, j + ST_Y, (*map)[i][j], 2);
            }
            else if (is_weapon((*map)[i][j])) {
                char c = (*map)[i][j];
                if (c == 'm') {
                    print_message_with_color(i + ST_X, j + ST_Y, "⚒", 6);
                }
                else if (c == 'a') {
                    print_message_with_color(i + ST_X, j + ST_Y, "🗡", 6);
                }
                else if (c ==  'M') {
                    print_message_with_color(i + ST_X, j + ST_Y, "\u269A", 6);
                }
                else if (c == 'n') {
                    print_message_with_color(i + ST_X, j + ST_Y, "➳", 6);
                }
                else {
                    print_message_with_color(i + ST_X, j + ST_Y, "⚔", 6);
                }
            }
            else
                mvprintw(i + ST_X, j + ST_Y, "%c", (*map)[i][j]);
        }
}

void print_hunger(int x, int y, struct User* user) {
    print_message_with_color(x, y, "Your Hunger:[", 2);
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2));
    for (int i = 0; i < user->hunger; i++)
        mvprintw(x, y + 13 + i, "#");
    for (int i = user->hunger; i < 10; i++)
        mvprintw(x, y + 13 + i, ".");
    mvprintw(x, y + 13 + 10, "] (%d out of 10)", user->hunger);
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(1));
}

void print_status(struct User* user) {
    move(GAME_X + ST_X, GAME_Y / 3);
    change_color(4);
    printw("Score: %d \t Gold: %d \t Health: %d \t Games: %d", user->score, user->golds, user->health, user->number_of_games);
    undo_color(4);
    print_hunger(GAME_X + ST_X + 1, GAME_Y / 3 + 1, user);
}

void print_user_list(struct miniUser** user_list, int st, int sz, int cur, int user_pos) {
    clean_area(create_point(ST_X + 2, 0), create_point(GAME_X, GAME_Y));
    for (int i = 0; i < sz; i++) {
        if (i == cur)
            attron(A_REVERSE | A_BOLD);
        int color = 5;
        if (i + st < 3) {
            color = 3;
            attron(A_ITALIC);
        }
        if (user_pos == i + st)
            print_message_with_color(ST_X + 2 * i + 2, 0, "-->", color);
        char name[MAX_SIZE];
        strcpy(name, user_list[i + st]->username);
        switch (i + st) {
        case 0:
            strcat(name, " (legend)");
            mvprintw(ST_X + 2 * i + 2, ST_Y + 10 + strlen(name), "🥇");
            break;
        case 1:
            strcat(name, " (goat)");
            mvprintw(ST_X + 2 * i + 2, ST_Y + 10 + strlen(name), "🥈");
            break;
        case 2:
            strcat(name, " (khafan)");
            mvprintw(ST_X + 2 * i + 2, ST_Y + 10 + strlen(name), "🥉");
            break;
        default:
            break;
        }
        print_number_with_color(ST_X + 2 * i + 2, ST_Y, i + st + 1, color);
        print_message_with_color(ST_X + 2 * i + 2, ST_Y + 10, name, color);
        print_number_with_color(ST_X + 2 * i + 2, ST_Y + 30, user_list[i + st]->score, color);
        print_number_with_color(ST_X + 2 * i + 2, ST_Y + 40, user_list[i + st]->gold, color);
        print_number_with_color(ST_X + 2 * i + 2, ST_Y + 50, user_list[i + st]->number_of_games, color);
        print_message_with_color(ST_X + 2 * i + 2, ST_Y + 60, get_time(user_list[i + st]->first_game), color);
        if (i + st < 3)
            attroff(A_ITALIC);
        if (i == cur)
            attroff(A_REVERSE | A_BOLD);
    }
    refresh();
}