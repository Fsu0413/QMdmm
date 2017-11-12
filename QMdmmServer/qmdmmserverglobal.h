#ifndef QMDMMSERVERGLOBAL_H
#define QMDMMSERVERGLOBAL_H

#include <QMdmmCore/QMdmmCoreGlobal>
#include <cstddef>

#if 0
class QMDMMSERVER_EXPORT QMdmmServerGlobal
#endif

#ifndef QMDMM_STATIC
#ifdef _WIN32
#ifdef QMDMMSERVER_LIBRARY
#define QMDMMSERVER_EXPORT __declspec(dllexport)
#else
#define QMDMMSERVER_EXPORT __declspec(dllimport)
#endif
#else
#ifdef QMDMMSERVER_LIBRARY
#define QMDMMSERVER_EXPORT __attribute__((visibility("default")))
#else
#define QMDMMSERVER_EXPORT
#endif
#endif
#else
#define QMDMMSERVER_EXPORT
#endif

#endif // QMDMMSERVER_GLOBAL_H
