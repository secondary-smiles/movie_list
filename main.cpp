#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <pwd.h>
#include <fstream>

namespace fs = std::filesystem;

std::ostream& bold_on(std::ostream& os)
{
    return os << "\e[1m";
}

std::ostream& bold_off(std::ostream& os)
{
    return os << "\e[0m";
}

void programUsage(const char *programName) {
    std::cout << "Usage: " << programName << " <command> <movie name>" << std::endl;
}

// A function to return the current user's home directory
const char * getHomeDir() {
    const char *homedir;

    if ((homedir = getenv("HOME")) == nullptr) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    return homedir;
}

void checkProgram() {
    const char *homedir = getHomeDir();
    const fs::path dir = fs::path(homedir) / ".custom_lists";
    if (!fs::exists(dir)) {
        std::cout << "Directory " << dir << " does not exist\n"
                  << "Please run the init command to create the directory" << std::endl;
        exit(0);
    }
    const fs::path movies_w = dir / "movies_watched.txt";
    if (!fs::exists(movies_w)) {
        std::cout << "File " << movies_w << " does not exist\n"
                  << "Please run the init command to create the file" << std::endl;
        exit(0);
    }
    const fs::path movies_uw = dir / "movies_unwatched.txt";
    if (!fs::exists(movies_uw)) {
        std::cout << "File " << movies_uw << " does not exist\n"
                  << "Please run the init command to create the file" << std::endl;
        exit(0);
    }
}

int checkMovie(const char *movie) {
    const fs::path dir = fs::path(getHomeDir()) / ".custom_lists";
    const fs::path movies_w = dir / "movies_watched.txt";
    const fs::path movies_uw = dir / "movies_unwatched.txt";
    std::ifstream file_w(movies_w);
    std::ifstream file_uw(movies_uw);
    std::string line;
    while (std::getline(file_w, line)) {
        if (line == movie) {
            return 1;
        }
    }
    while (std::getline(file_uw, line)) {
        if (line == movie) {
            return 2;
        }
    }
    return 0;
}

void removeMovie(const char *movie) {
    const fs::path filePath_uw = fs::path(getHomeDir()) / ".custom_lists" / "movies_unwatched.txt";
    const fs::path tempPath_uw = fs::path(getHomeDir()) / ".custom_lists" / "temp_unwatched.txt";
    std::ifstream file_uw(filePath_uw);
    std::string line_uw;
    std::ofstream temp_uw(tempPath_uw);
    while (std::getline(file_uw, line_uw)) {
        bool found = false;
        if (line_uw == movie) {
            found = true;
        }
        if (!found) {
            temp_uw << line_uw << std::endl;
        }
    }
    temp_uw.close();
    file_uw.close();
    fs::remove(filePath_uw);
    fs::rename(tempPath_uw, filePath_uw);

    const fs::path filePath_w = fs::path(getHomeDir()) / ".custom_lists" / "movies_watched.txt";
    const fs::path tempPath_w = fs::path(getHomeDir()) / ".custom_lists" / "temp_watched.txt";
    std::ifstream file_w(filePath_w);
    std::string line_w;
    std::ofstream temp_w(tempPath_w);
    while (std::getline(file_w, line_w)) {
        bool found = false;
        if (line_w == movie) {
            found = true;
        }
        if (!found) {
            temp_w << line_w << std::endl;
        }
    }
    temp_w.close();
    file_w.close();
    fs::remove(filePath_w);
    fs::rename(tempPath_w, filePath_w);
}

void list() {
    checkProgram();

    std::cout << bold_on << "Unwatched:" << bold_off << std::endl;
    const fs::path filePath_uw = fs::path(getHomeDir()) / ".custom_lists" / "movies_unwatched.txt";
    std::ifstream file_uw(filePath_uw);
    std::string line_uw;
    while (std::getline(file_uw, line_uw)) {
        std::cout << line_uw << std::endl;
    }
    std::cout << std::endl;

    std::cout << bold_on << "Watched:" << bold_off << std::endl;
    const fs::path filePath_w = fs::path(getHomeDir()) / ".custom_lists" / "movies_watched.txt";
    std::ifstream file_w(filePath_w);
    std::string line_w;
    while (std::getline(file_w, line_w)) {
        std::cout << line_w << std::endl;
    }

    std::cout << std::endl;
}

