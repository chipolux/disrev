#ifndef BWM_H
#define BWM_H

/* BWM Format (little-endian unless otherwise noted)
 * struct BWMFile {
 *   char[4]      // "BWM1"
 *   uint16       // always 21?
 *   uint16       // always 1?
 *   uint32       // always 2?
 *
 *   uint32       // vertex coord byte count
 *   char[n]      // vertex coords (groups of 3 floats)
 *
 *   uint32       // vertex data byte count
 *   char[n]      // vertex data (2hf uv1, 2hf uv2, 4b norm, 4b tang, 4B color)
 *
 *   uint32       // vertex index byte count
 *   char[n]      // vertex indexes (3H tris)
 *
 *   uint32       // vertex matrix byte count
 *   VMatrix[n]   // vertex matrices
 *
 *   uint32       // mystery byte count
 *   char[n]      // mystery data
 *
 *   uint32       // mesh count
 *   Mesh[n]      // meshes
 *
 *   uint32       // object count
 *   Object[n]    // objects
 *
 *   uint32       // instance count
 *   Instance[n]  // instances
 *
 *   uint32       // c1 count
 *   uint32       // ??
 *   uint32       // c2 count
 *   uint32       // ??
 *   uint32[n]    // c1 data
 *   uint32[n]    // c2 data
 *   uint32       // c3 count
 *   { uint32, uint32, uint32[n] }[n]
 *
 *   uint32       // group count
 *   Group[n+1]   // groups
 * }
 *
 * struct VMatrix {
 *   float[3]     // x row?
 *   float[3]     // y row?
 *   float[3]     // z row?
 *   float[3]     // w row?
 * }
 *
 * struct Mesh {
 *   char[4]      // mystery data (expect 02 00 00 00)
 *   uint32       // vertex coord offset
 *   uint32       // vertex coord stride (expect 12)
 *   uint32       // vertex data offset
 *   uint32       // vertex data stride (expect 20)
 *   char[4]      // mystery data (expect 12 34 56 78)
 *   uint32       // vertex index offset
 *   uint32       // vertex index count
 *   uint32       // vertex count
 *   char[5]      // mystery data (expect 03 00 00 00 01)
 * }
 *
 * struct Object {
 *   uint32       // matrix start
 *   uint32       // matrix end
 *   uint32       // mesh index
 *   bool8        // is flipped
 *   uint32       // material path length
 *   char[n]      // material path (utf-8)
 *   uint32       // is LOD
 * }
 *
 * struct Instance {
 *   float[3]     // bounding box min
 *   float[3]     // bounding box max
 *   char[4]      // mystery bytes
 *   int16        // mystery (expect -1)
 * }
 *
 * struct Group {
 *   uint32       // instance indexes count
 *   uint32[n]    // instance indexes
 * }
 */

#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QtQml>

namespace bwm
{

using Group = QList<quint32>;
using LabeledGroup = QPair<quint32, QList<quint32>>;

/* Matrices are stored as a set of 12 single precision floats like so:
 *    ┌           ┐┌    ┐
 *  x │ 0   1   2 ││  3 │
 *  y │ 4   5   6 ││  7 │
 *  z │ 8   9  10 ││ 11 │
 *    └           ┘└    ┘
 * This is your standard affine transformation matrix minus the identity row
 * for w. First three columns are transformation, final column is translation.
 */
struct PODMatrix {
    qint64 offset;
    float values[12];
};
QDebug operator<<(QDebug d, const PODMatrix &m);

struct PODMesh {
    qint64 offset;
    quint32 unk1; // mystery data (expect 02 00 00 00)
    quint32 vco;  // vertex coord offset
    quint32 vcs;  // vertex coord stride (expect 12)
    quint32 vdo;  // vertex data offset
    quint32 vds;  // vertex data stride (expect 20)
    quint32 unk2; // mystery data (expect 12 34 56 78)
    quint32 vio;  // vertex index offset
    quint32 vic;  // vertex index count
    quint32 vc;   // vertex count
    char unk3[5]; // mystery data (expect 03 00 00 00 01)
};
QDebug operator<<(QDebug d, const PODMesh &m);

struct PODInstance {
    qint64 offset;
    float min[3];
    float max[3];
    quint32 unk1;
    qint16 unk2;
};
QDebug operator<<(QDebug d, const PODInstance &m);

struct PODObject {
    qint64 offset;
    quint32 indexStart; // matrix/instance start
    quint32 indexEnd;   // matrix/instance end
    quint32 meshIndex;  // mesh index
    quint8 isFlipped;   // is flipped
    QString materialPath;
    quint32 lod;
    QList<PODMatrix> matrices;
    QList<PODInstance> instances;
};
QDebug operator<<(QDebug d, const PODObject &m);

class Matrix : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

    Q_PROPERTY(qint64 offset READ offset CONSTANT)
    Q_PROPERTY(float x1 READ x1 WRITE setX1 NOTIFY dataChanged)
    Q_PROPERTY(float x2 READ x2 WRITE setX2 NOTIFY dataChanged)
    Q_PROPERTY(float x3 READ x3 WRITE setX3 NOTIFY dataChanged)
    Q_PROPERTY(float x4 READ x4 WRITE setX4 NOTIFY dataChanged)
    Q_PROPERTY(float y1 READ y1 WRITE setY1 NOTIFY dataChanged)
    Q_PROPERTY(float y2 READ y2 WRITE setY2 NOTIFY dataChanged)
    Q_PROPERTY(float y3 READ y3 WRITE setY3 NOTIFY dataChanged)
    Q_PROPERTY(float y4 READ y4 WRITE setY4 NOTIFY dataChanged)
    Q_PROPERTY(float z1 READ z1 WRITE setZ1 NOTIFY dataChanged)
    Q_PROPERTY(float z2 READ z2 WRITE setZ2 NOTIFY dataChanged)
    Q_PROPERTY(float z3 READ z3 WRITE setZ3 NOTIFY dataChanged)
    Q_PROPERTY(float z4 READ z4 WRITE setZ4 NOTIFY dataChanged)

