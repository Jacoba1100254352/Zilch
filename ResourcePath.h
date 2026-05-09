// ResourcePath.h
#ifndef ZILCH_RESOURCE_PATH_H
#define ZILCH_RESOURCE_PATH_H

#include <filesystem>
#include <string>

[[nodiscard]] std::string resolveResourcePath(const std::filesystem::path& relativePath);

#endif // ZILCH_RESOURCE_PATH_H
