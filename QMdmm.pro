# SPDX-License-Identifier: AGPL-3.0-or-later

include(QMdmm.pri)

TEMPLATE = subdirs

QMdmmServer.depends = QMdmmCore

SUBDIRS += \
    QMdmmCore \
    QMdmmServer

