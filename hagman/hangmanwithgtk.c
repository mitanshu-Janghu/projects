#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ================= STRUCTS ================= */

struct WordData {
    char word[30];
    char hint[100];
};

struct User {
    char username[30];
    char password[30];
    int gamesPlayed;
    int gamesWon;
    int gamesLost;
    int totalScore;
};

struct GameState {
    char word[50];
    char masked[50];
    int wrong;
    int lives;
    int len;
    int gameOver;
};

/* ================= GLOBALS ================= */

GtkWidget *window, *stack;
GtkWidget *entry_user, *entry_pass, *entry_guess;
GtkWidget *label_word, *label_hint, *label_lives;
GtkWidget *label_hangman, *label_info;
GtkWidget *label_score, *label_leader;

struct User currentUser;
struct GameState game;

/* ================= DATA ================= */

struct WordData words[] = {
    {"PROGRAMMING","Creating software"},
    {"ELEPHANT","Largest land mammal"},
    {"CRYPTOCURRENCY","Digital money"},
    {"ALGORITHM","Step-by-step solution"},
    {"PYRAMID","Ancient structure"}
};

const char *hangmanStages[] = {
"  ___\n  |   |\n      |\n      |\n      |\n______|",
"  ___\n  |   |\n  O   |\n      |\n      |\n______|",
"  ___\n  |   |\n  O   |\n  |   |\n      |\n______|",
"  ___\n  |   |\n  O   |\n /|   |\n      |\n______|",
"  ___\n  |   |\n  O   |\n /|\\  |\n      |\n______|",
"  ___\n  |   |\n  O   |\n /|\\  |\n /    |\n______|",
"  ___\n  |   |\n  O   |\n /|\\  |\n / \\  |\n______|"
};

/* ================= FILE HANDLING ================= */

int userExists(const char *u) {
    FILE *f = fopen("users.dat","rb");
    struct User t;
    if (!f) return 0;
    while (fread(&t,sizeof(t),1,f))
        if (!strcmp(t.username,u)) { fclose(f); return 1; }
    fclose(f);
    return 0;
}

int loginUser(const char *u,const char *p) {
    FILE *f = fopen("users.dat","rb");
    if (!f) return 0;
    while (fread(&currentUser,sizeof(currentUser),1,f)) {
        if (!strcmp(currentUser.username,u) &&
            !strcmp(currentUser.password,p)) {
            fclose(f); return 1;
        }
    }
    fclose(f);
    return 0;
}

void registerUser(const char *u,const char *p) {
    struct User n = {0};
    strcpy(n.username,u);
    strcpy(n.password,p);
    FILE *f = fopen("users.dat","ab");
    fwrite(&n,sizeof(n),1,f);
    fclose(f);
}

void updateStats(int won) {
    FILE *f = fopen("users.dat","rb");
    FILE *t = fopen("temp.dat","wb");
    struct User u;

    currentUser.gamesPlayed++;
    if (won) {
        currentUser.gamesWon++;
        currentUser.totalScore += 100;
    } else currentUser.gamesLost++;

    while (fread(&u,sizeof(u),1,f)) {
        if (!strcmp(u.username,currentUser.username))
            fwrite(&currentUser,sizeof(currentUser),1,t);
        else fwrite(&u,sizeof(u),1,t);
    }
    fclose(f); fclose(t);
    remove("users.dat");
    rename("temp.dat","users.dat");
}

/* ================= GAME ================= */

void initGame() {
    int i = rand() % 5;
    strcpy(game.word, words[i].word);
    strcpy(game.masked, words[i].word);
    game.len = strlen(game.word);
    for (int j=0;j<game.len;j++) game.masked[j]='_';
    game.masked[game.len]='\0';
    game.wrong = 0;
    game.lives = 6;
    game.gameOver = 0;

    gtk_label_set_text(GTK_LABEL(label_word), game.masked);
    gtk_label_set_text(GTK_LABEL(label_hint), words[i].hint);
    gtk_label_set_text(GTK_LABEL(label_lives), "Lives: 6");
    gtk_label_set_text(GTK_LABEL(label_hangman), hangmanStages[0]);
    gtk_label_set_text(GTK_LABEL(label_info), "");
    gtk_widget_set_sensitive(entry_guess, TRUE);
}

