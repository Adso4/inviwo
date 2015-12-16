/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/qt/widgets/properties/filepropertywidgetqt.h>
#include <inviwo/qt/widgets/inviwofiledialog.h>

#include <warn/push>
#include <warn/ignore/all>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStandardPaths>
#endif

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QSettings>
#include <QUrl>
#include <warn/pop>

namespace inviwo {

FilePropertyWidgetQt::FilePropertyWidgetQt(FileProperty* property)
    : PropertyWidgetQt(property), property_(property) {
    generateWidget();
    updateFromProperty();
}

void FilePropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    setLayout(hLayout);

    label_ = new EditableLabelQt(this, property_);
    hLayout->addWidget(label_);

    QHBoxLayout* hWidgetLayout = new QHBoxLayout();
    hWidgetLayout->setContentsMargins(0, 0, 0, 0);
    QWidget* widget = new QWidget();
    widget->setLayout(hWidgetLayout);

    lineEdit_ = new QLineEdit(this);
    lineEdit_->setReadOnly(true);

    QSizePolicy sp = lineEdit_->sizePolicy();
    sp.setHorizontalStretch(3);
    lineEdit_->setSizePolicy(sp);
    hWidgetLayout->addWidget(lineEdit_);

    auto revealButton = new QToolButton(this);
    revealButton->setIcon(QIcon(":/icons/reveal.png"));
    hWidgetLayout->addWidget(revealButton);
    connect(revealButton, &QToolButton::pressed, [&]() {
        auto dir = filesystem::directoryExists(property_->get())
                       ? property_->get()
                       : filesystem::getFileDirectory(property_->get());

        QDesktopServices::openUrl(
            QUrl(QString::fromStdString("file:///" + dir), QUrl::TolerantMode));
    });

    openButton_ = new QToolButton(this);
    openButton_->setIcon(QIcon(":/icons/open.png"));
    hWidgetLayout->addWidget(openButton_);
    connect(openButton_, SIGNAL(pressed()), this, SLOT(setPropertyValue()));

    sp = widget->sizePolicy();
    sp.setHorizontalStretch(3);
    widget->setSizePolicy(sp);
    hLayout->addWidget(widget);
}

void FilePropertyWidgetQt::setPropertyValue() {
    std::string path{ property_->get() };

    if (!path.empty()) {
        if (filesystem::directoryExists(path)) {  // if a folder is selected
            // TODO: replace with filesystem:: functionality!
            path = QDir(QString::fromStdString(path)).absolutePath().toStdString();
        } else if (filesystem::fileExists(path)) {
            // if a file is selected, set path the the folder, not the file
            path = QDir(QString::fromStdString(filesystem::getFileDirectory(path)))
                       .absolutePath()
                       .toStdString();
        }
    }

    // Setup Extensions
    std::vector<FileExtension> filters = property_->getNameFilters();
    InviwoFileDialog importFileDialog(this, property_->getDisplayName(),
                                      property_->getContentType(), path);

    for (const auto& filter : filters) importFileDialog.addExtension(filter);

    switch (property_->getAcceptMode()) {
        case FileProperty::AcceptMode::Save:
            importFileDialog.setAcceptMode(QFileDialog::AcceptSave);
            break;

        case FileProperty::AcceptMode::Open:
            importFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
            break;

        default:
            importFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    }

    switch (property_->getFileMode()) {
        case FileProperty::FileMode::AnyFile:
            importFileDialog.setFileMode(QFileDialog::AnyFile);
            break;

        case FileProperty::FileMode::ExistingFile:
            importFileDialog.setFileMode(QFileDialog::ExistingFile);
            break;

        case FileProperty::FileMode::Directory:
            importFileDialog.setFileMode(QFileDialog::Directory);
            break;

        case FileProperty::FileMode::ExistingFiles:
            importFileDialog.setFileMode(QFileDialog::ExistingFiles);
            break;

        case FileProperty::FileMode::DirectoryOnly:
            importFileDialog.setFileMode(QFileDialog::DirectoryOnly);
            break;

        default:
            importFileDialog.setFileMode(QFileDialog::AnyFile);
            break;
    }

    if (importFileDialog.exec()) {
        QString path = importFileDialog.selectedFiles().at(0);
        property_->set(path.toStdString());
    }

    updateFromProperty();
}

bool FilePropertyWidgetQt::requestFile() {
   setPropertyValue();
   return !property_->get().empty();
}

void FilePropertyWidgetQt::updateFromProperty() {
    lineEdit_->setText(QFileInfo(QString::fromStdString(property_->get())).fileName());
}

}  // namespace
