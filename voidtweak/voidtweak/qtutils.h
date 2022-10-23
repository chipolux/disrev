#ifndef QTUTILS_H
#define QTUTILS_H

#include <QObject>
#include <QtGlobal>

// Custom message handler for Qt logging
void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg);

// Helper to clear a collection of QObject pointers
template <typename Container> inline void qDeleteAllLater(Container &c)
{
    for (QObject *o : qAsConst(c)) {
        o->deleteLater();
    }
    c.clear();
}

/* Helper macros to cut down on Q_PROPERTY boilerplate.
 *
 * RW_PROP(<TYPE>, <PROP NAME>, <SETTER NAME>)
 * RO_PROP(<TYPE>, <PROP NAME>)
 * RW_FUZZY_PROP(<TYPE>, <PROP NAME>, <SETTER NAME>)
 *
 * Setter Name is used to generate a setter function, not for automatically
 * using one. If you need to use custom setters/getters with a property then
 * you'll have to define those manually.
 *
 * Use RW_FUZZY_PROP to use qFuzzyCompare instead of a plain == comparison in
 * the setter, this is required to safely compare floats and doubles.
 *
 * Instead of:
 *    Q_PROPERTY(int thing READ thing WRITE setThing NOTIFY thingChanged)
 * and the m_thing member, signal, slots, and accessor function just do:
 *    RW_PROP(int, thing, setThing)
 *
 * Same for a read only property:
 *    Q_PROPERTY(QString stuff READ stuff NOTIFY stuffChanged)
 * and all that implies you just do:
 *    RO_PROP(QString, stuff)
 *
 * And for either, if you need to pass a complex type that contains a ',' like
 * a QMap signature then you can wrap it in SINGLE_ARG(QMap<QString, QString>)!
 *
 * Use these all down at the bottom of your class to keep things nice and clean
 * for debugging.
 */

// Helper when you need to pass complex types like QMap<Thing, Other>
#define SINGLE_ARG(...) __VA_ARGS__

// Read + Write in C++ & QML
#define RW_PROP(T, R, W)                                                                           \
  public:                                                                                          \
    T R() const                                                                                    \
    {                                                                                              \
        return m_##R;                                                                              \
    }                                                                                              \
  public Q_SLOTS:                                                                                  \
    void W(T R)                                                                                    \
    {                                                                                              \
        if (m_##R == R) {                                                                          \
            return;                                                                                \
        }                                                                                          \
        m_##R = R;                                                                                 \
        emit R##Changed(R);                                                                        \
    }                                                                                              \
  Q_SIGNALS:                                                                                       \
    void R##Changed(T R);                                                                          \
                                                                                                   \
  private:                                                                                         \
    Q_PROPERTY(T R READ R WRITE W NOTIFY R##Changed)                                               \
    T m_##R;

// Read + Write in C++ & QML using fuzzy compare
#define RW_FUZZY_PROP(T, R, W)                                                                     \
  public:                                                                                          \
    T R() const                                                                                    \
    {                                                                                              \
        return m_##R;                                                                              \
    }                                                                                              \
  public Q_SLOTS:                                                                                  \
    void W(T R)                                                                                    \
    {                                                                                              \
        if (qFuzzyCompare(m_##R, R)) {                                                             \
            return;                                                                                \
        }                                                                                          \
        m_##R = R;                                                                                 \
        emit R##Changed(R);                                                                        \
    }                                                                                              \
  Q_SIGNALS:                                                                                       \
    void R##Changed(T R);                                                                          \
                                                                                                   \
  private:                                                                                         \
    Q_PROPERTY(T R READ R WRITE W NOTIFY R##Changed)                                               \
    T m_##R;

// Read Only in C++ & QML
#define RO_PROP(T, R)                                                                              \
  public:                                                                                          \
    T R() const                                                                                    \
    {                                                                                              \
        return m_##R;                                                                              \
    }                                                                                              \
  Q_SIGNALS:                                                                                       \
    void R##Changed(T R);                                                                          \
                                                                                                   \
  private:                                                                                         \
    Q_PROPERTY(T R READ R NOTIFY R##Changed)                                                       \
    T m_##R;

// CONSTANT MEMBER Prop
#define CM_PROP(T, R)                                                                              \
  public:                                                                                          \
    Q_PROPERTY(T R MEMBER R CONSTANT)                                                              \
    T R;

#endif // QTUTILS_H
