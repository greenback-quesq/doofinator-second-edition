// AI GENERATED CODE AS BASIS
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>

// Placeholder for the music player functions
class MusicPlayer {
public:
    void begin() {
        printw("Music player initialized.\n");
    }

    void startPlayingFile(const std::string& file) {
        printw("Playing file: %s\n", file.c_str());
    }

    void pausePlaying(bool pause) {
        if (pause) {
            printw("Music paused.\n");
        } else {
            printw("Music resumed.\n");
        }
    }

    void stopPlaying() {
        printw("Music stopped.\n");
    }

    void setVolume(int left, int right) {
        printw("Volume set to: %d, %d\n", left, right);
    }
};

MusicPlayer musicPlayer;

// Variables definition
int count = 0;  // track listing selection
bool play = true;  // start or stop the music
int vol_setting = 0;  // tracking current volume, initialized to three
int randomNumber; 
bool flag = true;  // necessary for skipping or returning to tracks

void displayMenu() {
    printw("==================================\n");
    printw("       DOOFINATOR JUKEBOX         \n");
    printw("==================================\n");
    printw("1. Play Next Track\n");
    printw("2. Play Previous Track\n");
    printw("3. Pause/Resume Track\n");
    printw("4. Increase Volume\n");
    printw("5. Decrease Volume\n");
    printw("6. Show Current Time\n");
    printw("Enter a number to select an option: ");
    refresh();
}

void handleUserInput() {
    int userInput = getch() - '0';
    switch (userInput) {
        case 1:
            count++;
            musicPlayer.stopPlaying();
            flag = false;
            break;
        case 2:
            count--;
            musicPlayer.stopPlaying();
            flag = false;
            break;
        case 3:
            musicPlayer.pausePlaying(!play);
            play = !play;
            break;
        case 4:
            vol_setting++;
            musicPlayer.setVolume(vol_setting, vol_setting);
            break;
        case 5:
            vol_setting--;
            musicPlayer.setVolume(vol_setting, vol_setting);
            break;
        case 6:
            std::time_t now = std::time(0);
            printw("Current time: %s", std::ctime(&now));
            break;
        default:
            printw("Invalid option, please try again.\n");
            break;
    }
    displayMenu(); // Display the menu again after handling input
}

void setup() {
    std::srand(static_cast<unsigned>(std::time(0)));
    initscr();
    cbreak();
    noecho();
    musicPlayer.begin();
    musicPlayer.setVolume(vol_setting, vol_setting);
    displayMenu(); // Display the menu at startup
}

void loop() {
    while (flag) {
        handleUserInput(); // Handle user input for the interactive menu
    }
}

int main() {
    setup();
    loop();
    endwin();
    return 0;
}
