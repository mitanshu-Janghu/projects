#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
struct WordData {
    char word[30];
    char category[30];
    char difficulty[10];
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

// Function prototypes
void showHangman(int attempts);
int registerUser();
int loginUser(struct User *currentUser);
void saveUserData(struct User user);
void updateUserStats(struct User *user, int won, int score);
void displayStats(struct User user);
void clearInputBuffer();
int calculateScore(int remainingLives, int wordLength, char difficulty[]);

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void showHangman(int attempts) {
    switch (attempts) {
        case 0:
            printf("\n  ___\n  |   |\n      |\n      |\n      |\n______|\n");
            break;
        case 1:
            printf("\n  ___\n  |   |\n  O   |\n      |\n      |\n______|\n");
            break;
        case 2:
            printf("\n  ___\n  |   |\n  O   |\n  |   |\n      |\n______|\n");
            break;
        case 3:
            printf("\n  ___\n  |   |\n  O   |\n /|   |\n      |\n______|\n");
            break;
        case 4:
            printf("\n  ___\n  |   |\n  O   |\n /|\\  |\n      |\n______|\n");
            break;
        case 5:
            printf("\n  ___\n  |   |\n  O   |\n /|\\  |\n /    |\n______|\n");
            break;
        case 6:
            printf("\n  ___\n  |   |\n  O   |\n /|\\  |\n / \\  |\n______|\n");
            break;
    }
}

int registerUser() {
    struct User newUser;
    FILE *file;
    char confirmPass[30];
    
    printf("\n===== USER REGISTRATION =====\n");
    printf("Enter username: ");
    scanf("%s", newUser.username);
    clearInputBuffer();
    
    file = fopen("users.dat", "rb");
    if (file != NULL) {
        struct User temp;
        while (fread(&temp, sizeof(struct User), 1, file)) {
            if (strcmp(temp.username, newUser.username) == 0) {
                printf("Username already exists! Please choose a different username.\n");
                fclose(file);
                return 0;
            }
        }
        fclose(file);
    }
    
    printf("Enter password: ");
    scanf("%s", newUser.password);
    clearInputBuffer();
    
    printf("Confirm password: ");
    scanf("%s", confirmPass);
    clearInputBuffer();
    
    if (strcmp(newUser.password, confirmPass) != 0) {
        printf("Passwords do not match! Registration failed.\n");
        return 0;
    }
    
    newUser.gamesPlayed = 0;
    newUser.gamesWon = 0;
    newUser.gamesLost = 0;
    newUser.totalScore = 0;
    
    file = fopen("users.dat", "ab");
    if (file == NULL) {
        printf("Error creating user file!\n");
        return 0;
    }
    
    fwrite(&newUser, sizeof(struct User), 1, file);
    fclose(file);
    
    printf("\nRegistration successful! You can now login.\n");
    return 1;
}

int loginUser(struct User *currentUser) {
    char username[30], password[30];
    FILE *file;
    int found = 0;
    
    printf("\n===== USER LOGIN =====\n");
    printf("Enter username: ");
    scanf("%s", username);
    clearInputBuffer();
    
    printf("Enter password: ");
    scanf("%s", password);
    clearInputBuffer();
    
    file = fopen("users.dat", "rb");
    if (file == NULL) {
        printf("No registered users found!\n");
        return 0;
    }
    
    struct User temp;
    while (fread(&temp, sizeof(struct User), 1, file)) {
        if (strcmp(temp.username, username) == 0 && strcmp(temp.password, password) == 0) {
            *currentUser = temp;
            found = 1;
            break;
        }
    }
    fclose(file);
    
    if (found) {
        printf("\nLogin successful! Welcome back, %s!\n", currentUser->username);
        return 1;
    } else {
        printf("\nInvalid username or password!\n");
        return 0;
    }
}

int calculateScore(int remainingLives, int wordLength, char difficulty[]) {
    int baseScore = wordLength * 10;
    int lifeBonus = remainingLives * 15;
    int difficultyMultiplier = 1;
    
    if (strcmp(difficulty, "Easy") == 0)
        difficultyMultiplier = 1;
    else if (strcmp(difficulty, "Medium") == 0)
        difficultyMultiplier = 2;
    else if (strcmp(difficulty, "Hard") == 0)
        difficultyMultiplier = 3;
    
    return (baseScore + lifeBonus) * difficultyMultiplier;
}

void updateUserStats(struct User *user, int won, int score) {
    FILE *file, *tempFile;
    struct User temp;
    
    user->gamesPlayed++;
    if (won) {
        user->gamesWon++;
        user->totalScore += score;
    } else {
        user->gamesLost++;
    }
    
    file = fopen("users.dat", "rb");
    tempFile = fopen("temp.dat", "wb");
    
    if (file == NULL || tempFile == NULL) {
        printf("Error updating user statistics!\n");
        return;
    }
    
    while (fread(&temp, sizeof(struct User), 1, file)) {
        if (strcmp(temp.username, user->username) == 0) {
            fwrite(user, sizeof(struct User), 1, tempFile);
        } else {
            fwrite(&temp, sizeof(struct User), 1, tempFile);
        }
    }
    
    fclose(file);
    fclose(tempFile);
    
    remove("users.dat");
    rename("temp.dat", "users.dat");
}

void displayStats(struct User user) {
    float winPercentage = 0;
    if (user.gamesPlayed > 0) {
        winPercentage = ((float)user.gamesWon / user.gamesPlayed) * 100;
    }
    
    printf("\n===== YOUR STATISTICS =====\n");
    printf("Username: %s\n", user.username);
    printf("Games Played: %d\n", user.gamesPlayed);
    printf("Games Won: %d\n", user.gamesWon);
    printf("Games Lost: %d\n", user.gamesLost);
    printf("Win Percentage: %.2f%%\n", winPercentage);
    printf("Total Score: %d\n", user.totalScore);
    printf("============================\n");
}

void playGame(struct User *currentUser) {
 
    struct WordData words[] = {
        {"PROGRAMMING", "Technology", "Medium", "Creating software"},
        {"ELEPHANT", "Animals", "Easy", "Largest land mammal"},
        {"CRYPTOCURRENCY", "Technology", "Hard", "Digital currency"},
        {"COMPUTER", "Technology", "Easy", "Used for processing data"},
        {"COUNTRY", "General", "Easy", "A nation with its own government"},
        {"ALGORITHM", "Technology", "Medium", "Step-by-step procedure"},
        {"PYRAMID", "Architecture", "Medium", "Ancient Egyptian structure"},
        {"JAVASCRIPT", "Technology", "Medium", "Web programming language"},
        {"TELESCOPE", "Science", "Medium", "Used to observe distant objects"},
        {"DEMOCRACY", "Politics", "Hard", "Government by the people"}
    };

    int totalWords = 10;
    char word[50], masked[50], guess;
    char guessedLetters[26] = {0};
    int guessedCount = 0;
    int lives = 7, wrong = 0, i, found, len, win = 0;
    int index = rand() % totalWords;

    strcpy(word, words[index].word);
    len = strlen(word);

    for (i = 0; i < len; i++)
        masked[i] = '_';
    masked[len] = '\0';

    printf("\n===== HANGMAN WORD GUESSING GAME =====\n");
    printf("Category: %s\n", words[index].category);
    printf("Difficulty: %s\n", words[index].difficulty);
    printf("Hint: %s\n", words[index].hint);
    printf("The word has %d letters.\n", len);
    printf("You have %d lives.\n", lives);

    while (wrong < lives && !win) {
        showHangman(wrong);
        printf("\nWord: %s\n", masked);
        printf("Lives remaining: %d\n", lives - wrong);
        
        if (guessedCount > 0) {
            printf("Guessed letters: ");
            for (i = 0; i < guessedCount; i++) {
                printf("%c ", guessedLetters[i]);
            }
            printf("\n");
        }
        
        printf("Guess a letter: ");
        scanf(" %c", &guess);
        clearInputBuffer();
        guess = toupper(guess);
        
        if (!isalpha(guess)) {
            printf("Please enter a valid letter!\n");
            continue;
        }
        
        int duplicate = 0;
        for (i = 0; i < guessedCount; i++) {
            if (guessedLetters[i] == guess) {
                duplicate = 1;
                break;
            }
        }
        
        if (duplicate) {
            printf("You already guessed that letter!\n");
            continue;
        }
        
        guessedLetters[guessedCount++] = guess;

        found = 0;
        for (i = 0; i < len; i++) {
            if (word[i] == guess && masked[i] == '_') {
                masked[i] = guess;
                found = 1;
            }
        }

        if (!found) {
            printf("Wrong guess!\n");
            wrong++;
        } else {
            printf("Good guess!\n");
        }

        if (strcmp(word, masked) == 0)
            win = 1;
    }

    int score = 0;
    if (win) {
        printf("\n%s\n", masked);
        score = calculateScore(lives - wrong, len, words[index].difficulty);
        printf("Congratulations! You guessed the word correctly.\n");
        printf("Score: %d\n", score);
    } else {
        showHangman(6);
        printf("\nYou lost! The correct word was: %s\n", word);
    }
    
    updateUserStats(currentUser, win, score);
    
    printf("\nGame Over.\n");
}

int main() {
    struct User currentUser;
    int choice, loggedIn = 0;
    
    srand(time(0));
    
    printf("╔═══════════════════════════════════════╗\n");
    printf("║   HANGMAN WORD GUESSING GAME         ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    
    while (!loggedIn) {
        printf("\n1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                registerUser();
                break;
            case 2:
                if (loginUser(&currentUser)) {
                    loggedIn = 1;
                }
                break;
            case 3:
                printf("Thank you for playing!\n");
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }
    
    while (1) {
        printf("\n===== MAIN MENU =====\n");
        printf("1. Play Game\n");
        printf("2. View Statistics\n");
        printf("3. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                playGame(&currentUser);
                break;
            case 2:
                displayStats(currentUser);
                break;
            case 3:
                printf("Logging out... Goodbye, %s!\n", currentUser.username);
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }
    
    return 0;
}