// SPDX-License-Identifier: AGPL-3.0-or-later

#include "mainwindow.h"

#include <QMdmmCoreGlobal>

#include <QApplication>
#include <QFont>
#include <QLocale>
#include <QTranslator>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    [[maybe_unused]] QApplication a(argc, argv);

    // Load the translation matching the system locale (falls back to the English source text).
    QTranslator translator;
    if (translator.load(QLocale(), u"qmdmm"_s, u"_"_s, u":/i18n"_s))
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
