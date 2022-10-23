#include "decl.h"

#include <QDebug>

namespace decl
{

QString parse(const QByteArray &data, QList<Scope> &entities)
{
    qInfo() << "Started parsing entities...";
    if (data.first(9) != QByteArrayLiteral("Version 6")) {
        return u"Unknown header!"_qs;
    }
    QByteArray token;
    QList<QByteArray> tokens;
    QList<Scope> stack;
    bool inEscape = false;
    bool inString = false;
    for (int i = 9; i < data.size(); ++i) {
        const auto c = data[i];
        // shortcut escape sequences
        if (inEscape) {
            token.append(c);
            inEscape = false;
            continue;
        } else if (c == '\\') {
            inEscape = true;
            continue;
        }
        // shortcut characters within a string and string toggles
        if (inString || c == '"' || c == '\'') {
            token.append(c);
            if (c == '"' || c == '\'') {
                inString = !inString;
            }
            continue;
        }
        // detect end of token and insert it into token cache
        if (!token.isEmpty() && QByteArrayLiteral(" \t\r\n={};").contains(c)) {
            tokens.append(token);
            token.clear();
        }
        switch (c) {
        // skip these control characters
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '=': {
            break;
        }
        // handle value end marker
        case ';': {
            if (stack.isEmpty()) {
                return u"Missing stack frame for key-value pair @ %1"_qs.arg(i);
            }
            if (tokens.isEmpty()) {
                return u"Missing tokens for key-value pair @ %1"_qs.arg(i);
            }
            // NOTE: some values have a key that is made up of 2+ tokens
            // we just combine them into 1 key string for now
            QByteArray value = tokens.takeLast();
            const int keyStart = stack.count() - 1;
            const int keyEnd = tokens.count() - 1;
            QByteArray key;
            for (int ki = keyEnd; ki >= keyStart; ki--) {
                key.prepend((ki == keyStart ? "" : " ") + tokens.takeAt(ki));
            }
            stack.last().append({QString(key), QString(value)});
            break;
        }
        // handle start of nested scope
        case '{': {
            if (stack.isEmpty() && tokens.count() == 3) {
                stack.append({
                    {u"entityId"_qs, QString(tokens.takeLast())},
                    {u"entityType"_qs, QString(tokens.takeLast())},
                    {u"definitionType"_qs, QString(tokens.takeLast())},
                });
            } else if (!stack.isEmpty()) {
                stack.append(Scope());
            }
            break;
        }
        // handle exiting nested scope
        case '}': {
            if (stack.count() == 1 && tokens.isEmpty()) {
                entities.append(stack.takeLast());
            } else if (stack.count() > 1 && !tokens.isEmpty()) {
                QVariant scope = QVariant::fromValue(stack.takeLast());
                stack.last().append({QString(tokens.takeLast()), scope});
            }
            break;
        }
        default: {
            token.append(c);
            break;
        }
        }
    }
    if (!stack.isEmpty() || !tokens.isEmpty()) {
        return u"Bad entities, leftover data!"_qs;
    }
    qInfo() << "Loaded entities:" << entities.count();
    return {};
}

} // namespace decl