void on_guess(GtkButton *b,gpointer d) {
    if (game.gameOver) return;

    const char *t = gtk_entry_get_text(GTK_ENTRY(entry_guess));
    if (strlen(t)!=1 || !isalpha(t[0])) return;

    char g = toupper(t[0]);
    int found = 0;

    for (int i=0;i<game.len;i++) {
        if (game.word[i]==g && game.masked[i]=='_') {
            game.masked[i]=g;
            found=1;
        }
    }
    if (!found) game.wrong++;

    gtk_label_set_text(GTK_LABEL(label_word), game.masked);
    gtk_label_set_text(GTK_LABEL(label_hangman),
        hangmanStages[game.wrong > 6 ? 6 : game.wrong]);

    char buf[20];
    sprintf(buf,"Lives: %d", game.lives - game.wrong);
    gtk_label_set_text(GTK_LABEL(label_lives), buf);

    if (!strcmp(game.word,game.masked)) {
        gtk_label_set_text(GTK_LABEL(label_info),"🎉 YOU WON!");
        updateStats(1);
        game.gameOver=1;
    }
    else if (game.wrong>=game.lives) {
        gtk_label_set_text(GTK_LABEL(label_info),"❌ YOU LOST!");
        updateStats(0);
        game.gameOver=1;
    }
    gtk_entry_set_text(GTK_ENTRY(entry_guess),"");
}

/* ================= LEADERBOARD ================= */

void loadLeaderboard() {
    FILE *f = fopen("users.dat","rb");
    struct User u[50];
    int n=0;
    if (!f) return;
    while (fread(&u[n],sizeof(u[n]),1,f)) n++;
    fclose(f);

    for (int i=0;i<n;i++)
        for (int j=i+1;j<n;j++)
            if (u[j].totalScore > u[i].totalScore) {
                struct User t=u[i]; u[i]=u[j]; u[j]=t;
            }

    char text[1500]="🏆 LEADERBOARD 🏆\n\n";
    for (int i=0;i<n;i++) {
        char line[200];
        sprintf(line,"%d. %s | Score: %d\n",
                i+1,u[i].username,u[i].totalScore);
        strcat(text,line);
    }
    gtk_label_set_text(GTK_LABEL(label_leader), text);
    gtk_stack_set_visible_child_name(GTK_STACK(stack),"leader");
}

/* ================= UI ================= */

void on_login(GtkButton *b,gpointer d) {
    if (loginUser(
        gtk_entry_get_text(GTK_ENTRY(entry_user)),
        gtk_entry_get_text(GTK_ENTRY(entry_pass))
    )) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack),"game");
        initGame();
    } else gtk_label_set_text(GTK_LABEL(label_info),"Invalid login");
}

void on_register(GtkButton *b,gpointer d) {
    const char *u=gtk_entry_get_text(GTK_ENTRY(entry_user));
    const char *p=gtk_entry_get_text(GTK_ENTRY(entry_pass));
    if (userExists(u)) {
        gtk_label_set_text(GTK_LABEL(label_info),"User exists");
        return;
    }
    registerUser(u,p);
    gtk_label_set_text(GTK_LABEL(label_info),"Registered! Login now");
}

void on_logout(GtkButton *b,gpointer d) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack),"login");
}

void back(GtkButton *b,gpointer d) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack),"game");
}

/* ================= MAIN ================= */

