// /*
// Copyright (C) 2024 Gregory EUSTACHE, cen1

// QuazipTextCodec is a wrapper/abstraction around QTextCodec

// This file is part of QuaZip.

// QuaZip is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.

// QuaZip is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with QuaZip.  If not, see <http://www.gnu.org/licenses/>.

// See COPYING file for the full LGPL text.
// */

// #include "quazip_textcodec.h"

// #ifndef QUAZIP_CAN_USE_QTEXTCODEC

// #include <QHash>

// static QHash<QStringConverter::Encoding,QuazipTextCodec*> *static_hash_quazip_codecs = nullptr;

// class QuazipTextCodecCleanup
// {
// public:
//     explicit QuazipTextCodecCleanup()
//     {
//     }
//     ~QuazipTextCodecCleanup()
//     {
//         if (static_hash_quazip_codecs)
//         {
//             QList<QuazipTextCodec*>list_quazip_codecs = static_hash_quazip_codecs->values();
//             qDeleteAll(list_quazip_codecs.begin(),list_quazip_codecs.end());
//             static_hash_quazip_codecs->clear();
//             delete static_hash_quazip_codecs;
//             static_hash_quazip_codecs = nullptr;
//         }
//     }
// };

// Q_GLOBAL_STATIC(QuazipTextCodecCleanup, createQuazipTextCodecCleanup)

// QuazipTextCodec::QuazipTextCodec()
// {
// }

// void QuazipTextCodec::setup()
// {
//     if (static_hash_quazip_codecs) return;
//       (void)createQuazipTextCodecCleanup();

//     static_hash_quazip_codecs = new QHash<QStringConverter::Encoding,QuazipTextCodec*>;
// }

// QuazipTextCodec *QuazipTextCodec::codecForName(const QByteArray &name)
// {
//     QuazipTextCodec::setup();
//     QStringConverter::Encoding  encoding = QStringConverter::Utf8;

//     std::optional<QStringConverter::Encoding> opt_encoding = QStringConverter::encodingForName(name);
//     if (opt_encoding != std::nullopt)
//     {
//         encoding = opt_encoding.value();
//     }
//     if (static_hash_quazip_codecs->contains(encoding))
//     {
//         return static_hash_quazip_codecs->value(encoding);
//     }

//     QuazipTextCodec *codec = new QuazipTextCodec();

//     codec->mEncoding = encoding;
//     static_hash_quazip_codecs->insert(encoding,codec);
//     return codec;
// }

// QuazipTextCodec *QuazipTextCodec::codecForLocale()
// {
//     QuazipTextCodec::setup();
//     return QuazipTextCodec::codecForName("System");
// }

// QByteArray QuazipTextCodec::fromUnicode(const QString &str) const
// {
//     auto from = QStringEncoder(mEncoding);
//     return from(str);
// }

// QString QuazipTextCodec::toUnicode(const QByteArray &a) const
// {
//     auto to = QStringDecoder(mEncoding);
//     return to(a);
// }

// #endif







// #include "quaziptextcodec.h"

#ifndef QUAZIP_CAN_USE_QTEXTCODEC
// 只有在 QT6 环境下才需要实现 QuazipTextCodec

#include <QHash>
#include <QMutex>
#include <QStringConverter>

// 静态变量用于管理编解码器和区域设置
static QHash<QByteArray, QuazipTextCodec*> *supportedCodecs = nullptr;
static QuazipTextCodec *localeCodec = nullptr;
static QMutex mutex;

QuazipTextCodec::QuazipTextCodec()
  : mEncoding(QStringConverter::Encoding::Utf8) // 默认为 UTF-8
{
}

QByteArray QuazipTextCodec::fromUnicode(const QString &str) const
{
    QStringEncoder encoder(mEncoding);
    if (encoder.isValid()) {
        return encoder.encode(str);
    }
    return QByteArray(); // 如果编码无效，返回空字节数组
}

QString QuazipTextCodec::toUnicode(const QByteArray &a) const
{
    QStringDecoder decoder(mEncoding);
    if (decoder.isValid()) {
        return decoder.decode(a);
    }
    return QString(); // 如果解码无效，返回空字符串
}

void QuazipTextCodec::setup()
{
    if (supportedCodecs != nullptr) return;

    supportedCodecs = new QHash<QByteArray, QuazipTextCodec*>();

           // 创建并添加支持的编码
    auto addCodec = [](const QByteArray &name, QStringConverter::Encoding encoding) {
        QuazipTextCodec *codec = new QuazipTextCodec();
        codec->mEncoding = encoding;
        supportedCodecs->insert(name, codec);
        return codec;
    };

    addCodec("UTF-8", QStringConverter::Encoding::Utf8);
    addCodec("UTF-16", QStringConverter::Encoding::Utf16);
    addCodec("UTF-16BE", QStringConverter::Encoding::Utf16BE);
    addCodec("UTF-16LE", QStringConverter::Encoding::Utf16LE);
    addCodec("ISO-8859-1", QStringConverter::Encoding::Latin1);
    addCodec("Latin1", QStringConverter::Encoding::Latin1);

           // 将区域编解码器默认设置为 UTF-8
    localeCodec = supportedCodecs->value("UTF-8");
}

QuazipTextCodec *QuazipTextCodec::codecForName(const QByteArray &name)
{
    QMutexLocker locker(&mutex);
    if (supportedCodecs == nullptr) {
        setup();
    }
    return supportedCodecs->value(name, nullptr);
}

QuazipTextCodec *QuazipTextCodec::codecForLocale()
{
    QMutexLocker locker(&mutex);
    if (localeCodec == nullptr) {
        setup();
    }
    return localeCodec;
}

#endif // QUAZIP_CAN_USE_QTEXTCODEC
