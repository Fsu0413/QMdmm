#ifndef QMDMMCORE_TEST_TEST_H_
#define QMDMMCORE_TEST_TEST_H_

#include <QMetaObject>

#include <type_traits>

const QMetaObject *registerTestObjectImpl(const QMetaObject *metaObject=nullptr);

template<typename T>
struct RegisterTestObject
{
    static_assert(std::is_base_of_v<QObject, std::remove_cv_t<T>>);
    static_assert(std::is_convertible_v<typename std::remove_cv_t<T> *, QObject *>);

    RegisterTestObject()
    {
        registerTestObjectImpl(&T::staticMetaObject);
    }
};

#endif
