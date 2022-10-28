#include "bwm.h"

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QIODevice>

namespace bwm
{

Matrix::Matrix()
    : offset(0)
    , values{0}
{
}

Matrix::Matrix(QDataStream &stream)
    : offset(0)
    , values{0}
{
    offset = stream.device()->pos();
    stream >> values[0] >> values[1] >> values[2] >> values[3] >> values[4] >> values[5] >>
        values[6] >> values[7] >> values[8] >> values[9] >> values[10] >> values[11];
}

void parse(const QByteArray &input)
{
    if (input.first(4) != "BWM1") {
        qWarning() << "Bad magic:" << input.first(4);
        return;
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
        qWarning() << "Bad version info?:" << v1 << v2 << v3;
        return;
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
    QList<Matrix> matrices;
    for (quint32 i = 0; i < vmSize / Matrix::Size; ++i) {
        matrices.append(Matrix(stream));
    }
    qDebug() << "Read" << matrices.count() << "matrices.";

    quint32 vuSize;
    stream >> vuSize;
    auto vuOffset = stream.device()->pos();
    stream.skipRawData(vuSize);
    qDebug() << "Skipped" << vuSize << "bytes of unknown vertex data at" << Qt::hex << vuOffset;

    quint32 meshCount;
    stream >> meshCount;
    for (quint32 i = 0; i < meshCount; ++i) {
        auto mOffset = stream.device()->pos();
        quint32 md1, vco, vcs, vdo, vds, md2, vio, vic, vc;
        stream >> md1 >> vco >> vcs >> vdo >> vds >> md2 >> vio >> vic >> vc;
        stream.skipRawData(5);
        if (md1 != 2) {
            qWarning() << "Mesh with bad starting md1:" << i << mOffset << md1;
            return;
        }
        if (md2 != 0x12345678) {
            qWarning() << "Mesh with bad starting md2:" << i << mOffset << md2;
            return;
        }
    }
    qDebug() << "Read" << meshCount << "meshes.";

    quint32 objectCount;
    stream >> objectCount;
    for (quint32 i = 0; i < objectCount; ++i) {
        quint32 ms, me, mi, strLen, lod;
        quint8 isFlipped;
        stream >> ms >> me >> mi;
        stream >> isFlipped;
        char *str = nullptr;
        stream.readBytes(str, strLen);
        QString matPath = QString::fromUtf8(str, strLen);
        delete[] str;
        qDebug() << "Object" << matPath << matrices[ms].offset;
        stream >> lod;
    }
    qDebug() << "Read" << objectCount << "objects.";

    quint32 instanceCount;
    stream >> instanceCount;
    for (quint32 i = 0; i < instanceCount; ++i) {
        float minX, minY, minZ, maxX, maxY, maxZ;
        stream >> minX >> minY >> minZ >> maxX >> maxY >> maxZ;
        stream.skipRawData(4);
        qint16 unk;
        stream >> unk;
        if (unk != -1) {
            qWarning() << "Instance with bad unk:" << i << unk;
            return;
        }
    }
    qDebug() << "Read" << instanceCount << "instances.";

    // TODO: what are all theses lists and groups of indexes for, they all seem
    //       to be valid indexes for the list of instances/matrices

    quint32 idx1Count, idx2Count, idx3Count, idx4Count;
    stream >> idx1Count >> idx2Count >> idx3Count >> idx4Count;
    if (stream.atEnd()) {
        qWarning() << "EOF reach before idx1!";
        return;
    }
    stream.skipRawData(idx1Count * 4);
    if (stream.atEnd()) {
        qWarning() << "EOF reach before idx2!";
        return;
    }
    stream.skipRawData(idx2Count * 4);
    if (stream.atEnd()) {
        qWarning() << "EOF reach before idx3!";
        return;
    }
    stream.skipRawData(idx3Count * 4);
    if (stream.atEnd()) {
        qWarning() << "EOF reach before idx4!";
        return;
    }
    stream.skipRawData(idx4Count * 4);
    qDebug() << "Skipped" << idx1Count << "indexes.";
    qDebug() << "Skipped" << idx2Count << "indexes.";
    qDebug() << "Skipped" << idx3Count << "indexes.";
    qDebug() << "Skipped" << idx4Count << "indexes.";

    if (stream.atEnd()) {
        qWarning() << "EOF reach before g1!";
        return;
    }
    quint32 g1Count;
    stream >> g1Count;
    for (quint32 i = 0; i < g1Count; ++i) {
        stream.skipRawData(4);
        quint32 g1aCount;
        stream >> g1aCount;
        stream.skipRawData(g1aCount * 4);
    }
    qDebug() << "Skipped" << g1Count << "groups.";

    if (stream.atEnd()) {
        qWarning() << "EOF reach before g2!";
        return;
    }
    quint32 g2Count;
    stream >> g2Count;
    for (quint32 i = 0; i < g2Count; ++i) {
        quint32 g2aCount;
        stream >> g2aCount;
        stream.skipRawData(g2aCount * 4);
    }
    qDebug() << "Skipped" << g2Count << "groups.";

    if (stream.atEnd()) {
        qWarning() << "EOF reach before idx5!";
        return;
    }
    quint32 idx5Count;
    stream >> idx5Count;
    stream.skipRawData(idx5Count * 4);
    qDebug() << "Skipped" << idx5Count << "indexes.";
}

} // namespace bwm
