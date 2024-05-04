// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVERGLOBAL_H
#define QMDMMSERVERGLOBAL_H

#include <QMdmmCore/QMdmmCoreGlobal>

#if 0
class QMDMMSERVER_EXPORT QMdmmServerGlobal
#endif

#ifndef QMDMM_STATIC
#ifdef QMDMMSERVER_LIBRARY
#define QMDMMSERVER_EXPORT Q_DECL_EXPORT
#else
#define QMDMMSERVER_EXPORT Q_DECL_IMPORT
#endif
#else
#define QMDMMSERVER_EXPORT
#endif

#endif // QMDMMSERVER_GLOBAL_H
