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

// Разбор remote вроде
//  - git@github.com:user/repo.git
//  - git@github-alt:user/repo.git (алиас из ~/.ssh/config)
//  - https://github.com/user/repo(.git)
//  - ssh://git@github.com/user/repo(.git)
bool parse_github_remote(const std::string &remote,
                         std::string &user,
                         std::string &repo)
{
    std::string path;

    // Пытаемся найти схему вида "https://", "ssh://", "git://", ...
    auto scheme_pos = remote.find("://");
    if (scheme_pos != std::string::npos)
    {
        // Формат: scheme://[user@]host[:port]/path...
        auto path_start = remote.find('/', scheme_pos + 3);
        if (path_start == std::string::npos || path_start + 1 >= remote.size())
            return false;

        // Отбрасываем ведущий '/', остаётся что-то вроде "user/repo.git" или
        // "user/repo.git/..."
        path = remote.substr(path_start + 1);
    }
    else
    {
        // Формат в стиле scp: [user@]host:path
        auto colon_pos = remote.rfind(':');
        if (colon_pos == std::string::npos || colon_pos + 1 >= remote.size())
            return false;

        // После ':' идёт путь "user/repo.git" или "user/repo.git/..."
        path = remote.substr(colon_pos + 1);
    }

    if (path.empty())
        return false;

    // user / repo[.git] / ...
    auto slash1 = path.find('/');
    if (slash1 == std::string::npos || slash1 == 0 || slash1 + 1 >= path.size())
        return false;

    user = path.substr(0, slash1);

    auto rest = path.substr(slash1 + 1);
    auto slash2 = rest.find('/');
    std::string repo_part =
        (slash2 == std::string::npos) ? rest : rest.substr(0, slash2);

    // обрежем ".git" в конце, если есть
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