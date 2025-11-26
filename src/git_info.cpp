#include "git_info.h"

#include <cstdio>
#include <string>

// Runs a shell command and captures its stdout.
// Returns empty string on error or if nothing was printed.
static std::string run_command_capture(const char* cmd)
{
#ifdef _WIN32
    FILE* pipe = _popen(cmd, "r");
#else
    FILE* pipe = popen(cmd, "r");
#endif
    if (!pipe)
        return {};

    std::string result;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    // strip trailing newlines
    while (!result.empty() &&
           (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }

    return result;
}

GitInfo detect_git_info()
{
    GitInfo info;

#ifdef _WIN32
    std::string commit = run_command_capture("git rev-parse HEAD 2>nul");
    std::string remote = run_command_capture("git config --get remote.origin.url 2>nul");
#else
    std::string commit = run_command_capture("git rev-parse HEAD 2>/dev/null");
    std::string remote = run_command_capture("git config --get remote.origin.url 2>/dev/null");
#endif

    if (!commit.empty()) {
        info.commit = commit;
        info.has_commit = true;
    }

    if (!remote.empty()) {
        info.remote = remote;
        info.has_remote = true;
    }

    return info;
}