int main(int argc,char *argv[]) {
    gtk_init(&argc,&argv);
    srand(time(NULL));

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"Hangman Game");
    gtk_window_set_default_size(GTK_WINDOW(window),600,420);
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

    stack=gtk_stack_new();
    gtk_container_add(GTK_CONTAINER(window),stack);

    /* LOGIN */
    GtkWidget *login=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    entry_user=gtk_entry_new();
    entry_pass=gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_pass),FALSE);
    GtkWidget *btnL=gtk_button_new_with_label("Login");
    GtkWidget *btnR=gtk_button_new_with_label("Register");
    label_info=gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(login),entry_user,0,0,5);
    gtk_box_pack_start(GTK_BOX(login),entry_pass,0,0,5);
    gtk_box_pack_start(GTK_BOX(login),btnL,0,0,5);
    gtk_box_pack_start(GTK_BOX(login),btnR,0,0,5);
    gtk_box_pack_start(GTK_BOX(login),label_info,0,0,5);
    gtk_stack_add_named(GTK_STACK(stack),login,"login");

    /* GAME */
    GtkWidget *gameBox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    GtkWidget *left=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    GtkWidget *right=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);

    label_hangman=gtk_label_new(hangmanStages[0]);
    gtk_widget_override_font(label_hangman,
        pango_font_description_from_string("Monospace 12"));

    label_word=gtk_label_new("");
    label_hint=gtk_label_new("");
    label_lives=gtk_label_new("");
    entry_guess=gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_guess),1);

    GtkWidget *btnG=gtk_button_new_with_label("Guess");
    GtkWidget *btnN=gtk_button_new_with_label("New Game");
    GtkWidget *btnO=gtk_button_new_with_label("Logout");
    GtkWidget *btnLB=gtk_button_new_with_label("Leaderboard");

    gtk_box_pack_start(GTK_BOX(left),label_hangman,0,0,5);
    gtk_box_pack_start(GTK_BOX(right),label_word,0,0,5);
    gtk_box_pack_start(GTK_BOX(right),label_hint,0,0,5);
    gtk_box_pack_start(GTK_BOX(right),label_lives,0,0,5);
    gtk_box_pack_start(GTK_BOX(right),entry_guess,0,0,5);
    gtk_box_pack_start(GTK_BOX(right),btnG,0,0,5);
    gtk_box_pack_start(GTK_BOX(right),btnN,0,0,5);
    gtk_box_pack_start(GTK_BOX(right),btnLB,0,0,5);
    gtk_box_pack_start(GTK_BOX(right),btnO,0,0,5);

    gtk_box_pack_start(GTK_BOX(gameBox),left,0,0,5);
    gtk_box_pack_start(GTK_BOX(gameBox),right,0,0,5);
    gtk_stack_add_named(GTK_STACK(stack),gameBox,"game");

    /* LEADERBOARD */
    GtkWidget *leader=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    label_leader=gtk_label_new("");
    GtkWidget *btnBack=gtk_button_new_with_label("Back");
    gtk_box_pack_start(GTK_BOX(leader),label_leader,0,0,5);
    gtk_box_pack_start(GTK_BOX(leader),btnBack,0,0,5);
    gtk_stack_add_named(GTK_STACK(stack),leader,"leader");

    g_signal_connect(btnL,"clicked",G_CALLBACK(on_login),NULL);
    g_signal_connect(btnR,"clicked",G_CALLBACK(on_register),NULL);
    g_signal_connect(btnG,"clicked",G_CALLBACK(on_guess),NULL);
    g_signal_connect(btnN,"clicked",G_CALLBACK(initGame),NULL);
    g_signal_connect(btnO,"clicked",G_CALLBACK(on_logout),NULL);
    g_signal_connect(btnLB,"clicked",G_CALLBACK(loadLeaderboard),NULL);
    g_signal_connect(btnBack,"clicked",G_CALLBACK(back),NULL);

    gtk_stack_set_visible_child_name(GTK_STACK(stack),"login");
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}





