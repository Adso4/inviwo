/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/raiiutils.h>

#include <thread>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QCursor>
#include <warn/pop>

namespace inviwo {

InviwoApplicationQt::InviwoApplicationQt(std::string displayName, int& argc, char** argv)
    : QApplication(argc, argv)
    , InviwoApplication(argc, argv, displayName)
    , mainWindow_(nullptr)
    , uiLocal_(getCurrentStdLocale()) {

    QCoreApplication::setOrganizationName("Inviwo Foundation");
    QCoreApplication::setOrganizationDomain("inviwo.org");
    QCoreApplication::setApplicationName(displayName.c_str());

    setPostEnqueueFront([this]() { postEvent(this, new InviwoQtEvent()); });

    fileWatcher_ = new QFileSystemWatcher(this);
    connect(fileWatcher_, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));

    // Make qt write errors in the console;
    qInstallMessageHandler(&InviwoApplicationQt::logQtMessages);
}

void InviwoApplicationQt::setMainWindow(QMainWindow* mainWindow) { 
    mainWindow_ = mainWindow; 
    // Enable widgets to find the main window using the object name
    mainWindow_->setObjectName("InviwoMainWindow");
}

void InviwoApplicationQt::registerFileObserver(FileObserver* fileObserver) {
    ivwAssert(std::find(fileObservers_.cbegin(), fileObservers_.cend(), fileObserver) ==
                  fileObservers_.cend(),
              "File observer already registered.");
    fileObservers_.push_back(fileObserver);
}

void InviwoApplicationQt::unRegisterFileObserver(FileObserver* fileObserver) {
    util::erase_remove(fileObservers_, fileObserver);
}

void InviwoApplicationQt::startFileObservation(std::string fileName) {
    QString qFileName = QString::fromStdString(fileName);
    if (!fileWatcher_->files().contains(qFileName)) fileWatcher_->addPath(qFileName);
}

void InviwoApplicationQt::stopFileObservation(std::string fileName) {
    QString qFileName = QString::fromStdString(fileName);
    if (fileWatcher_->files().contains(qFileName)) fileWatcher_->removePath(qFileName);
}

void InviwoApplicationQt::fileChanged(QString fileName) {
    wait(200);

    if (QFile::exists(fileName)) {
        std::string fileNameStd = fileName.toLocal8Bit().constData();

        // don't use iterators here, they might be invalidated.
        size_t size = fileObservers_.size();
        for (size_t i = 0; i < size && i < fileObservers_.size(); ++i) {
            if (fileObservers_[i]->isObserved(fileNameStd)) {
                fileObservers_[i]->fileChanged(fileNameStd);
            }
        }

        if (!fileWatcher_->files().contains(fileName)) {
            fileWatcher_->addPath(fileName);
        }
    }
}

void InviwoApplicationQt::closeInviwoApplication() { QCoreApplication::quit(); }

void InviwoApplicationQt::playSound(Message message) {}

std::locale InviwoApplicationQt::getUILocale() const { return uiLocal_; }

void InviwoApplicationQt::printApplicationInfo() {
    InviwoApplication::printApplicationInfo();
    LogInfoCustom("InviwoInfo", "Qt Version " << QT_VERSION_STR);
}

void InviwoApplicationQt::resizePool(size_t newSize) {
    auto start = std::chrono::system_clock::now();
    std::chrono::milliseconds timelimit(250);
    auto timeout = [&timelimit, &start]() {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() -
                                                                start) > timelimit;
    };

    // try resizing for timelimit time, run processFront to avoid potential deadlocks.
    size_t size = pool_.trySetSize(newSize);
    while (size != newSize && !timeout()) {
        processFront();
        size = pool_.trySetSize(newSize);
    }
    if (size == newSize) return;

    // if not done yet, continue trying, but block interaction and keep the GUI running by also
    // calling processEvents
    auto enabled = mainWindow_->isEnabled();
    util::OnScopeExit cleanup{[this, enabled]() {
        mainWindow_->setEnabled(enabled);
        QApplication::restoreOverrideCursor();
    }};
    mainWindow_->setEnabled(false);
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    while (size != newSize) {
        if (timeout()) {
            auto left = size - newSize;
            LogInfo("Waiting for " << left << " background thread" << (left > 1 ? "s" : "")
                                   << " to finish");
            timelimit += std::chrono::milliseconds(1000);
        }

        size = pool_.trySetSize(newSize);
        processFront();
        processEvents();
    }
}

void InviwoApplicationQt::wait(int ms) {
    if (ms <= 0) return;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


void InviwoApplicationQt::logQtMessages(QtMsgType type, const QMessageLogContext& context,
                                        const QString& msg) {
#ifdef IVW_DEBUG
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            inviwo::LogCentral::getPtr()->log("Qt Debug", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            break;
        case QtWarningMsg:
            inviwo::LogCentral::getPtr()->log("Qt Warning", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            break;
        case QtCriticalMsg:
            inviwo::LogCentral::getPtr()->log("Qt Critical", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            break;
        case QtFatalMsg:
            inviwo::LogCentral::getPtr()->log("Qt Fatal", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            abort();
            break;

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
        case QtInfoMsg:
            inviwo::LogCentral::getPtr()->log("Qt Info", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            break;
        #endif
    }
#endif
}

bool InviwoApplicationQt::event(QEvent* e) {
    if (e->type() == InviwoQtEvent::type()) {
        e->accept();
        processFront();
        return true;
    } else {
        return QApplication::event(e);
    }
}

std::locale InviwoApplicationQt::getCurrentStdLocale() {
    std::locale loc;
    try {
        // use the system locale provided by Qt

#ifdef WIN32
        // need to change locale given by Qt from underscore to hyphenated ("sv_SE" to "sv-SE")
        // although std::locale should only accept locales with underscore, e.g. "sv_SE"
        std::string localeName(QLocale::system().name().replace('_', '-').toStdString());
#else
        std::string localeName(QLocale::system().name().toStdString());
#endif
        loc = std::locale(localeName.c_str());
    }
    catch (std::exception &e) {
        LogWarnCustom("getStdLocale", "Locale could not be set. " << e.what());
    }
    return loc;
}

QEvent::Type InviwoQtEvent::InviwoQtEventType = QEvent::None;

}  // namespace