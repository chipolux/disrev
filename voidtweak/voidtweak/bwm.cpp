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
                << ", mat=" << o.materialPath.split('/').last() << ")";
    return d;
}

QString matName(const PODObject &obj) { return obj.materialPath.split('/').last(); }

QDebug operator<<(QDebug d, const PODInstance &i)
{
    d.nospace() << "Instance(offset=" << i.offset;
    d.nospace() << ", min=(" << i.min[0] << ", " << i.min[1] << ", " << i.min[2] << ")";
    d.nospace() << ", max=(" << i.max[0] << ", " << i.max[1] << ", " << i.max[2] << "))";
    return d;
}

QString minVert(const PODInstance &i)
{
    return u"%1,%2,%3"_qs.arg(i.min[0]).arg(i.min[1]).arg(i.min[2]);
}

QString maxVert(const PODInstance &i)
{
    return u"%1,%2,%3"_qs.arg(i.max[0]).arg(i.max[1]).arg(i.max[2]);
}

Matrix::Matrix(const PODMatrix &data, QObject *parent)
    : QObject(parent)
    , m_data(data)
{
}
// clang-format off
void Matrix::setX1(const float &v) {if (v != m_data.values[ 0]) {m_data.values[ 0] = v; emit dataChanged();}}
void Matrix::setX2(const float &v) {if (v != m_data.values[ 1]) {m_data.values[ 1] = v; emit dataChanged();}}
void Matrix::setX3(const float &v) {if (v != m_data.values[ 2]) {m_data.values[ 2] = v; emit dataChanged();}}
void Matrix::setX4(const float &v) {if (v != m_data.values[ 3]) {m_data.values[ 3] = v; emit dataChanged();}}
void Matrix::setY1(const float &v) {if (v != m_data.values[ 4]) {m_data.values[ 4] = v; emit dataChanged();}}
void Matrix::setY2(const float &v) {if (v != m_data.values[ 5]) {m_data.values[ 5] = v; emit dataChanged();}}
void Matrix::setY3(const float &v) {if (v != m_data.values[ 6]) {m_data.values[ 6] = v; emit dataChanged();}}
void Matrix::setY4(const float &v) {if (v != m_data.values[ 7]) {m_data.values[ 7] = v; emit dataChanged();}}
void Matrix::setZ1(const float &v) {if (v != m_data.values[ 8]) {m_data.values[ 8] = v; emit dataChanged();}}
void Matrix::setZ2(const float &v) {if (v != m_data.values[ 9]) {m_data.values[ 9] = v; emit dataChanged();}}
void Matrix::setZ3(const float &v) {if (v != m_data.values[10]) {m_data.values[10] = v; emit dataChanged();}}
void Matrix::setZ4(const float &v) {if (v != m_data.values[11]) {m_data.values[11] = v; emit dataChanged();}}
// clang-format on

Instance::Instance(const PODInstance &data, QObject *parent)
    : QObject(parent)
    , m_data(data)
{
}
// clang-format off
void Instance::setMinX(const float &v) {if (v != m_data.min[0]) {m_data.min[0] = v; emit dataChanged();}}
void Instance::setMinY(const float &v) {if (v != m_data.min[1]) {m_data.min[1] = v; emit dataChanged();}}
void Instance::setMinZ(const float &v) {if (v != m_data.min[2]) {m_data.min[2] = v; emit dataChanged();}}
void Instance::setMaxX(const float &v) {if (v != m_data.max[0]) {m_data.max[0] = v; emit dataChanged();}}
void Instance::setMaxY(const float &v) {if (v != m_data.max[1]) {m_data.max[1] = v; emit dataChanged();}}
void Instance::setMaxZ(const float &v) {if (v != m_data.max[2]) {m_data.max[2] = v; emit dataChanged();}}
// clang-format on

Object::Object(const PODObject &data, QObject *parent)
    : QObject(parent)
    , m_data(data)
{
    for (const auto &m : m_data.matrices) {
        m_matrices.append(new Matrix(m, this));
    }
    for (const auto &i : m_data.instances) {
        m_instances.append(new Instance(i, this));
    }
}

PODObject Object::pod()
{
    m_data.matrices.clear();
    for (const auto m : m_matrices) {
        m_data.matrices.append(m->pod());
    }
    m_data.instances.clear();
    for (const auto i : m_instances) {
        m_data.instances.append(i->pod());
    }
    return m_data;
}

