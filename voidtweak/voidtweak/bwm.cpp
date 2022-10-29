#include "bwm.h"

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QIODevice>

namespace bwm
{

QDebug operator<<(QDebug d, const PODMatrix &m)
{
    d.nospace() << "Matrix(offset=" << m.offset << ")";
    return d;
}

QDebug operator<<(QDebug d, const PODMesh &m)
{
    d.nospace() << "Mesh(offset=" << m.offset << ", vco=" << m.vco << ", vdo=" << m.vdo << ")";
    return d;
}

QDebug operator<<(QDebug d, const PODObject &o)
{
    d.nospace() << "Object(offset=" << o.offset << ", indexStart=" << o.indexStart
                << ", indexEnd=" << o.indexEnd << ", mesh=" << o.meshIndex << ", lod=" << o.lod
                << ", mat=" << o.materialPath << ")";
    return d;
}

QDebug operator<<(QDebug d, const PODInstance &i)
{
    d.nospace() << "Instance(offset=" << i.offset << ", min=" << i.min << ", max=" << i.max << ")";
    return d;
}

QString parse(const QByteArray &input, QList<PODObject> &objects)
{
    if (input.first(4) != "BWM1") {
        return u"Bad magic:"_qs.arg(input.first(4));
    }
    QDataStream stream(input);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.skipRawData(4);
    quint16 v1, v2;
    quint32 v3;
    stream >> v1;
    stream >> v2;
    stream >> v3;
    if (v1 != 21 || v2 != 1 || v3 != 2) {
        return u"Bad version info: %1.%2.%3"_qs.arg(v1).arg(v2).arg(v3);
    }

    quint32 vcSize;
    stream >> vcSize;
    auto vcOffset = stream.device()->pos();
    stream.skipRawData(vcSize);
    qDebug() << "Skipped" << vcSize << "bytes of vertex coords at" << Qt::hex << vcOffset;

    quint32 vdSize;
    stream >> vdSize;
    auto vdOffset = stream.device()->pos();
    stream.skipRawData(vdSize);
    qDebug() << "Skipped" << vdSize << "bytes of vertex data at" << Qt::hex << vdOffset;

    quint32 viSize;
    stream >> viSize;
    auto viOffset = stream.device()->pos();
    stream.skipRawData(viSize);
    qDebug() << "Skipped" << viSize << "bytes of vertex indexes at" << Qt::hex << viOffset;

    quint32 vmSize;
    stream >> vmSize;
    QList<PODMatrix> matrices;
    for (quint32 i = 0; i < vmSize / (12 * 4); ++i) {
        PODMatrix m;
        m.offset = stream.device()->pos();
        stream >> m.values[0] >> m.values[1] >> m.values[2] >> m.values[3] >> m.values[4] >>
            m.values[5] >> m.values[6] >> m.values[7] >> m.values[8] >> m.values[9] >>
            m.values[10] >> m.values[11];
        matrices.append(m);
    }
    qDebug() << "Read" << matrices.count() << "matrices.";

    quint32 vuSize;
    stream >> vuSize;
    auto vuOffset = stream.device()->pos();
    stream.skipRawData(vuSize);
    qDebug() << "Skipped" << vuSize << "bytes of unknown vertex data at" << Qt::hex << vuOffset;

    quint32 meshCount;
    stream >> meshCount;
    QList<PODMesh> meshes;
    for (quint32 i = 0; i < meshCount; ++i) {
        PODMesh m;
        m.offset = stream.device()->pos();
        stream >> m.unk1 >> m.vco >> m.vcs >> m.vdo >> m.vds >> m.unk2 >> m.vio >> m.vic >> m.vc;
        stream.readRawData(m.unk3, 5);
        if (m.unk1 != 2) {
            return u"Mesh %1 has bad unk1 %2"_qs.arg(i).arg(m.unk1);
        }
        if (m.unk2 != 0x12345678) {
            return u"Mesh %1 has bad unk2 %2"_qs.arg(i).arg(m.unk2);
        }
        meshes.append(m);
    }
    qDebug() << "Read" << meshes.count() << "meshes.";

    quint32 objectCount;
    stream >> objectCount;
    for (quint32 i = 0; i < objectCount; ++i) {
        PODObject o;
        o.offset = stream.device()->pos();
        stream >> o.indexStart >> o.indexEnd >> o.meshIndex >> o.isFlipped;
        quint32 strLen;
        char *str = nullptr;
        stream.readBytes(str, strLen);
        o.materialPath = QString::fromUtf8(str, strLen);
        delete[] str;
        stream >> o.lod;
        objects.append(o);
    }
    qDebug() << "Read" << objects.count() << "objects.";

    quint32 instanceCount;
    stream >> instanceCount;
    QList<PODInstance> instances;
    for (quint32 i = 0; i < instanceCount; ++i) {
        PODInstance ins;
        ins.offset = stream.device()->pos();
        stream >> ins.min[0] >> ins.min[1] >> ins.min[2] >> ins.max[0] >> ins.max[1] >>
            ins.max[2] >> ins.unk1 >> ins.unk2;
        if (ins.unk2 != -1) {
            return u"Instance %1 has bad unk2 %2"_qs.arg(i).arg(ins.unk2);
        }
        instances.append(ins);
    }
    qDebug() << "Read" << instances.count() << "instances.";

    // TODO: what are all theses lists and groups of indexes for, they all seem
    //       to be valid indexes for the list of instances/matrices
    quint32 idx1Count, idx2Count, idx3Count, idx4Count;
    stream >> idx1Count >> idx2Count >> idx3Count >> idx4Count;
    Group idx1, idx2, idx3, idx4;
    quint32 v;
    for (quint32 i = 0; i < idx1Count; ++i) {
        stream >> v;
        idx1.append(v);
    }
    qDebug() << "Read" << idx1.count() << "idx1.";
    for (quint32 i = 0; i < idx2Count; ++i) {
        stream >> v;
        idx2.append(v);
    }
    qDebug() << "Read" << idx2.count() << "idx2.";
    for (quint32 i = 0; i < idx3Count; ++i) {
        stream >> v;
        idx3.append(v);
    }
    qDebug() << "Read" << idx3.count() << "idx3.";
    for (quint32 i = 0; i < idx4Count; ++i) {
        stream >> v;
        idx4.append(v);
    }
    qDebug() << "Read" << idx4.count() << "idx4.";

    if (stream.atEnd()) {
        return u"EOF reached before g1!"_qs;
    }
    quint32 g1Count;
    stream >> g1Count;
    QList<LabeledGroup> g1;
    for (quint32 i = 0; i < g1Count; ++i) {
        LabeledGroup g;
        stream >> g.first;
        quint32 gCount;
        stream >> gCount;
        for (quint32 gi = 0; gi < gCount; ++gi) {
            stream >> v;
            g.second.append(v);
        }
        g1.append(g);
    }
    qDebug() << "Read" << g1.count() << "labeled groups.";

    if (stream.atEnd()) {
        return u"EOF reached before g2!"_qs;
    }
    quint32 g2Count;
    stream >> g2Count;
    QList<Group> g2;
    for (quint32 i = 0; i < g2Count; ++i) {
        Group g;
        quint32 gCount;
        stream >> gCount;
        for (quint32 gi = 0; gi < gCount; ++gi) {
            stream >> v;
            g.append(v);
        }
        g2.append(g);
    }
    qDebug() << "Read" << g2.count() << "groups.";

    if (stream.atEnd()) {
        return u"EOF reached before idx5!"_qs;
    }
    quint32 idx5Count;
    stream >> idx5Count;
    Group idx5;
    for (quint32 i = 0; i < idx5Count; ++i) {
        stream >> v;
        idx5.append(v);
    }
    qDebug() << "Read" << idx5.count() << "indexes.";

    for (auto &o : objects) {
        for (auto i = o.indexStart; i < o.indexEnd; ++i) {
            o.matrices.append(matrices[i]);
            o.instances.append(instances[i]);
        }
    }
    return {};
}

} // namespace bwm
