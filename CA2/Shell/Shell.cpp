#include "Shell.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

// Constructor
Shell::Shell() : running(true) {
    // Initialize available commands
    initCommands();
}

// Registers command strings to member functions
void Shell::initCommands() {
    // Using lambda to bind member functions to the map
    commandMap["echo"] = [this](const std::vector<std::string>& args) { handleEcho(args); };
    commandMap["print_n"] = [this](const std::vector<std::string>& args) { handlePrintN(args); };
    commandMap["len"] = [this](const std::vector<std::string>& args) { handleLen(args); };
    commandMap["exit"] = [this](const std::vector<std::string>& args) { handleExit(args); };
}

std::vector<std::string> Shell::splitByPipe(const std::string& line) {
    std::vector<std::string> segments;
    std::stringstream ss(line);
    std::string segment;

    while (std::getline(ss, segment, '|')) {
        
        if (!segment.empty()) {
            segments.push_back(segment);
        }
    }

    return segments;
}

// Main execution loop
void Shell::run() {
    std::string line;
    
    // Keep reading lines until exit is called or input stream ends
    while (running && std::getline(std::cin, line)) {
        if (line.empty()) continue;
        processLine(line);
    }
}

// Shell.cpp (partial implementation of logic)

void Shell::processLine(const std::string& line) {
    // 1. Split the line by '|'
    // Example: "echo hello > np | len" -> ["echo hello > np ", " len"]
    std::vector<std::string> rawCommands = splitByPipe(line);
    
    std::vector<Command> pipeline;

    // 2. Parse each segment to handle redirections (<, >)
    for (size_t i = 0; i < rawCommands.size(); ++i) {
        Command cmd = parseCommand(rawCommands[i]);
        
        // If it's NOT the first command, its input usually comes from the previous PIPE
        // UNLESS it explicitly requested input from NAMED_PIPE
        if (i > 0 && cmd.inputType != IOType::NAMED_PIPE) {
            cmd.inputType = IOType::PIPE;
        }

        // If it's NOT the last command, its output usually goes to the next PIPE
        // UNLESS it explicitly requested output to NAMED_PIPE
        if (i < rawCommands.size() - 1 && cmd.outputType != IOType::NAMED_PIPE) {
            cmd.outputType = IOType::PIPE;
        }
        
        pipeline.push_back(cmd);
    }

    if (pipeline.empty()) return;

    if (pipeline[0].name == "exit") {
        running = false;
        return; 
    }

    // 3. Execute the pipeline
    // Here we will loop through 'pipeline', use fork(), pipe(), and dup2()
    // based on inputType and outputType of each command.
    executePipeline(pipeline); // We need to implement this
}

Command Shell::parseCommand(std::string commandStr) {
    Command cmd;
    std::stringstream ss(commandStr);
    std::string token;
    
    while (ss >> token) {
        if (token == ">") {
            std::string nextToken;
            if (ss >> nextToken && nextToken == "np") {
                cmd.outputType = IOType::NAMED_PIPE;
            }
            // Note: Project says we don't need to support file writing, only np 
        } 
        else if (token == "<") {
            std::string nextToken;
            if (ss >> nextToken && nextToken == "np") {
                cmd.inputType = IOType::NAMED_PIPE;
            }
        } 
        else {
            if (cmd.name.empty()) cmd.name = token;
            else cmd.args.push_back(token);
        }
    }
    // Add command name to args list too (convention for execvp)
    if (!cmd.name.empty()) {
        cmd.args.insert(cmd.args.begin(), cmd.name);
    }
    
    return cmd;
}

// --- Command Implementations ---

void Shell::handleEcho(const std::vector<std::string>& args) {
    // args[0] is "echo", so we print from args[1] onwards
    for (size_t i = 1; i < args.size(); ++i) {
        std::cout << args[i] << (i == args.size() - 1 ? "" : " ");
    }
    std::cout << std::endl;
}

void Shell::handlePrintN(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Error: print_n requires an argument." << std::endl;
        return;
    }

    try {
        int n = std::stoi(args[1]);
        for (int i = 0; i < n; ++i) {
            std::cout << "hello" << (i == n - 1 ? "" : ", ");
        }
        std::cout << std::endl;
    } catch (...) {
        std::cerr << "Error: Invalid number argument." << std::endl;
    }
}

