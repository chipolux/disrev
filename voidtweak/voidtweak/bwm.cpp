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
    qInfo() << "Read" << matrices.count() << "matrices.";

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
    qInfo() << "Read" << meshCount << "meshes.";

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
        qInfo() << "Object" << matPath << matrices[ms].offset;
        stream >> lod;
    }
    qInfo() << "Read" << objectCount << "objects.";

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
    qInfo() << "Read" << instanceCount << "instances.";

    // mystery junk?
    {
        quint32 c1, c2, c3;
        stream >> c1;
        stream.skipRawData(4);
        stream >> c2;
        stream.skipRawData(4);
        stream.skipRawData(4 * c1);
        qDebug() << "Skipped" << c1 << "uint32.";
        stream.skipRawData(4 * c2);
        qDebug() << "Skipped" << c2 << "uint32.";
        stream >> c3;
        for (quint32 i = 0; i < c3; ++i) {
            stream.skipRawData(4);
            quint32 c4;
            stream >> c4;
            stream.skipRawData(4 * c4);
        }
        qDebug() << "Skipped" << c3 << "mystery entries.";
    }

    quint32 groupCount;
    stream >> groupCount;
    for (quint32 i = 0; i < groupCount; ++i) {
        quint32 indexCount;
        stream >> indexCount;
        stream.skipRawData(4 * indexCount);
    }
    qInfo() << "Read" << groupCount << "groups.";
}

} // namespace bwm
