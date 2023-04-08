#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int highlight = 1;
int choice = 0;
int c;
char *menu_items[] = {
        "  Add an event",
        "  Delete an event",
        "  View your calendar",
        "  Search by date",
        "  Exit"
    };

int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
typedef struct {
    char date[80];
    char hour[80];
    char name[80];
    char place[80];
} meeting;
char* get_current_datetime() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    static char datetime[80];
    strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M", tm);
    return datetime;
}

int is_valid_date(char *date) {
    // Get the current time
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    // Add one month to the current time
    tm->tm_mon += 1;
    time_t one_month_from_now = mktime(tm);

    // Convert the entered date to a time_t
    struct tm entered_date = {0};
    strptime(date, "%Y-%m-%d", &entered_date);
    time_t entered_time = mktime(&entered_date);

    // Check if the entered date is more than a month from now
    return difftime(entered_time, one_month_from_now) <= 0;
}

void main_menu(int hi){
        clear();
        char *datetime = get_current_datetime();
        printw("============ Main menu ============\n");
        printw("=== Use the arrows to navigate ====\n");
        printw("========================================\n");
        printw("============%s============\n",datetime);
        printw("========================================\n\n");
        for (int i = 0; i < menu_size; i++) {
            if (i + 1 == hi) {
                attron(A_REVERSE);//highlight la alegere
            }
            printw("%s\n",menu_items[i]);
            attroff(A_REVERSE);//hz
        }
        return;
}
int choose(){

    while (1) {
        main_menu(highlight);
        c = getch();
        switch(c) {
            case 'w':
            case KEY_UP:
                if (highlight > 1)
                    highlight--;
                break;
            case 's':
            case KEY_DOWN:
                if (highlight < menu_size)
                    highlight++;
                    choice++;
                break;
            case 'd':
            case KEY_RIGHT:
                if (highlight < menu_size)
                    highlight++;
                break;
            case 'a':
            case KEY_LEFT:
                if (highlight > 1)
                    highlight--;
                break;
             case '\n':
            case KEY_ENTER:
                choice = highlight;

                return choice;
        }
    }
    return 0;
}

void add_meeting(){
    clear();
    echo();
    meeting m;
    attron(COLOR_PAIR(1));
    mvprintw(1,15,"================================================\n");
    mvprintw(2,15,"============= Enter the event date =============\n");
    mvprintw(3,15,"====== The format should be : YYYY-MM-DD =======\n");
    mvprintw(4,15,">>> ");
    getstr(m.date);
    if (!is_valid_date(m.date)) {
        mvprintw(5,15,"I don't like the entered date\n");
        getch();
        return;
    }
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2));
    mvprintw(6,15,"========== Enter the event hour ===========\n");
    mvprintw(7,15,">>> ");
    getstr(m.hour);
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(3));
    mvprintw(8,15,"========== Enter the event name =========\n");
    mvprintw(9,15,">>> ");
    getstr(m.name);
    attroff(COLOR_PAIR(3));
    mvprintw(10,15,"========== Enter the place of the event=========\n");
    mvprintw(11,15,">>> ");
    getstr(m.place);

    // Open the text file in append mode
    FILE *fp = fopen("events.txt", "a");
    if (fp == NULL) {
        printw("Error opening file!\n");
        return;
    }

    // Write the event details to the file
    fprintf(fp, "%s %s %s %s\n", m.date, m.hour, m.name, m.place);

    // Close the file
    fclose(fp);

   mvprintw(11,15," Press Backspace to go to main menu ");
   int c = getch();
   if(c == KEY_BACKSPACE)
       choose();

   return;
}

