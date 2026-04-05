#include "TaskRunSession.h"

#include <exception>

#include <QMetaObject>

#include "RunController.h"

namespace disassemble::desktop {

TaskRunSession::TaskRunSession(QObject *parent)
    : QObject(parent)
{
}

TaskRunSession::~TaskRunSession()
{
    requestCancel();
    joinWorker();
}

void TaskRunSession::start(const disassemble::core::ProcessingTask &task)
{
    joinWorker();
    cancelRequested_.store(false);
    running_.store(true);

    worker_ = std::thread([this, task]() {
        try {
            const auto result = RunController::runTask(
                task,
                [this](const disassemble::core::RunProgress &progress) {
                    QMetaObject::invokeMethod(this, [this, progress]() {
                        emit progressChanged(progress);
                    }, Qt::QueuedConnection);
                },
                [this]() {
                    return cancelRequested_.load();
                });

            running_.store(false);
            QMetaObject::invokeMethod(this, [this, result]() {
                emit finished(result);
            }, Qt::QueuedConnection);
        } catch (const std::exception &error) {
            running_.store(false);
            const auto message = QString::fromUtf8(error.what());
            QMetaObject::invokeMethod(this, [this, message]() {
                emit failed(message);
            }, Qt::QueuedConnection);
        }
    });
}

void TaskRunSession::requestCancel()
{
    cancelRequested_.store(true);
}

bool TaskRunSession::isRunning() const
{
    return running_.load();
}

void TaskRunSession::joinWorker()
{
    if (worker_.joinable()) {
        worker_.join();
    }
}

} // namespace disassemble::desktop
