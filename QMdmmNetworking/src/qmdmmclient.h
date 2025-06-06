// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCLIENT_H
#define QMDMMCLIENT_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmRoom>

#include <QObject>

QMDMM_EXPORT_NAME(QMdmmClientConfiguration)
QMDMM_EXPORT_NAME(QMdmmClient)

namespace QMdmmNetworking {
namespace p {
class ClientP;
}

namespace v0 {

struct QMDMMNETWORKING_EXPORT ClientConfiguration final : public QVariantMap
{
    Q_GADGET
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName DESIGNABLE false FINAL)

public:
    static QMDMMNETWORKING_EXPORT const ClientConfiguration &defaults();

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

class QMDMMNETWORKING_EXPORT Client final : public QObject
{
    Q_OBJECT

public:
    explicit Client(ClientConfiguration clientConfiguration, QObject *parent = nullptr);
    ~Client() override;

    bool connectToHost(const QString &host, QMdmmCore::Data::AgentState initialState);

    [[nodiscard]] QMdmmCore::Room *room();
    [[nodiscard]] const QMdmmCore::Room *room() const;

public slots: // NOLINT(readability-redundant-access-specifiers)
    void notifySpeak(const QString &content);
    void notifyOperate(const void *todo);

    // There is no request timer called here. The timeout can be established in UI.
    // This function can be called for a timeout request, to generate a default reply and reply to server
    void requestTimeout();

    void replyStoneScissorsCloth(QMdmmCore::Data::StoneScissorsCloth stoneScissorsCloth);
    void replyActionOrder(const QList<int> &actionOrder);
    void replyAction(QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    void replyUpgrade(const QList<QMdmmCore::Data::UpgradeItem> &upgrades);

signals:
    void socketErrorDisconnected(const QString &errorString, QPrivateSignal);

    void requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder, QPrivateSignal);
    void requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum, QPrivateSignal);
    void requestAction(int currentOrder, QPrivateSignal);
    void requestUpgrade(int remainingTimes, QPrivateSignal);

    void notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState, QPrivateSignal);
    void notifyPlayerRemoved(const QString &playerName, QPrivateSignal);
    void notifyGameStart(QPrivateSignal);
    void notifyRoundStart(QPrivateSignal);
    void notifyStoneScissorsCloth(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &ssc, QPrivateSignal);
    void notifyActionOrder(const QHash<int, QString> &actionOrderResult, QPrivateSignal);
    void notifyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace, QPrivateSignal);
    void notifyRoundOver(QPrivateSignal);
    void notifyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades, QPrivateSignal);
    void notifyGameOver(const QStringList &winners, QPrivateSignal);
    void notifySpoken(const QString &playerName, const QString &content, QPrivateSignal);
    void notifyOperated(QPrivateSignal);

private:
    friend class p::ClientP;
    // ClientP is QObject. QPointer can't be used since it is incomplete here
    p::ClientP *const d;
    Q_DISABLE_COPY_MOVE(Client);
};
} // namespace v0

inline namespace v1 {
using v0::Client;
using v0::ClientConfiguration;
} // namespace v1
} // namespace QMdmmNetworking

#endif
