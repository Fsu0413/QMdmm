// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCLIENT_H
#define QMDMMCLIENT_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmRoom>

#include <QObject>

using QMdmmRoom = QMdmmCore::Room;
namespace QMdmmData = QMdmmCore::Data;

QMDMM_EXPORT_NAME(QMdmmClientConfiguration)
QMDMM_EXPORT_NAME(QMdmmClient)

struct QMDMMNETWORKING_EXPORT QMdmmClientConfiguration final : public QVariantMap
{
    Q_GADGET
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName DESIGNABLE false FINAL)

public:
    static QMDMMNETWORKING_EXPORT const QMdmmClientConfiguration &defaults();

#ifdef Q_MOC_RUN
    Q_INVOKABLE QMdmmClientConfiguration();
    Q_INVOKABLE QMdmmClientConfiguration(const QMdmmClientConfiguration &);
#else
    using QVariantMap::QMap;
    using QVariantMap::operator=;
#endif

    // NOLINTBEGIN(bugprone-macro-parentheses)

#define DEFINE_CONFIGURATION(type, valueName, ValueName) \
    [[nodiscard]] type valueName() const;                \
    void set##ValueName(type valueName);
#define DEFINE_CONFIGURATION2(type, valueName, ValueName) \
    [[nodiscard]] type valueName() const;                 \
    void set##ValueName(const type &valueName);

    // NOLINTEND(bugprone-macro-parentheses)

    DEFINE_CONFIGURATION2(QString, screenName, ScreenName)

#undef DEFINE_CONFIGURATION2
#undef DEFINE_CONFIGURATION
};

class QMdmmClientPrivate;

class QMDMMNETWORKING_EXPORT QMdmmClient final : public QObject
{
    Q_OBJECT

public:
    explicit QMdmmClient(QMdmmClientConfiguration clientConfiguration, QObject *parent = nullptr);
    ~QMdmmClient() override;

    bool connectToHost(const QString &host, QMdmmData::AgentState initialState);

    [[nodiscard]] QMdmmRoom *room();
    [[nodiscard]] const QMdmmRoom *room() const;

public slots: // NOLINT(readability-redundant-access-specifiers)
    void notifySpeak(const QString &content);
    void notifyOperate(const void *todo);

    // There is no request timer called here. The timeout can be established in UI.
    // This function can be called for a timeout request, to generate a default reply and reply to server
    void requestTimeout();

    void replyStoneScissorsCloth(QMdmmData::StoneScissorsCloth stoneScissorsCloth);
    void replyActionOrder(const QList<int> &actionOrder);
    void replyAction(QMdmmData::Action action, const QString &toPlayer, int toPlace);
    void replyUpgrade(const QList<QMdmmData::UpgradeItem> &upgrades);

signals:
    void socketErrorDisconnected(const QString &errorString, QPrivateSignal);

    void requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder, QPrivateSignal);
    void requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum, QPrivateSignal);
    void requestAction(int currentOrder, QPrivateSignal);
    void requestUpgrade(int remainingTimes, QPrivateSignal);

    void notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmData::AgentState &agentState, QPrivateSignal);
    void notifyPlayerRemoved(const QString &playerName, QPrivateSignal);
    void notifyGameStart(QPrivateSignal);
    void notifyRoundStart(QPrivateSignal);
    void notifyStoneScissorsCloth(const QHash<QString, QMdmmData::StoneScissorsCloth> &ssc, QPrivateSignal);
    void notifyActionOrder(const QHash<int, QString> &actionOrderResult, QPrivateSignal);
    void notifyAction(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace, QPrivateSignal);
    void notifyRoundOver(QPrivateSignal);
    void notifyUpgrade(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades, QPrivateSignal);
    void notifyGameOver(const QStringList &winners, QPrivateSignal);
    void notifySpoken(const QString &playerName, const QString &content, QPrivateSignal);
    void notifyOperated(QPrivateSignal);

private:
    friend class QMdmmClientPrivate;
    // QMdmmClientPrivate is QObject. QPointer can't be used since it is incomplete here
    QMdmmClientPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmClient);
};

#endif
