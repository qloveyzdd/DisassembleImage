#pragma once

#include <atomic>
#include <thread>

#include <QObject>

#include "../../core/model/ProcessingTask.h"
#include "../../core/model/RunProgress.h"
#include "../../core/model/RunResult.h"

namespace disassemble::desktop {

class TaskRunSession : public QObject
{
    Q_OBJECT

public:
    explicit TaskRunSession(QObject *parent = nullptr);
    ~TaskRunSession() override;

    void start(const disassemble::core::ProcessingTask &task);
    void requestCancel();
    bool isRunning() const;

signals:
    void progressChanged(const disassemble::core::RunProgress &progress);
    void finished(const disassemble::core::RunResult &result);
    void failed(const QString &message);

private:
    void joinWorker();

    std::atomic_bool cancelRequested_{false};
    std::atomic_bool running_{false};
    std::thread worker_;
};

} // namespace disassemble::desktop
