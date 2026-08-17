// SPDX-License-Identifier: AGPL-3.0-or-later

#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    [[maybe_unused]] QApplication a(argc, argv);

    // Load the translation matching the system locale (falls back to the English source text).
    QTranslator translator;
    if (translator.load(QLocale(), QStringLiteral("qmdmm"), QStringLiteral("_"), QStringLiteral(":/i18n")))
        QApplication::installTranslator(&translator);

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
