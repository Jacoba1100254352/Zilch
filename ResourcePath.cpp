// ResourcePath.cpp
#include "ResourcePath.h"

#include <system_error>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace {
std::filesystem::path executableDirectory()
{
#if defined(__APPLE__)
    uint32_t size = 0;
    (void)_NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(static_cast<std::size_t>(size) + 1U, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) == 0)
        return std::filesystem::path(buffer.data()).parent_path();
#elif defined(_WIN32)
    std::vector<wchar_t> buffer(MAX_PATH);
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length > 0 && length < buffer.size())
        return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
#elif defined(__linux__)
    std::vector<char> buffer(4096, '\0');
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1U);
    if (length > 0)
        return std::filesystem::path(std::string(buffer.data(), static_cast<std::size_t>(length))).parent_path();
#endif
    return {};
}
}

std::string resolveResourcePath(const std::filesystem::path& relativePath)
{
    if (relativePath.is_absolute())
        return relativePath.string();

    std::error_code ec;
    const std::vector<std::filesystem::path> roots = {
        std::filesystem::current_path(ec),
        executableDirectory()
    };

    for (const std::filesystem::path& root : roots) {
        if (root.empty())
            continue;
        const std::filesystem::path candidate = root / relativePath;
        ec.clear();
        if (std::filesystem::exists(candidate, ec))
            return candidate.string();
    }

    return relativePath.string();
}
