#include "zutils.h"

#include <QDebug>
#include <QtZlib/zlib.h>

#define ZLIB_CHUNK 16384

using namespace zutils;

bool zutils::deflt(QByteArray &input, QByteArray &output)
{
    int ret;
    z_Bytef buffer[ZLIB_CHUNK];
    z_stream stream;
    output.clear();

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    ret = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 10, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        qWarning() << "Failed to initialize zlib deflate:" << ret;
        return false;
    }

    stream.next_in = reinterpret_cast<z_Bytef *>(input.data());
    stream.avail_in = input.size();
    do {
        stream.next_out = buffer;
        stream.avail_out = ZLIB_CHUNK;
        ret = deflate(&stream, Z_FINISH);
        if (stream.avail_out < ZLIB_CHUNK) {
            output.append(reinterpret_cast<char *>(buffer), ZLIB_CHUNK - stream.avail_out);
        }
    } while (ret == Z_OK || ret == Z_BUF_ERROR);
    deflateEnd(&stream);

    if (ret != Z_STREAM_END) {
        qWarning() << "Failed to deflate:" << ret;
        output.resize(0);
    }
    return ret == Z_STREAM_END;
}

bool zutils::inflt(QByteArray &input, QByteArray &output)
{
    int ret;
    z_Bytef buffer[ZLIB_CHUNK];
    z_stream stream;
    output.clear();

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    ret = inflateInit2(&stream, 10);
    if (ret != Z_OK) {
        qWarning() << "Failed to initialize zlib inflate:" << ret;
        return false;
    }

    stream.next_in = reinterpret_cast<z_Bytef *>(input.data());
    stream.avail_in = input.size();
    do {
        stream.next_out = buffer;
        stream.avail_out = ZLIB_CHUNK;
        ret = inflate(&stream, Z_FINISH);
        if (stream.avail_out < ZLIB_CHUNK) {
            output.append(reinterpret_cast<char *>(buffer), ZLIB_CHUNK - stream.avail_out);
        }
    } while (ret == Z_OK || ret == Z_BUF_ERROR);
    inflateEnd(&stream);

    if (ret != Z_STREAM_END) {
        qWarning() << "Failed to inflate:" << ret << stream.msg;
        output.resize(0);
    }
    return ret == Z_STREAM_END;
}
