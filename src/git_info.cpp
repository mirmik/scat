#include "git_info.h"

#include <cstdio>
#include <string>

// Runs a shell command and captures its stdout.
// Returns empty string on error or if nothing was printed.
static std::string run_command_capture(const char *cmd)
{
#ifdef _WIN32
    FILE *pipe = _popen(cmd, "r");
#else
    FILE *pipe = popen(cmd, "r");
#endif
    if (!pipe)
        return {};

    std::string result;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), pipe))
    {
        result += buffer;
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    // strip trailing newlines
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
    {
        result.pop_back();
    }

    return result;
}

GitInfo detect_git_info()
{
    GitInfo info;

#ifdef _WIN32
    std::string commit = run_command_capture("git rev-parse HEAD 2>nul");
    std::string remote =
        run_command_capture("git config --get remote.origin.url 2>nul");
#else
    std::string commit = run_command_capture("git rev-parse HEAD 2>/dev/null");
    std::string remote =
        run_command_capture("git config --get remote.origin.url 2>/dev/null");
#endif

    if (!commit.empty())
    {
        info.commit = commit;
        info.has_commit = true;
    }

    if (!remote.empty())
    {
        info.remote = remote;
        info.has_remote = true;
    }

    return info;
}

// Разбор remote вроде git@github.com:user/repo.git или
// https://github.com/user/repo(.git)
bool parse_github_remote(const std::string &remote,
                         std::string &user,
                         std::string &repo)
{
    const std::string host = "github.com";
    auto pos = remote.find(host);
    if (pos == std::string::npos)
        return false;

    pos += host.size();

    // пропускаем ':' или '/' после github.com
    while (pos < remote.size() && (remote[pos] == ':' || remote[pos] == '/'))
        ++pos;

    if (pos >= remote.size())
        return false;

    // user / repo[.git] / ...
    auto slash1 = remote.find('/', pos);
    if (slash1 == std::string::npos)
        return false;

    user = remote.substr(pos, slash1 - pos);

    auto start_repo = slash1 + 1;
    if (start_repo >= remote.size())
        return false;

    auto slash2 = remote.find('/', start_repo);
    std::string repo_part =
        (slash2 == std::string::npos)
            ? remote.substr(start_repo)
            : remote.substr(start_repo, slash2 - start_repo);

    // обрежем .git в конце, если есть
    const std::string dot_git = ".git";
    if (repo_part.size() > dot_git.size() &&
        repo_part.compare(
            repo_part.size() - dot_git.size(), dot_git.size(), dot_git) == 0)
    {
        repo_part.resize(repo_part.size() - dot_git.size());
    }

    if (user.empty() || repo_part.empty())
        return false;

    repo = repo_part;
    return true;
}

GitHubInfo detect_github_info()
{
    GitInfo gi = detect_git_info();
    GitHubInfo out;
    if (!gi.has_commit || !gi.has_remote)
        return out;

    if (!parse_github_remote(gi.remote, out.user, out.repo))
        return out;

    out.commit = gi.commit;
    out.ok = true;
    return out;
}

std::string detect_git_dir()
{
#ifdef _WIN32
    return run_command_capture("git rev-parse --git-dir 2>nul");
#else
    return run_command_capture("git rev-parse --git-dir 2>/dev/null");
#endif
}