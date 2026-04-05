#include "TaskPresetStore.h"

#include <algorithm>
#include <stdexcept>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QString>

namespace fs = std::filesystem;

namespace {

QString sanitizePresetFileName(const std::string &name)
{
    QString value = QString::fromStdString(name).trimmed();
    if (value.isEmpty()) {
        value = QStringLiteral("preset");
    }

    for (qsizetype index = 0; index < value.size(); ++index) {
        const auto ch = value.at(index);
        if (!ch.isLetterOrNumber() && ch != QChar('_') && ch != QChar('-')) {
            value[index] = QChar('_');
        }
    }

    return value + QStringLiteral(".json");
}

QString toJsonMode(disassemble::desktop::InputMode mode)
{
    return mode == disassemble::desktop::InputMode::Directory
        ? QStringLiteral("directory")
        : QStringLiteral("single_image");
}

disassemble::desktop::InputMode fromJsonMode(const QString &mode)
{
    return mode == QStringLiteral("directory")
        ? disassemble::desktop::InputMode::Directory
        : disassemble::desktop::InputMode::SingleImage;
}

QString toJsonDirection(disassemble::core::ProcessingDirection direction)
{
    switch (direction) {
    case disassemble::core::ProcessingDirection::None:
        return QStringLiteral("none");
    case disassemble::core::ProcessingDirection::X:
        return QStringLiteral("x");
    case disassemble::core::ProcessingDirection::Y:
        return QStringLiteral("y");
    }
    return QStringLiteral("none");
}

disassemble::core::ProcessingDirection fromJsonDirection(const QString &direction)
{
    if (direction == QStringLiteral("x")) {
        return disassemble::core::ProcessingDirection::X;
    }
    if (direction == QStringLiteral("y")) {
        return disassemble::core::ProcessingDirection::Y;
    }
    return disassemble::core::ProcessingDirection::None;
}

QString toJsonConflictPolicy(disassemble::core::OutputConflictPolicy policy)
{
    switch (policy) {
    case disassemble::core::OutputConflictPolicy::ForbidOverwrite:
        return QStringLiteral("forbid_overwrite");
    case disassemble::core::OutputConflictPolicy::OverwriteExisting:
        return QStringLiteral("overwrite_existing");
    case disassemble::core::OutputConflictPolicy::AutoRename:
        return QStringLiteral("auto_rename");
    }
    return QStringLiteral("forbid_overwrite");
}

disassemble::core::OutputConflictPolicy fromJsonConflictPolicy(const QString &policy)
{
    if (policy == QStringLiteral("overwrite_existing")) {
        return disassemble::core::OutputConflictPolicy::OverwriteExisting;
    }
    if (policy == QStringLiteral("auto_rename")) {
        return disassemble::core::OutputConflictPolicy::AutoRename;
    }
    return disassemble::core::OutputConflictPolicy::ForbidOverwrite;
}

QJsonObject toJsonObject(const disassemble::desktop::TaskFormState &state, const QString &name)
{
    QJsonObject object;
    object[QStringLiteral("version")] = 1;
    object[QStringLiteral("name")] = name;
    object[QStringLiteral("inputMode")] = toJsonMode(state.inputMode);
    object[QStringLiteral("inputImagePath")] = QString::fromStdString(state.inputImagePath.string());
    object[QStringLiteral("inputDirectory")] = QString::fromStdString(state.inputDirectory.string());
    object[QStringLiteral("outputRoot")] = QString::fromStdString(state.outputRoot.string());
    object[QStringLiteral("autoDetectModels")] = state.autoDetectModels;
    object[QStringLiteral("inputObjPath")] = QString::fromStdString(state.inputObjPath.string());
    object[QStringLiteral("outputObjPath")] = QString::fromStdString(state.outputObjPath.string());
    object[QStringLiteral("direction")] = toJsonDirection(state.direction);
    object[QStringLiteral("outputSizesText")] = QString::fromStdString(state.outputSizesText);
    object[QStringLiteral("prefixesText")] = QString::fromStdString(state.prefixesText);
    object[QStringLiteral("outputConflictPolicy")] = toJsonConflictPolicy(state.outputConflictPolicy);
    object[QStringLiteral("enableParallel")] = state.enableParallel;
    object[QStringLiteral("maxWorkers")] = static_cast<int>(state.maxWorkers);
    return object;
}

disassemble::desktop::TaskFormState fromJsonObject(const QJsonObject &object)
{
    disassemble::desktop::TaskFormState state;
    state.inputMode = fromJsonMode(object.value(QStringLiteral("inputMode")).toString());
    state.inputImagePath = object.value(QStringLiteral("inputImagePath")).toString().toStdString();
    state.inputDirectory = object.value(QStringLiteral("inputDirectory")).toString().toStdString();
    state.outputRoot = object.value(QStringLiteral("outputRoot")).toString().toStdString();
    state.autoDetectModels = object.value(QStringLiteral("autoDetectModels")).toBool(true);
    state.inputObjPath = object.value(QStringLiteral("inputObjPath")).toString().toStdString();
    state.outputObjPath = object.value(QStringLiteral("outputObjPath")).toString().toStdString();
    state.direction = fromJsonDirection(object.value(QStringLiteral("direction")).toString());
    state.outputSizesText = object.value(QStringLiteral("outputSizesText")).toString(QStringLiteral("6144*6720")).toStdString();
    state.prefixesText = object.value(QStringLiteral("prefixesText")).toString(QStringLiteral("radian")).toStdString();
    state.outputConflictPolicy = fromJsonConflictPolicy(object.value(QStringLiteral("outputConflictPolicy")).toString());
    state.enableParallel = object.value(QStringLiteral("enableParallel")).toBool(false);
    state.maxWorkers = static_cast<unsigned int>(std::max(1, object.value(QStringLiteral("maxWorkers")).toInt(1)));
    return state;
}

QJsonObject readJsonFile(const fs::path &path)
{
    QFile file(QString::fromStdString(path.string()));
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("无法读取预设文件: " + path.string());
    }