void delete_meeting(){
    clear();
    echo();
    char name[80];
    int found = 0;
    mvprintw(1,15,"================================================\n");
    mvprintw(2,15,"============== Your Events ======\n");

    // Open the text file in read mode
    FILE *fp = fopen("events.txt", "r");
    if (fp == NULL) {
        printw("Error opening file!\n");
        return;
    }

    // Read and display each event from the file
    int line = 4;
    meeting m;
    while (fscanf(fp, "%s %s %s %s\n", m.date, m.hour, m.name, m.place) == 4) {
        mvprintw(line++, 15, "Date: %s Hour: %s Name: %s Place: %s\n", m.date, m.hour, m.name, m.place);
    }

   // Close the file
   fclose(fp);
    mvprintw(line + 1,15,"============== What do you want to delete? ==============\n");
    mvprintw(line + 2,15,"Write the event name or press ~ to exit \n");
   while (!found) {

       mvprintw(line + 3,15,">>>");
       getstr(name);

       if (strcmp(name, "~") == 0) {
           choose();
           return;
       }

       // Open the text file in read mode
       fp = fopen("events.txt", "r");
       if (fp == NULL) {
           printw("Error opening file!\n");
           return;
       }

       // Check if the event name exists in the text file
       while (fscanf(fp, "%s %s %s %s\n", m.date, m.hour, m.name, m.place) == 4) {
           if (strcmp(m.name, name) == 0) {
               found = 1;
               break;
           }


       // Close the file
       fclose(fp);

       if (!found) {
           mvprintw(line + 7,15,"The entered name wasn't found, please retry.\n");
           line++;
       }
   }
}
   // Create a temporary file to store the updated events
   FILE *tmp = fopen("tmp.txt", "w");
   if (tmp == NULL) {
       printw("Error creating temporary file!\n");
       fclose(fp);
       return;
   }

   // Open the text file in read mode
   fp = fopen("events.txt", "r");
   if (fp == NULL) {
       printw("Error opening file!\n");
       fclose(tmp);
       return;
   }

   // Read each event from the text file
   while (fscanf(fp, "%s %s %s %s\n", m.date, m.hour, m.name, m.place) == 4) {
       // If the event name doesn't match the one to be deleted,
       // write it to the temporary file
       if (strcmp(m.name, name) != 0) {
           fprintf(tmp, "%s %s %s %s\n", m.date, m.hour, m.name, m.place);
       }
   }

   // Close both files
   fclose(fp);
   fclose(tmp);

   // Delete the original text file and rename the temporary file
   remove("events.txt");
   rename("tmp.txt", "events.txt");
    mvprintw(line+6,15," Event '%s' was deleted",name);
   mvprintw(line + 7 ,15,"  Press Backspace to go to main menu ");
   int c = getch();
   if(c == KEY_BACKSPACE)
       choose();

   return;
}


void view(){
    clear();
    echo();
    char str[80];
    mvprintw(1,15,"================================================\n");
    mvprintw(2,15,"============== Your events ======\n");

    // Open the text file in read mode
    FILE *fp = fopen("events.txt", "r");
    if (fp == NULL) {
        printw("Error opening file!\n");
        return;
    }

    // Read and display each event from the file
    int line = 4;
    meeting m;
    while (fscanf(fp, "%s %s %s %s\n", m.date, m.hour, m.name, m.place) == 4) {
        mvprintw(line++, 15, "Date: %s Hour: %s Name: %s Place: %s\n", m.date, m.hour, m.name, m.place);
    }

   // Close the file
   fclose(fp);

   int j = getch();
   while(j != KEY_ENTER){
       getstr(str);
       break;
   }

   mvprintw(line + 2, 15," Press Backspace to go to main menu ");
   int c = getch();
   if(c == KEY_BACKSPACE)
       choose();

   return;
}
void search_meetings(){
    clear();
    echo();
    char date[80];
    mvprintw(1,15,"================================================\n");
    mvprintw(2,15,"================ Search events =================\n");
    mvprintw(3,15,"Enter the date of the searched event (YYYY-MM-DD):\n");
    mvprintw(4,15,">>>");
    getstr(date);

    // Open the text file in read mode
    FILE *fp = fopen("events.txt", "r");
    if (fp == NULL) {
        printw("Error opening file!\n");
        return;
    }

    // Read and display each event from the file
    int line = 6;
    meeting m;
    int flag = 0;
    while (fscanf(fp, "%s %s %s %s\n", m.date, m.hour, m.name, m.place) == 4) {
        if (strcmp(m.date, date) == 0) {
            mvprintw(line++, 15, "Date: %s Hour: %s Name: %s Place: %s\n", m.date, m.hour, m.name, m.place);
            flag = 1;
        }

    }
    if(flag == 0){
    mvprintw(line,15,"We didn't find any events at this date :(((");
    }

   // Close the file
   fclose(fp);
   mvprintw(line + 1, 15," Press Backspace to go to main menu ");
   int c = getch();
   if(c == KEY_BACKSPACE)
       choose();

   return;
}
int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    keypad(stdscr, TRUE);
    int aleg = choose();
    while (aleg != 0) {
        switch(aleg) {
            case 1:
                add_meeting();
                break;
            case 2:
                delete_meeting();
                break;
            case 3:
                view();
                break;
            case 4:
                search_meetings();
                break;
            case 5:
                endwin();
                exit(0);
            default:
                break;
        }
        aleg = choose();
    }


    return 0;
}
