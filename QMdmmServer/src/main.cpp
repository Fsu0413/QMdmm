// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QCoreApplication>
#include <QScopedPointer>

#if 0
-h --help
-v --version

TCP options:
-t --tcp Enable TCP
-p --tcp-port=<port> TCP listen port

LocalSocket options:
-l --local Enable local socket
-L --local-name=<name> local socket name

WebSocket options:
-w --websocket Enable WebSocket
-W --websocket-name=<name> WebSocket name
-P --websocket-port=<port> WebSocket listen port

LogicRunner configurations:
-n --players=<2~> player number per Room
-o --timeout=<0,15~> operation timeout

Logic configurations:
-s --slash=, --knife=<1~> initial knife (slash) damage
-S --maximum-slash=, --maximum-knife=<5~> maximum knife (slash) damage
-k --kick=, --horse=<2~> initial horse (kick) damage
-K --maximum-kick=, --maximum-horse=<5~> maximum horse (kick) damage
-m --maxhp=<7~> initial max hp
-M --maximum-maxhp=<10~> maximum max hp
-r --punish-hp-modifier=<0,2~> punish hp modifier
-R --punish-hp-round-strategy=<strategy> punish hp round strategy
-z --zero-hp-as-dead=<true/false> treat zero hp as dead
-f --enable-let-move=<true/false> enable "let move"
-i --can-buy-only-in-initial-city=<true/false> can buy only in initial city

abcde g  j    q  u  xy
ABCDEFGHIJ NO Q TUV XYZ
0123456789
#endif

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> a(new QCoreApplication(argc, argv));
    return QCoreApplication::exec();
}
