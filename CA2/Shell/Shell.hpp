// Shell.hpp

#ifndef SHELL_HPP
#define SHELL_HPP

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>

// Enum to define where input/output comes from/goes to
enum class IOType {
    STD,        // Standard Input/Output (Console)
    PIPE,       // Unnamed Pipe ( | )
    NAMED_PIPE  // Named Pipe ( np )
};

// Helper struct to hold parsed command information
struct Command {
    std::string name;
    std::vector<std::string> args;
    IOType inputType = IOType::STD;
    IOType outputType = IOType::STD;
};

class Shell {
public:
    Shell();
    void run();

private:
    using CommandHandler = std::function<void(const std::vector<std::string>&)>;
    std::map<std::string, CommandHandler> commandMap;
    bool running;
    
    const std::string namedPipePath = "np_fifo"; 

    void initCommands();
    
    // Step 1: Split raw line by '|' into command strings
    std::vector<std::string> splitByPipe(const std::string& line);
    
    // Step 2: Parse a single command string (handle < np, > np)
    Command parseCommand(std::string commandStr);

    // Step 3: The main logic that orchestrates everything
    void processLine(const std::string& line);

    void configureInput(const Command& cmd, int prev_pipe_read_fd);
    void configureOutput(const Command& cmd, int current_pipe_write_fd);
    void runChildProcess(const Command& cmd, int prev_pipe_read_fd, int current_pipe_write_fd, int current_pipe_read_fd);
    void executePipeline(std::vector<Command>& pipeline);

    // Handlers
    void handleEcho(const std::vector<std::string>& args);
    void handlePrintN(const std::vector<std::string>& args);
    void handleLen(const std::vector<std::string>& args);
    void handleExit(const std::vector<std::string>& args);
};

#endif // SHELL_HPP