    const auto document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        throw std::runtime_error("预设文件格式不正确: " + path.string());
    }
    return document.object();
}

void writeJsonFile(const fs::path &path, const QJsonObject &object)
{
    fs::create_directories(path.parent_path());

    QFile file(QString::fromStdString(path.string()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        throw std::runtime_error("无法写入预设文件: " + path.string());
    }
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
}

} // namespace

namespace disassemble::desktop {

TaskPresetStore::TaskPresetStore(const fs::path &rootOverride)
{
    if (!rootOverride.empty()) {
        baseDirectory_ = rootOverride;
        return;
    }

    const auto configRoot = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    baseDirectory_ = fs::path(configRoot.toStdString()) / "task-config";
}

fs::path TaskPresetStore::rootDirectory() const
{
    return baseDirectory_;
}

void TaskPresetStore::saveLastSession(const TaskFormState &state) const
{
    writeJsonFile(baseDirectory_ / "last-session.json", toJsonObject(state, QStringLiteral("__last__")));
}

std::optional<TaskFormState> TaskPresetStore::loadLastSession() const
{
    const fs::path path = baseDirectory_ / "last-session.json";
    if (!fs::exists(path)) {
        return std::nullopt;
    }
    return fromJsonObject(readJsonFile(path));
}

void TaskPresetStore::saveNamedPreset(const std::string &name, const TaskFormState &state) const
{
    const auto fileName = sanitizePresetFileName(name);
    writeJsonFile(baseDirectory_ / "presets" / fileName.toStdString(), toJsonObject(state, QString::fromStdString(name)));
}

std::optional<TaskFormState> TaskPresetStore::loadNamedPreset(const std::string &name) const
{
    const auto fileName = sanitizePresetFileName(name);
    const fs::path path = baseDirectory_ / "presets" / fileName.toStdString();
    if (!fs::exists(path)) {
        return std::nullopt;
    }
    return fromJsonObject(readJsonFile(path));
}

std::vector<std::string> TaskPresetStore::listPresetNames() const
{
    std::vector<std::string> names;
    const fs::path presetDir = baseDirectory_ / "presets";
    if (!fs::exists(presetDir)) {
        return names;
    }

    for (const auto &entry : fs::directory_iterator(presetDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }

        try {
            const auto object = readJsonFile(entry.path());
            names.push_back(object.value(QStringLiteral("name")).toString().toStdString());
        } catch (...) {
            continue;
        }
    }

    std::sort(names.begin(), names.end());
    return names;
}

} // namespace disassemble::desktop
