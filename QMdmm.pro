include(QMdmm.pri)

TEMPLATE = subdirs

QMdmmServer.depends = QMdmmCore

SUBDIRS += \
    QMdmmCore \
    QMdmmServer

