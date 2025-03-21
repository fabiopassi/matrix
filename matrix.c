#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <time.h>

/************************************************/
/*       Script for matrix-like terminal        */
/************************************************/

/* Global variables */
int quit = 0;


/* Constants */
#define NUM_COL_PAIRS 6
#define LEN_COLOR 2
#define LEN_STRING 12


/* Typedef */

typedef struct {
    int x,y;
    char c;
} MyChar;


typedef struct {
    MyChar chars[LEN_STRING];
} MyString;


typedef struct Node {
    MyString val;
    struct Node* next;
} Node;



/* Functions */

void handler(int sig) {
    quit = 1;
}



void add_string(Node** head, int x_max, int num_avail_chars, char* available_chars) {

    Node* new = malloc(sizeof(*new));
    
    int x_pos = rand() % (x_max + 1);
    int chosen_char = 0;
    for (int i = 0; i < LEN_STRING; i++) {
        if (i % 2 == 0) { chosen_char = rand() % (num_avail_chars + 1); }
        new->val.chars[i].c = available_chars[chosen_char];
        new->val.chars[i].x = x_pos;
        new->val.chars[i].y = -i;
    }

    if (*head == NULL) {
        new->next = NULL;
    } else {
        new->next = *head;
    }

    *head = new;
}



int remove_last_element(Node** head) {

    Node* current = *head;
    Node* prev = NULL;

    if(current == NULL) {
        return 0;
    }

    while(current->next != NULL) {
        prev = current;
        current = current->next;
    }

    if(prev == NULL) {
        *head = NULL;
    } else {
        prev->next = current->next;
    }

    free(current);
    return 1;

}



void move_string(MyString* string) {
    for (int i = 0; i < LEN_STRING; i++) {
        string->chars[i].y += 1;
    }
}



void print_string(MyString* string, int y_max) {
    for (int i = 0; i < LEN_STRING; i++) {
        
        attron(COLOR_PAIR(i/LEN_COLOR + 1));

        if (string->chars[i].y >= 0 && string->chars[i].y <= y_max) {
            mvprintw(string->chars[i].y, string->chars[i].x, "%c", string->chars[i].c);
        }

        attroff(COLOR_PAIR(i/LEN_COLOR + 1));

        if ( i == LEN_STRING - 1 && string->chars[i].y >= 1 && string->chars[i].y <= y_max + 1 ) {
            mvaddch(string->chars[i].y - 1, string->chars[i].x, ' ');
        }
    
    }
}


/* Main */

int main(int argc, char** argv) {

    srand(time(NULL));

    /* ncurses settings and colors (many greens and white) */
    initscr();

    nodelay(stdscr, TRUE);
    noecho();
    curs_set(0);
    use_default_colors();

    if (! has_colors() == TRUE) {
        printf("Your terminal does not support colors.\n\n");
        endwin();
        exit(EXIT_FAILURE);
    }

    if (start_color() != OK) {
        printf("Error: something went wrong activating colors.\n\n");
        endwin();
        exit(EXIT_FAILURE);
    }

    /* Colors : different shades of green */
    init_color(27, 0, 900, 0);
    init_color(28, 40, 800, 30);
    init_color(29, 50, 666, 50);
    init_color(30, 100, 400, 100);
    init_color(31, 183, 308, 183);

    /* Color pairs (NOTE: -1 is default bkg) */
    init_pair(1, COLOR_WHITE, -1);
    init_pair(2, 27, -1);
    init_pair(3, 28, -1);
    init_pair(4, 29, -1);
    init_pair(5, 30, -1);  
    init_pair(6, 31, -1);

    /* Getting terminal info */
    int x_max, y_max;
    getmaxyx(stdscr, y_max, x_max);

    /* Settings for characters contained in strings and new number of strings per iterations */
    char available_chars[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890+-.,:;/!?^$&()=[]{}@#*";
    int num_avail_chars = strlen(available_chars);
    const int new_strings_per_cycle = 6;
    enum Boolean {False, True};
    enum Boolean remove = False;

    /* Define the head of the linked list containing out strings */
    Node* head = NULL;

    /* In case of ^C, the handler function is called and the program leaves the while loop below */
    signal(SIGINT, handler);

    while (! quit) {
        
        /* Generate new sequences of characters */
        for (int n = 0; n < new_strings_per_cycle; n++) {
            add_string(&head, x_max, num_avail_chars, available_chars);
        }

        Node* tmp = head;
        
        while (tmp->next != NULL) {

            /* Move actual characters */
            move_string(&(tmp->val));

            /* Print on buffer */
            print_string(&(tmp->val), y_max);

            tmp = tmp->next;
        }
        /* Moving and printing the last element */
        move_string(&(tmp->val));
        print_string(&(tmp->val), y_max);

        /* If I am not already removing the elements (because no string scrolled the whole screen), then I check if any string has now scrolled the whole screen */
        if ( remove == False && tmp->val.chars[LEN_STRING-1].y > y_max ) {
            remove = True;
        }

        /* Print on stdscr and wait */
        refresh();
        napms(80);

        /* Remove the elements outside of the screen */
        for (int i = 0; remove == True && i < new_strings_per_cycle; i++) {
            remove_last_element(&head);
        }

        /* Check for quit */
        int key = getch();
        if (key == 'q') { quit = 1; }
    }

    endwin();

    /* Free memory */
    while ( remove_last_element(&head) ) {}
    

    return 0;
}