void setScale(const float &scale, PODMatrix *matrix)
{
    matrix->values[0] = scale;
    matrix->values[1] = scale;
    matrix->values[2] = scale;
    matrix->values[4] = scale;
    matrix->values[5] = scale;
    matrix->values[6] = scale;
    matrix->values[8] = scale;
    matrix->values[9] = scale;
    matrix->values[10] = scale;
}

QString parse(const QByteArray &input, QList<PODObject> &objects)
{
    if (input.first(4) != "BWM1") {
        return u"Bad magic:"_qs.arg(input.first(4));
    }

    // GLTF components
    QVariantList gBuffers;
    QVariantList gBufferViews;
    QVariantList gAccessors;
    QVariantList gMeshes;
    QVariantList gNodes;

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
    // 3 float32 (x, y, z) coords per vertex
    qInfo() << "Skipped" << vcSize << "bytes of vertex coords at" << Qt::hex << vcOffset;
    gBufferViews.append(QVariantMap{
        {u"buffer"_qs, 0},
        {u"byteLength"_qs, vcSize},
        {u"byteOffset"_qs, vcOffset},
        {u"target"_qs, 34962},
        {u"name"_qs, u"vertexCoords"_qs},
    });

    quint32 vdSize;
    stream >> vdSize;
    auto vdOffset = stream.device()->pos();
    stream.skipRawData(vdSize);
    // 20 bytes of data per vertex (can some vertexes not have any data?)
    qDebug() << "Skipped" << vdSize << "bytes of vertex data at" << Qt::hex << vdOffset;

    quint32 viSize;
    stream >> viSize;
    auto viOffset = stream.device()->pos();
    stream.skipRawData(viSize);
    // uint16 per index
    qInfo() << "Skipped" << viSize << "bytes of vertex indexes at" << Qt::hex << viOffset;
    gBufferViews.append(QVariantMap{
        {u"buffer"_qs, 0},
        {u"byteLength"_qs, viSize},
        {u"byteOffset"_qs, viOffset},
        {u"target"_qs, 34963},
        {u"name"_qs, u"vertexIndices"_qs},
    });

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

    // there is a section size value here, but no BWM file in Dishonored 2 has
    // any data in this section.
    quint32 vuSize;
    stream >> vuSize;
    auto vuOffset = stream.device()->pos();
    stream.skipRawData(vuSize);
    if (vuSize) {
        qWarning() << "Skipped" << vuSize << "bytes of unknown vertex data at" << Qt::hex
                   << vuOffset;
    }

    quint32 meshCount;
    stream >> meshCount;
    QList<PODMesh> meshes;
    for (quint32 i = 0; i < meshCount; ++i) {
        PODMesh m;
        m.offset = stream.device()->pos();
        stream >> m.unk1 >> m.vco >> m.vcs >> m.vdo >> m.vds >> m.unk2 >> m.vio >> m.vic >> m.vc >>
            m.unk3 >> m.unk4;
        if (m.vds != 20) {
            return u"Mesh %1 has bad vertex data stride %2"_qs.arg(i).arg(m.vds);
        }
        if (m.unk1 != 2) {
            return u"Mesh %1 has bad unk1 %2"_qs.arg(i).arg(m.unk1);
        }
        if (m.unk2 != 0x12345678) {
            return u"Mesh %1 has bad unk2 %2"_qs.arg(i).arg(m.unk2);
        }
        if (m.unk3 != 3) {
            return u"Mesh %1 has bad unk3 %2"_qs.arg(i).arg(m.unk3);
        }
        if (m.unk4 != 1) {
            return u"Mesh %1 has bad unk4 %2"_qs.arg(i).arg(m.unk4);
        }
        meshes.append(m);
        const auto posIdx = gAccessors.count();
        gAccessors.append(QVariantMap{
            {u"bufferView"_qs, 0}, // vertex coords list is always the first buffer view
            {u"componentType"_qs, 5126},
            {u"type"_qs, u"VEC3"_qs},
            {u"byteOffset"_qs, m.vco},
            {u"count"_qs, m.vc},
        });
        const auto indicesIdx = gAccessors.count();
        gAccessors.append(QVariantMap{
            {u"bufferView"_qs, 1}, // indices list is always the second buffer view
            {u"componentType"_qs, 5123},
            {u"type"_qs, u"SCALAR"_qs},
            {u"byteOffset"_qs, m.vio},
            {u"count"_qs, m.vic},
        });
        gMeshes.append(QVariantMap{
            {u"name"_qs, u"M%1"_qs.arg(i)},
            {u"primitives"_qs, QVariantList{QVariantMap{
                                   {u"attributes"_qs, QVariantMap{{u"POSITION"_qs, posIdx}}},
                                   {u"indices"_qs, indicesIdx},
                               }}},
        });
    }
    qDebug() << "Read" << meshes.count() << "meshes.";

    quint32 objectCount;
    stream >> objectCount;
    QMap<quint32, quint32> idxToObj;
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
        for (quint32 ii = o.indexStart; ii < o.indexEnd; ++ii) {
            idxToObj[ii] = i;
        }
    }
    qDebug() << "Read" << objects.count() << "objects.";

    quint32 instanceCount;
    stream >> instanceCount;
    QList<PODInstance> instances;
    QList<QString> instanceNames;
    for (quint32 i = 0; i < instanceCount; ++i) {
        PODInstance ins;
        ins.offset = stream.device()->pos();
        stream >> ins.min[0] >> ins.min[1] >> ins.min[2] >> ins.max[0] >> ins.max[1] >>
            ins.max[2] >> ins.unk1 >> ins.unk2;
        if (ins.unk2 != -1) {
            return u"Instance %1 has bad unk2 %2"_qs.arg(i).arg(ins.unk2);
        }
        instances.append(ins);
        instanceNames.append(u"I%1"_qs.arg(i));
    }
    qDebug() << "Read" << instances.count() << "instances.";

    // TODO: what are all theses lists and groups of indexes for, they all seem
    //       to be valid indexes for the list of instances/matrices
    QSet<quint32> objs;
    quint32 idx1Count, idx2Count, idx3Count, idx4Count;
    stream >> idx1Count >> idx2Count >> idx3Count >> idx4Count;
    Group idx1, idx2, idx3;
    quint32 v;

    // NOTE: appears to be material.options.rendering/darkVisionLayer = 1
    for (quint32 i = 0; i < idx1Count; ++i) {
        stream >> v;
        idx1.append(v);
        instanceNames[v] = u"%1 dv1"_qs.arg(instanceNames[v]);
    }

    // NOTE: appears to be material.options.rendering/darkVisionLayer = 2
    for (quint32 i = 0; i < idx2Count; ++i) {
        stream >> v;
        idx2.append(v);
        instanceNames[v] = u"%1 dv2"_qs.arg(instanceNames[v]);
    }

    // NOTE: appears to be material.options.rendering/darkVisionLayer = 3
    for (quint32 i = 0; i < idx3Count; ++i) {
        stream >> v;
        idx3.append(v);
        instanceNames[v] = u"%1 dv3"_qs.arg(instanceNames[v]);
    }

    // NOTE: never used, but i bet it is material.options.rendering/darkVisionLayer = 4
    stream.skipRawData(idx4Count * 4);
    if (idx4Count) {
        qWarning() << "Skipped" << idx4Count << "indexes in idx4!";
    }

    if (stream.atEnd()) {
        return u"EOF reached before g1!"_qs;
    }

    // NOTE: appears to be special distant objects that need to override normal
    //       LOD/culling behaviour, like the ship you can see in the port far
    //       below at the start of dunwall tower streets.
    quint32 g1Count;
    stream >> g1Count;
    QList<LabeledGroup> g1;
    for (quint32 i = 0; i < g1Count; ++i) {
        LabeledGroup g;
        stream >> g.first;
        instanceNames[g.first] = u"%1 L"_qs.arg(instanceNames[g.first]);
        quint32 gCount;
        stream >> gCount;
        const PODObject lo = objects[idxToObj[g.first]];
        for (quint32 gi = 0; gi < gCount; ++gi) {
            stream >> v;
            g.second.append(v);
            objs.insert(idxToObj[v]);
            instanceNames[v] = u"%1 l%2"_qs.arg(instanceNames[v]).arg(g.first);
        }
        g1.append(g);
        objs.clear();
    }

    if (stream.atEnd()) {
        return u"EOF reached before g2!"_qs;
    }
    // there is always at least 1 of these groups, often far more, they can be
    // empty, but there are never any duplicate indexes
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
            objs.insert(idxToObj[v]);
            instanceNames[v] = u"%1 g%2"_qs.arg(instanceNames[v]).arg(i);
        }
        g2.append(g);
        objs.clear();
    }

    if (stream.atEnd()) {
        return u"EOF reached before idx5!"_qs;
    }

    // TODO: used rarely, not much in common with members, all have m_PhysicsMaterial
    //       set to env.<something> (metal, stone, dirt...)
    quint32 idx5Count;
    stream >> idx5Count;
    Group idx5;
    for (quint32 i = 0; i < idx5Count; ++i) {
        stream >> v;
        idx5.append(v);
        instanceNames[v] = u"%1 idx5"_qs.arg(instanceNames[v]);
    }

    for (auto &o : objects) {
        for (auto i = o.indexStart; i < o.indexEnd; ++i) {
            o.matrices.append(matrices[i]);
            o.instances.append(instances[i]);
        }
    }

    // write out gltf version of level
    {
        // pack these at the end so we can see group membership
        for (quint32 i = 0; i < instanceCount; ++i) {
            // ignoring LOD objects for now
            if (!objects[idxToObj[i]].lod) {
                const auto matrix = matrices[i].values;
                gNodes.append(QVariantMap{
                    {u"name"_qs, instanceNames[i]},
                    {u"mesh"_qs, objects[idxToObj[i]].meshIndex},
                    // glTF wants column-major row order, we have row-major, we also swap
                    // the Y and Z rows to get Z+ up instead of Y- up for blender
                    // clang-format off
                {u"matrix"_qs, QVariantList{
                     matrix[ 0], matrix[ 8], matrix[ 4], 0.0f,
                     matrix[ 1], matrix[ 9], matrix[ 5], 0.0f,
                     matrix[ 2], matrix[10], matrix[ 6], 0.0f,
                     matrix[ 3], matrix[11], matrix[ 7], 1.0f,
                }},
                    // clang-format on
                });
            }
        }

        QFile fBuff(u"D:\\Projects\\disrev\\level.bwm"_qs);
        if (fBuff.open(QFile::WriteOnly)) {
            fBuff.write(input);
            fBuff.close();
            gBuffers.append(QVariantMap{
                {u"byteLength"_qs, input.size()},
                {u"uri"_qs, u"level.bwm"_qs},
            });
        } else {
            qWarning() << "Failed to open:" << fBuff.fileName();
        }

        QVariantMap gltf{
            {u"asset"_qs, QVariantMap{{u"version"_qs, u"2.0"_qs}}},
            {u"buffers"_qs, gBuffers},
            {u"bufferViews"_qs, gBufferViews},
            {u"accessors"_qs, gAccessors},
            {u"meshes"_qs, gMeshes},
            {u"nodes"_qs, gNodes},
        };
        QFile fGltf(u"D:\\Projects\\disrev\\level.gltf"_qs);
        if (fGltf.open(QFile::WriteOnly)) {
            fGltf.write(QJsonDocument::fromVariant(gltf).toJson());
            fGltf.close();
        } else {
            qWarning() << "Failed to open:" << fGltf.fileName();
        }
    }

    return {};
}

QString inject(const PODObject &obj, QByteArray *output) { return inject(QList({obj}), output); }

QString inject(const QList<PODObject> &objects, QByteArray *output)
{
    if (output->first(4) != "BWM1") {
        return u"Bad magic:"_qs.arg(output->first(4));
    }
    QDataStream stream(output, QIODevice::ReadWrite);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    for (const auto &obj : objects) {
        for (const auto &m : obj.matrices) {
            stream.device()->seek(m.offset);
            stream << m.values[0] << m.values[1] << m.values[2] << m.values[3] << m.values[4]
                   << m.values[5] << m.values[6] << m.values[7] << m.values[8] << m.values[9]
                   << m.values[10] << m.values[11];
        }
        for (const auto &i : obj.instances) {
            stream.device()->seek(i.offset);
            stream << i.min[0] << i.min[1] << i.min[2] << i.max[0] << i.max[1] << i.max[2] << i.unk1
                   << i.unk2;
        }
    }
    return {};
}

} // namespace bwm
