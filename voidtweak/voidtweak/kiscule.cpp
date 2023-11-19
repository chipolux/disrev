#include "kiscule.h"

#include <QDebug>
#include <QVariant>

#include <decl.h>

namespace kiscule
{

/* NOTE: moved this out so we can eventually add stuff like getPoint, getQuat, getMat
 *       that can more efficiently get multiple values at one level rather than
 *       drilling down through all levels multiple times.
 */
inline void stepPath(QString &key, QString &path)
{
    // key == path when we are at the last element
    key = path;
    const auto index = path.indexOf('.');
    if (index > 0) {
        // if we see any '.' then we still have elements to go
        key = path.first(index);
        path.remove(0, index + 1);
    } else {
        // if we didn't see a '.' then we know we want a value from this entry
        path.clear();
    }
}

QString getValue(const decl::EntityEntry *entry, QString path)
{
    // we're at the last step on a long journey (or the first step...)
    if (path.isEmpty()) {
        return entry->value();
    }

    // extract key and modify path
    QString key;
    stepPath(key, path);

    // search for a matching key to extract a value from (or continue deeper)
    for (const auto e : entry->entries()) {
        if (e->key() == key) {
            return getValue(e, path);
        }
    }

    // we did not find a matching key, this is effectively an error state, bad path
    return {};
}

Comment::Comment(const decl::EntityEntry *entry, QObject *parent)
    : QObject(parent)
    , m_id()
    , m_rect()
    , m_outsideText()
    , m_insideText()
{
    m_id = getValue(entry, u"m_id"_qs).toInt();
    m_rect.setX(getValue(entry, u"m_pos.x"_qs).toInt());
    m_rect.setY(getValue(entry, u"m_pos.y"_qs).toInt());
    m_rect.setWidth(getValue(entry, u"m_size.x"_qs).toInt());
    m_rect.setHeight(getValue(entry, u"m_size.y"_qs).toInt());
    m_outsideText = getValue(entry, u"m_parameters.m_outsideText"_qs);
    m_insideText = getValue(entry, u"m_parameters.m_insideText"_qs);
    if (m_rect.width() < 0 || m_rect.height() < 0) {
        qCritical() << "Rect with negative size:" << m_id << "=" << m_rect;
    }
}

Node::Node(const decl::EntityEntry *entry, QObject *parent)
    : QObject(parent)
    , m_id()
    , m_pos()
    , m_name()
{
    m_id = getValue(entry, u"m_id"_qs).toInt();
    m_pos.setX(getValue(entry, u"m_nodePos.x"_qs).toInt());
    m_pos.setY(getValue(entry, u"m_nodePos.y"_qs).toInt());
    m_name = getValue(entry, u"m_name"_qs);
}

Root::Root(const decl::EntityEntry *entry, QObject *parent)
    : QObject(parent)
    , m_version()
    , m_comments()
    , m_lastPosition()
    , m_lastZoomLevel()
{
    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
    for (const auto e : entry->entries()) {
        const auto &key = e->key();
        if (key == u"m_version"_qs) {
            m_version = e->value().toInt();
        } else if (key == u"m_lastZoomLevel"_qs) {
            m_lastZoomLevel = e->value().toDouble();
        } else if (key == u"m_comments"_qs) {
            for (const auto c : e->entries()) {
                if (c->key() == u"num"_qs) {
                    // NOTE: list size may be useful optimization
                    continue;
                }
                auto comment = new Comment(c, this);
                const auto &r = comment->rect();
                minX = qMin(minX, r.x());
                maxX = qMax(maxX, r.x() + r.width());
                minY = qMin(minY, r.y());
                maxY = qMax(maxY, r.y() + r.height());
                m_comments.append(comment);
            }
        } else if (key == u"m_nodes"_qs) {
            for (const auto n : e->entries()) {
                if (n->key() == u"num"_qs) {
                    // NOTE: list size may be useful optimization
                    continue;
                }
                auto node = new Node(n, this);
                minX = qMin(minX, node->pos().x());
                maxX = qMax(maxX, node->pos().x());
                minY = qMin(minY, node->pos().y());
                maxY = qMax(maxY, node->pos().y());
                m_nodes.append(node);
            }
        }
    }
    m_boundingBox = {minX, minY, maxX - minX, maxY - minY};
}

} // namespace kiscule
