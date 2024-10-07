// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMNETWORKINGGLOBAL_H
#define QMDMMNETWORKINGGLOBAL_H

#include <QMdmmCoreGlobal>

#if 0
class QMDMMNETWORKING_EXPORT QMdmmNetworkingGlobal
#endif

#ifndef QMDMM_STATIC
#ifdef QMDMMSERVER_LIBRARY
#define QMDMMNETWORKING_EXPORT Q_DECL_EXPORT
#else
#define QMDMMNETWORKING_EXPORT Q_DECL_IMPORT
#endif
#else
#define QMDMMNETWORKING_EXPORT
#endif

#ifdef QMDMM_NEED_EXPORT_PRIVATE
#define QMDMMNETWORKING_PRIVATE_EXPORT QMDMMNETWORKING_EXPORT
#else
#define QMDMMNETWORKING_PRIVATE_EXPORT
#endif

#endif // QMDMMSERVER_GLOBAL_H