// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmcoreglobal.h"

bool QMdmmData::isPlaceAdjecent(Place p1, Place p2)
{
    if (p1 == p2)
        return false;

    if ((p1 != Country) && (p2 != Country))
        return false;

    return true;
}

QVersionNumber QMdmmGlobal::version()
{
    return QVersionNumber::fromString(QStringLiteral(QMDMM_VERSION));
}
