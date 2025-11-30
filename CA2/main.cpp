#include "Shell/Shell.hpp"
#include <iostream>
#include <sys/stat.h> // For mkfifo
#include <sys/types.h>
#include <unistd.h>   // For access, F_OK
#include <cstdlib>

// Function to ensure the named pipe exists before the shell starts
void setupNamedPipe(const std::string& pipeName) {
    // Check if the file exists using access()
    if (access(pipeName.c_str(), F_OK) == -1) {
        // File doesn't exist, create it with permissions 0666 (read/write for all)
        if (mkfifo(pipeName.c_str(), 0666) == -1) {
            perror("Error creating named pipe");
            exit(EXIT_FAILURE);
        }
        std::cout << "[Info] Named pipe '" << pipeName << "' created successfully." << std::endl;
    } else {
        // Optional: Let the user know it already exists
        // std::cout << "[Info] Named pipe '" << pipeName << "' found." << std::endl;
    }
}

int main() {
    // The name must match the one used inside Shell class
    const std::string FIFO_NAME = "np_fifo";

    // 1. Prepare the environment
    setupNamedPipe(FIFO_NAME);

    // 2. Instantiate the Shell
    Shell myShell;

    // 3. Start the main loop
    myShell.run();

    // unlink(FIFO_NAME.c_str());

    return 0;
}