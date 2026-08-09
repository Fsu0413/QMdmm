// SPDX-License-Identifier: AGPL-3.0-or-later

#include "mainwindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    [[maybe_unused]] QApplication a(argc, argv);

    // Make font suitable for displaying
    QFont font = QApplication::font();
    font.setPixelSize(50);
    QApplication::setFont(font);

    MainWindow mainwindow;

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    mainwindow.showMaximized();
#else
    mainwindow.show();
#endif

    // NOLINTNEXTLINE(clang-analyzer-core.StackAddressEscape)
    return QApplication::exec();
}