  public:
    explicit Matrix(const PODMatrix &data, QObject *parent);
    PODMatrix pod() const { return m_data; }
    const qint64 &offset() const { return m_data.offset; }
    const float &x1() const { return m_data.values[0]; }
    const float &x2() const { return m_data.values[1]; }
    const float &x3() const { return m_data.values[2]; }
    const float &x4() const { return m_data.values[3]; }
    const float &y1() const { return m_data.values[4]; }
    const float &y2() const { return m_data.values[5]; }
    const float &y3() const { return m_data.values[6]; }
    const float &y4() const { return m_data.values[7]; }
    const float &z1() const { return m_data.values[8]; }
    const float &z2() const { return m_data.values[9]; }
    const float &z3() const { return m_data.values[10]; }
    const float &z4() const { return m_data.values[11]; }
    void setX1(const float &v);
    void setX2(const float &v);
    void setX3(const float &v);
    void setX4(const float &v);
    void setY1(const float &v);
    void setY2(const float &v);
    void setY3(const float &v);
    void setY4(const float &v);
    void setZ1(const float &v);
    void setZ2(const float &v);
    void setZ3(const float &v);
    void setZ4(const float &v);

  signals:
    void dataChanged();

  private:
    PODMatrix m_data;
};

class Instance : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

    Q_PROPERTY(qint64 offset READ offset CONSTANT)
    Q_PROPERTY(float minX READ minX WRITE setMinX NOTIFY dataChanged)
    Q_PROPERTY(float minY READ minY WRITE setMinY NOTIFY dataChanged)
    Q_PROPERTY(float minZ READ minZ WRITE setMinZ NOTIFY dataChanged)
    Q_PROPERTY(float maxX READ maxX WRITE setMaxX NOTIFY dataChanged)
    Q_PROPERTY(float maxY READ maxY WRITE setMaxY NOTIFY dataChanged)
    Q_PROPERTY(float maxZ READ maxZ WRITE setMaxZ NOTIFY dataChanged)
    Q_PROPERTY(quint32 unk1 READ unk1 CONSTANT)
    Q_PROPERTY(qint16 unk2 READ unk2 CONSTANT)

  public:
    explicit Instance(const PODInstance &data, QObject *parent);
    PODInstance pod() const { return m_data; }
    const qint64 &offset() const { return m_data.offset; }
    const float &minX() const { return m_data.min[0]; }
    const float &minY() const { return m_data.min[1]; }
    const float &minZ() const { return m_data.min[2]; }
    const float &maxX() const { return m_data.max[0]; }
    const float &maxY() const { return m_data.max[1]; }
    const float &maxZ() const { return m_data.max[2]; }
    const quint32 &unk1() const { return m_data.unk1; }
    const qint16 &unk2() const { return m_data.unk2; }
    void setMinX(const float &v);
    void setMinY(const float &v);
    void setMinZ(const float &v);
    void setMaxX(const float &v);
    void setMaxY(const float &v);
    void setMaxZ(const float &v);

  signals:
    void dataChanged();

  private:
    PODInstance m_data;
};

class Object : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

    Q_PROPERTY(qint64 offset READ offset CONSTANT)
    Q_PROPERTY(quint32 indexStart READ indexStart CONSTANT)
    Q_PROPERTY(quint32 indexEnd READ indexEnd CONSTANT)
    Q_PROPERTY(quint32 meshIndex READ meshIndex CONSTANT)
    Q_PROPERTY(quint8 isFlipped READ isFlipped CONSTANT)
    Q_PROPERTY(QString materialPath READ materialPath CONSTANT)
    Q_PROPERTY(quint32 lod READ lod CONSTANT)
    Q_PROPERTY(QList<Matrix *> matrices READ matrices CONSTANT)
    Q_PROPERTY(const QList<Instance *> instances READ instances CONSTANT)

  public:
    explicit Object(const PODObject &data, QObject *parent);
    PODObject pod();
    const qint64 &offset() const { return m_data.offset; }
    const quint32 &indexStart() const { return m_data.indexStart; }
    const quint32 &indexEnd() const { return m_data.indexEnd; }
    const quint32 &meshIndex() const { return m_data.meshIndex; }
    const quint8 &isFlipped() const { return m_data.isFlipped; }
    const QString &materialPath() const { return m_data.materialPath; }
    const quint32 &lod() const { return m_data.lod; }
    const QList<Matrix *> &matrices() const { return m_matrices; }
    const QList<Instance *> &instances() const { return m_instances; }

  private:
    PODObject m_data;
    QList<Matrix *> m_matrices;
    QList<Instance *> m_instances;
};

QString parse(const QByteArray &input, QList<PODObject> &objects);

QString inject(const PODObject &obj, QByteArray *output);

} // namespace bwm

#endif // BWM_H
