#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "TaskFormState.h"

namespace disassemble::desktop {

class TaskPresetStore {
public:
    explicit TaskPresetStore(const std::filesystem::path &rootOverride = {});

    std::filesystem::path rootDirectory() const;
    void saveLastSession(const TaskFormState &state) const;
    std::optional<TaskFormState> loadLastSession() const;
    void saveNamedPreset(const std::string &name, const TaskFormState &state) const;
    std::optional<TaskFormState> loadNamedPreset(const std::string &name) const;
    std::vector<std::string> listPresetNames() const;

private:
    std::filesystem::path baseDirectory_;
};

} // namespace disassemble::desktop