void add(int argc, char *argv[]) {
    checkProgram();

    const fs::path filePath_uw = fs::path(getHomeDir()) / ".custom_lists" / "movies_unwatched.txt";
    std::ofstream file_uw(filePath_uw, std::ios_base::app);
    for (int i = 2; i < argc; i++) {
        if (checkMovie(argv[i]) == 0) {
            file_uw << argv[i] << std::endl;
        } else {
            std::cout << bold_on << "Movie " << bold_on << argv[i] << bold_off <<  " already in a list" << bold_off << std::endl;
        }
    }
    file_uw.close();
}

void remove(int argc, char *argv[]) {
    checkProgram();

    for (int i = 2; i < argc; i++) {
        if (checkMovie(argv[i]) != 0) {
            removeMovie(argv[i]);
        } else {
            std::cout << "Movie " << bold_on << argv[i] << bold_off <<  " not in a list" << std::endl;
        }
    }
}

void move(int argc, char *argv[]) {
    checkProgram();

    for (int i = 2; i < argc; i++) {
        if (checkMovie(argv[i]) == 1) {
            removeMovie(argv[i]);
            add(argc, argv);

        } else if (checkMovie(argv[i]) == 2) {
            removeMovie(argv[i]);

            const fs::path filePath_w = fs::path(getHomeDir()) / ".custom_lists" / "movies_watched.txt";
            std::ofstream file_w(filePath_w, std::ios_base::app);
            file_w << argv[i] << std::endl;
            file_w.close();
        } else {
            std::cout << "Movie " << bold_on << argv[i] << bold_off <<  " not in a list" << std::endl;
        }
    }
}

void init() {
    const char *homedir = getHomeDir();

    const fs::path dir = fs::path(homedir) / ".custom_lists";
    if (!fs::exists(dir)) {
        fs::create_directory(dir);
    } else {
        std::cout << "Directory" << dir << " already exists" << std::endl;
    }

    const fs::path file_uw = dir / "movies_unwatched.txt";
    if (!fs::exists(file_uw)) {
        std::ofstream ofstream(file_uw);
        ofstream.close();
    } else {
        std::cout << "File " << file_uw << " already exists" << std::endl;
    }

    const fs::path file_w = dir / "movies_watched.txt";
    if (!fs::exists(file_w)) {
        std::ofstream ofstream(file_w);
        ofstream.close();
    } else {
        std::cout << "File " << file_w << " already exists" << std::endl;
    }

    std::cout << "Movie lists setup finished." << std::endl;

}

void parse(int argc, char *argv[]) {
    // list of possible commands
    const char *commands[5] = { "list", "add", "remove", "move", "init" };
    const char *alternatives[5] = { "l", "a", "r", "m", "i" };

    // number of args

    // We need at argc to be at least 2 long
    if (argc < 2) {
        programUsage(argv[0]);
        return;
    }

    // Different help dialogues
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
        programUsage(argv[0]);
        return;
    }

    for (int i = 0; i < 5; i++) {
        if (strcmp(argv[1], commands[i]) == 0 || strcmp(argv[1], alternatives[i]) == 0) {
            switch (i) {
                case 0:
                    list();
                    break;
                case 1:
                    add(argc, argv);
                    break;
                case 2:
                    remove(argc, argv);
                    break;
                case 3:
                    move(argc, argv);
                    break;
                case 4:
                    init();
                    break;
                default:
                    break;
            }
        }
    }
}

int main(int argc, char *argv[]){

    // Parse the arguments
    parse(argc, argv);

    return 0;
}