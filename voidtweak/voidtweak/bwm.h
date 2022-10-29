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
#include <QString>
#include <QtGlobal>

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

QString parse(const QByteArray &input, QList<PODObject> &objects);

} // namespace bwm

#endif // BWM_H