void Shell::handleLen(const std::vector<std::string>& args) {
    // Case 1: Argument provided (e.g., "len hello")
    if (args.size() > 1) {
        std::cout << args[1].length() << std::endl;
        return;
    }

    // Case 2: No argument provided, read from STDIN (Pipe mode)
    // Example: "print_n 2 | len"
    // We read the entire input from cin until EOF
    std::string line;
    std::string content;
    
    // Read all lines from input (pipeline output)
    while (std::getline(std::cin, line)) {
        content += line;
    }

    // Simple implementation ignoring newline complications for now (counting content text):
    std::cout << content.length() << std::endl;
}

void Shell::handleExit(const std::vector<std::string>& args) {
    // Stop the main loop
    this->running = false;
}

// Helper: Sets up input redirection (STDIN)
void Shell::configureInput(const Command& cmd, int prev_pipe_read_fd) {
    if (cmd.inputType == IOType::PIPE) {
        if (prev_pipe_read_fd != -1) {
            // Replace STDIN with the read end of the previous pipe
            dup2(prev_pipe_read_fd, STDIN_FILENO);
            close(prev_pipe_read_fd);
        }
    } else if (cmd.inputType == IOType::NAMED_PIPE) {
        int fd = open(namedPipePath.c_str(), O_RDONLY);
        if (fd == -1) {
            perror("Error opening named pipe (read)");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
}

// Helper: Sets up output redirection (STDOUT)
void Shell::configureOutput(const Command& cmd, int current_pipe_write_fd) {
    if (cmd.outputType == IOType::PIPE) {
        if (current_pipe_write_fd != -1) {
            // Replace STDOUT with the write end of the current pipe
            dup2(current_pipe_write_fd, STDOUT_FILENO);
            close(current_pipe_write_fd);
        }
    } else if (cmd.outputType == IOType::NAMED_PIPE) {
        int fd = open(namedPipePath.c_str(), O_WRONLY);
        if (fd == -1) {
            perror("Error opening named pipe (write)");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

// Helper: Execution logic inside the child process
void Shell::runChildProcess(const Command& cmd, int prev_pipe_read_fd, int current_pipe_write_fd, int current_pipe_read_fd) {
    // 1. Setup Input
    configureInput(cmd, prev_pipe_read_fd);

    // 2. Setup Output
    configureOutput(cmd, current_pipe_write_fd);

    // 3. Close unused file descriptors (Critical to prevent hangs)
    if (current_pipe_read_fd != -1) close(current_pipe_read_fd);

    // 4. Execute Command
    if (commandMap.find(cmd.name) != commandMap.end()) {
        commandMap[cmd.name](cmd.args);
    } else {
        std::cerr << "Command not found: " << cmd.name << std::endl;
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS); // Ensure child terminates here
}

// Main execution loop
void Shell::executePipeline(std::vector<Command>& pipeline) {
    int numCmds = pipeline.size();
    int prev_pipe_read_fd = -1; 
    int pipefd[2];

    for (int i = 0; i < numCmds; ++i) {
        Command& cmd = pipeline[i];
        bool needPipe = (cmd.outputType == IOType::PIPE);

        // Create pipe if needed
        if (needPipe) {
            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                return;
            }
        } else {
            pipefd[0] = -1;
            pipefd[1] = -1;
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork failed");
            return;
        }

        if (pid == 0) {
            // CHILD PROCESS
            runChildProcess(cmd, prev_pipe_read_fd, pipefd[1], pipefd[0]);
        } else {
            // PARENT PROCESS
            
            // Close the read end of the PREVIOUS pipe (child has it now)
            if (prev_pipe_read_fd != -1) {
                close(prev_pipe_read_fd);
            }

            // Prepare for the next iteration
            if (needPipe) {
                // Close write end (parent doesn't write)
                close(pipefd[1]);
                // Save read end for the next child
                prev_pipe_read_fd = pipefd[0]; 
            } else {
                prev_pipe_read_fd = -1;
            }
        }
    }

    // Wait for all children to finish
    for (int i = 0; i < numCmds; ++i) {
        wait(NULL);
    }
}