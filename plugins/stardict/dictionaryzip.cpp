/******************************************************************************
 * This file is part of the Mula project
 * Copyright (c) 2011 Laszlo Papp <lpapp@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "dictionaryzip.h"

#include "dictionarycache.h"

#include <QtCore/QtGlobal>

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>

#include <zlib.h>

#ifdef Q_OS_WIN32
#include <io.h>
#endif

#include <sys/stat.h>

using namespace MulaPluginStarDict;

#define USE_CACHE 1

#define BUFFERSIZE 10240

/*
 * Output buffer must be greater than or
 * equal to 110% of input buffer size, plus
 * 12 bytes. 
*/
#define OUT_BUFFER_SIZE 0xffffL

#define IN_BUFFER_SIZE ((unsigned long)((double)(OUT_BUFFER_SIZE - 12) * 0.89))

/* For gzip-compatible header, as defined in RFC 1952 */

/* Magic for GZIP (rfc1952)                */
#define GZ_MAGIC1     0x1f	/* First magic byte                        */
#define GZ_MAGIC2     0x8b	/* Second magic byte                       */

/* FLaGs (bitmapped), from rfc1952         */
enum Flags {
    GZ_FTEXT      = 0x01,	/* Set for ASCII text                      */
    GZ_FHCRC      = 0x02,	/* Header CRC16                            */
    GZ_FEXTRA     = 0x04,	/* Optional field (random access index)    */
    GZ_FNAME      = 0x08,	/* Original name                           */
    GZ_COMMENT    = 0x10,	/* Zero-terminated, human-readable comment */
};

enum ExtraFlags {
    GZ_MAX        =    2,	/* Maximum compression                     */
    GZ_FAST       =    4,	/* Fasted compression                      */
};

/* These are from rfc1952                  */
enum SystemType {
    GZ_OS_FAT        = 0,	    /* FAT filesystem (MS-DOS, OS/2, NT/Win32) */
    GZ_OS_AMIGA      = 1,	    /* Amiga                                   */
    GZ_OS_VMS        = 2,	    /* VMS (or OpenVMS)                        */
    GZ_OS_UNIX       = 3,       /* Unix                                    */
    GZ_OS_VMCMS      = 4,       /* VM/CMS                                  */
    GZ_OS_ATARI      = 5,       /* Atari TOS                               */
    GZ_OS_HPFS       = 6,       /* HPFS filesystem (OS/2, NT)              */
    GZ_OS_MAC        = 7,       /* Macintosh                               */
    GZ_OS_Z          = 8,       /* Z-System                                */
    GZ_OS_CPM        = 9,       /* CP/M                                    */
    GZ_OS_TOPS20     = 10,      /* TOPS-20                                 */
    GZ_OS_NTFS       = 11,      /* NTFS filesystem (NT)                    */
    GZ_OS_QDOS       = 12,      /* QDOS                                    */
    GZ_OS_ACORN      = 13,      /* Acorn RISCOS                            */
    GZ_OS_UNKNOWN    = 255,     /* unknown                                 */
};

#define GZ_RND_S1       'R'	    /* First magic for random access format    */
#define GZ_RND_S2       'A'	    /* Second magic for random access format   */

// TODO: gz_header_s usage from the zlib.h include
enum Fields {
    GZ_ID1           = 0,	/* GZ_MAGIC1                               */
    GZ_ID2           = 1,	/* GZ_MAGIC2                               */
    GZ_CM            = 2,	/* Compression Method (Z_DEFALTED)         */
    GZ_FLG	         = 3,	/* FLaGs (see above)                       */
    GZ_MTIME         = 4,	/* Modification TIME                       */
    GZ_XFL           = 8,	/* eXtra FLags (GZ_MAX or GZ_FAST)         */
    GZ_OS            = 9,	/* Operating System                        */
    GZ_XLEN          = 10,	/* eXtra LENgth (16bit)                    */
    GZ_FEXTRA_START  = 12,	/* Start of extra fields                   */
    GZ_SI1           = 12,	/* Subfield ID1                            */
    GZ_SI2           = 13,  /* Subfield ID2                            */
    GZ_SUBLEN        = 14,	/* Subfield length (16bit)                 */
    GZ_VERSION       = 16,  /* Version for subfield format             */
    GZ_CHUNKLEN      = 18,	/* Chunk length (16bit)                    */
    GZ_CHUNKCNT      = 20,	/* Number of chunks (16bit)                */
    GZ_RNDDATA       = 22,	/* Random access data (16bit)              */
};

enum DictionaryFormat {
    DICTIONARY_UNKNOWN    = 0,
    DICTIONARY_TEXT       = 1,
    DICTIONARY_GZIP       = 2,
    DICTIONARY_DZIP       = 3,
};

class DictionaryZip::Private
{
    public:
        Private()
            : start(0)
            , end(0)
            , size(0)
            , type(DICTIONARY_UNKNOWN)
            , initialized(0)
            , headerLength(GZ_XLEN - 1)
            , extraLength(0)
            , subLength(0)
            , method(0)
            , flags(0)
            , extraFlags(0)
            , os(0)
            , version(0)
            , chunkLength(0)
            , chunkCount(0)
            , chunks(0)
            , offsets(0)
            , crc(0)
            , originalLength(0)
            , compressedLength(0)
        {   
        }   

        ~Private()
        {   
        }   
 
        unsigned char *start;	    /* start of mmap'd area */
        unsigned char *end;	    /* end of mmap'd area */
        unsigned long size;		        /* size of mmap */

        int type;
        z_stream zStream;
        int initialized;

        int headerLength;
        int extraLength;
        int subLength;
        char method;
        char flags;
        QDateTime mtime;
        char extraFlags;
        char os;
        int version;
        int chunkLength;
        int chunkCount;
        int *chunks;
        unsigned long *offsets;	/* Sum-scan of chunks. */
        QString originalFileName;
        QString comment;
        unsigned long crc;
        unsigned long originalLength;
        unsigned long compressedLength;
        QList<DictionaryCache> cache;
        QFile mapFile;

        static const int dictionaryCacheSize = 5;
};

DictionaryZip::DictionaryZip()
    : d(new Private)
{
}

DictionaryZip::~DictionaryZip()
{
    close();
}

int
DictionaryZip::readHeader(const QString &fileName, int computeCRC)
{
    char id1;
    char id2;
    char si1;
    char si2;
    char buffer[BUFFERSIZE];
    int i;
    unsigned long crc = crc32( 0L, Z_NULL, 0 );
    int count;
    unsigned long offset;

    QFile file(fileName);
    if( !file.open( QIODevice::ReadOnly ) ) 
    {   
        qDebug() << "Failed to open file:" << fileName;
        return -1;
    }   

    if (file.read( &id1, 1 ) < 0)
        qWarning() << "Invalid ZIP file. Unexpected end of file.";

    if (file.read( &id2, 1 ) < 0)
        qWarning() << "Invalid ZIP file. Unexpected end of file.";

    if (uchar(id1) != GZ_MAGIC1 || uchar(id2) != GZ_MAGIC2)
    {
        d->type = DICTIONARY_TEXT;
        d->compressedLength = d->originalLength = file.size();
        d->originalFileName = fileName;

        QFileInfo fileInfo(fileName);
        d->mtime = fileInfo.lastModified();
        if (computeCRC)
        {
            file.seek(0);
            while (!file.atEnd())
            {
                if ((count = file.read( buffer, 1*BUFFERSIZE)))
                    crc = crc32(crc, (Bytef *)buffer, count);
            }
        }
        d->crc = crc;
        file.close();
        return 0;
    }

    d->type = DICTIONARY_GZIP;

    if (file.read( &d->method, 1 ) < 0) {
        qWarning() << "Invalid ZIP file. Unexpected end of file.";
        return -1;
    }

    if (file.read( &d->flags, 1 ) < 0) {
        qWarning() << "Invalid ZIP file. Unexpected end of file.";
        return -1;
    }

    time_t mtime = 0;
    char chtime;
    for(int i = 0; i < 4; ++i) {
        if (file.read( &chtime, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        } else {
            mtime |= chtime << i*8;
        }
    }
    d->mtime.setTime_t(mtime);

    if (file.read( &d->extraFlags, 1 ) < 0) {
        qWarning() << "Invalid ZIP file. Unexpected end of file.";
        return -1;
    }

    if (file.read( &d->os, 1 ) < 0) {
        qWarning() << "Invalid ZIP file. Unexpected end of file.";
        return -1;
    }

    if (d->flags & GZ_FEXTRA)
    {
        d->extraLength = 0;
        char extraLength;
        for(int i = 0; i < 2; ++i) {
            if (file.read( &extraLength, 1 ) < 0) {
                qWarning() << "Invalid ZIP file. Unexpected end of file.";
                return -1;
            } else {
                d->extraLength |= extraLength << i*8;
            }
        }

        d->headerLength += extraLength + 2;

        if (file.read( &si1, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        }

        if (file.read( &si2, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        }

        if (uchar(si1) == GZ_RND_S1 || uchar(si2) == GZ_RND_S2)
        {
            d->subLength = 0;
            char subLength;
            for(int i = 0; i < 2; ++i) {
                if (file.read( &subLength, 1 ) < 0) {
                    qWarning() << "Invalid ZIP file. Unexpected end of file.";
                    return -1;
                } else {
                    d->subLength |= subLength << i*8;
                }
            }

            d->version = 0;
            char version;
            for(int i = 0; i < 2; ++i) {
                if (file.read( &version, 1 ) < 0) {
                    qWarning() << "Invalid ZIP file. Unexpected end of file.";
                    return -1;
                } else {
                    d->version |= version << i*8;
                }
            }

            if (d->version != 1) 
            {
                qDebug() << Q_FUNC_INFO << QString("dzip header version %1 not supported").arg(d->version);
            }

            d->chunkLength = 0;
            char chunkLength;
            for(int i = 0; i < 2; ++i) {
                if (file.read( &chunkLength, 1 ) < 0) {
                    qWarning() << "Invalid ZIP file. Unexpected end of file.";
                    return -1;
                } else {
                    d->chunkLength |= chunkLength << i*8;
                }
            }

            d->chunkCount = 0;
            char chunkCount;
            for(int i = 0; i < 2; ++i) {
                if (file.read( &chunkCount, 1 ) < 0) {
                    qWarning() << "Invalid ZIP file. Unexpected end of file.";
                    return -1;
                } else {
                    d->chunkCount |= chunkCount << i*8;
                }
            }

            if (d->chunkCount <= 0)
            {
                file.close();
                return 5; // TODO: const or enum value for this ?
            }

            d->chunks = (int *)malloc(sizeof( d->chunks[0] ) * d->chunkCount );

            char chunk;
            for (int j = 0; j < d->chunkCount; ++j)
            {
                d->chunks[j] = 0;
                for(int i = 0; i < 2; ++i) {
                    if (file.read( &chunk, 1 ) < 0) {
                        qWarning() << "Invalid ZIP file. Unexpected end of file.";
                        return -1;
                    } else {
                        d->chunks[j] |= chunk << i*8;
                    }
                }
            }
            d->type = DICTIONARY_DZIP;
        }
        else
        {
            file.seek(d->headerLength);
        }
    }

    if (d->flags & GZ_FNAME)
    { /* FIXME! Add checking against header len */
        int i = 0;
        while (file.read(&buffer[i++], 1) && !file.atEnd());
        buffer[i] = '\0';

        d->originalFileName = buffer;
        d->headerLength += d->originalFileName.length() + 1;
    }
    else
    {
        d->originalFileName = "";
    }

    if (d->flags & GZ_COMMENT)
    { /* FIXME! Add checking for header len */
        int i = 0;
        while (file.read(&buffer[i++], 1) && !file.atEnd());
        buffer[i] = '\0';
        d->comment = buffer;
        d->headerLength += d->comment.length() + 1;
    }
    else
    {
       d-> comment = "";
    }

    if (d->flags & GZ_FHCRC)
    {
        file.read(2);
        d->headerLength += 2;
    }

    if (file.pos() != d->headerLength + 1)
    {
        qDebug() << Q_FUNC_INFO << QString("File position (%1) != header length + 1 (%2)").arg(file.pos()).arg(d->headerLength + 1 ); 
    }

    file.seek(file.size() - 8);

    d->crc = 0;
    char chcrc;
    for(int i = 0; i < 4; ++i) {
        if (file.read(&chcrc, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        } else {
            d->crc |= chcrc << i*8;
        }
    }

    d->originalLength = 0;
    char length;
    for(int i = 0; i < 4; ++i) {
        if (file.read( &length, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        } else {
            d->originalLength |= length << i*8;
        }
    }

    d->compressedLength = file.pos();

    /* Compute offsets */
    d->offsets = (unsigned long *)malloc( sizeof( d->offsets[0] ) * d->chunkCount );
    
    for (offset = d->headerLength + 1, i = 0; i < d->chunkCount; ++i)
    {
        d->offsets[i] = offset;
        offset += d->chunks[i];
    }

    file.close();
    return 0;
}

bool
DictionaryZip::open(const QString& fileName, int computeCRC)
{
    d->initialized = 0;

    if (!QFileInfo(fileName).isFile())
    {
        qDebug() << Q_FUNC_INFO << QString("%1 is not a regular file -- ignoring").arg(fileName);
        return false;
    }

    if (readHeader(fileName, computeCRC))
    {
        qDebug() << Q_FUNC_INFO << QString("\"%1\" not in text or dzip format").arg(fileName);
        return false;
    }

    QFile file(fileName);
    if( !file.open( QIODevice::ReadOnly ) ) 
    {   
        qDebug() << "Failed to open file:" << fileName;
        return -1;
    }

    d->size = file.size();

    d->mapFile.setFileName(fileName);
    if( !d->mapFile.open( QIODevice::ReadOnly ) ) 
    {   
        qDebug() << "Failed to open file:" << fileName;
        return -1; 
    }   
 
    uchar *data = d->mapFile.map(0, d->size);
    if (data == NULL)
    {   
        qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(fileName);
        return false;
    }   

    d->start = data;
    d->end = d->start + d->size;

    for (int j = 0; j < d->dictionaryCacheSize; ++j)
    {
        DictionaryCache dictionaryCache;
        dictionaryCache.setChunk(-1);
        dictionaryCache.setStamp(-1);
        dictionaryCache.setByteArray(NULL);
        dictionaryCache.setCount(0);

        d->cache.append(dictionaryCache);
    }

    return true;
}

void
DictionaryZip::close()
{
    if (d->chunks)
        ::free(d->chunks);

    if (d->offsets)
        ::free(d->offsets);

    if (d->initialized)
    {
        if (inflateEnd( &d->zStream ))
        {
            qDebug() << Q_FUNC_INFO << QString("Cannot shut down inflation engine: %1").arg(d->zStream.msg);
        }
    }

    /* for (int i = 0; i < DICTIONARY_CACHE_SIZE; ++i)
    {
        if (d->cache[i].inBuffer)
            ::free(d->cache[i].inBuffer);
    } */
}

QByteArray
DictionaryZip::read(unsigned long start, unsigned long size)
{
    unsigned long end;
    int count;
    QByteArray inByteArray;
    QByteArray outByteArray;
    int firstChunk;
    int lastChunk;
    int firstOffset;
    int lastOffset;
    int found;
    int target;
    int lastStamp;
    static int stamp = 0;
    QByteArray resultString;

    end = start + size;

    switch (d->type)
    {
    case DICTIONARY_GZIP:
        qWarning() << Q_FUNC_INFO << "Cannot seek on pure gzip format files.";
        qWarning() << "Use plain text (for performance) or dzip format (for space savings).";
        break;

    case DICTIONARY_TEXT:
        resultString = QByteArray::fromRawData(reinterpret_cast<char*>(d->start), size);
        break;

    case DICTIONARY_DZIP:
        if (!d->initialized)
        {
            ++d->initialized;
            d->zStream.zalloc = NULL;
            d->zStream.zfree = NULL;
            d->zStream.opaque = NULL;
            d->zStream.next_in = 0;
            d->zStream.avail_in = 0;
            d->zStream.next_out = NULL;
            d->zStream.avail_out = 0;

            if (inflateInit2( &d->zStream, -15 ) != Z_OK)
            {
                qWarning() << Q_FUNC_INFO << QString("Cannot initialize inflation engine: %1").arg(d->zStream.msg);
            }
        }

        firstChunk = start / d->chunkLength;
        firstOffset = start - firstChunk * d->chunkLength;
        lastChunk = end / d->chunkLength;
        lastOffset = end - lastChunk * d->chunkLength;
        //PRINTF(DBG_UNZIP,
        // ("   start = %lu, end = %lu\n"
        //"firstChunk = %d, firstOffset = %d,"
        //" lastChunk = %d, lastOffset = %d\n",
        //start, end, firstChunk, firstOffset, lastChunk, lastOffset ));
        for (int i = firstChunk; i <= lastChunk; ++i)
        {
            /* Access cache */
            found = 0;
            target = 0;
            lastStamp = INT_MAX;
            for (int j = 0; j < d->dictionaryCacheSize; ++j)
            {
#if USE_CACHE
                if (d->cache.at(j).chunk() == i)
                {
                    found = 1;
                    target = j;
                    break;
                }
#endif
                if (d->cache.at(j).stamp() < lastStamp)
                {
                    lastStamp = d->cache.at(j).stamp();
                    target = j;
                }
            }

            d->cache[target].setStamp(++stamp);
            if (found)
            {
                count = d->cache.at(target).count();
                inByteArray = d->cache.at(target).byteArray();
            }
            else
            {
                d->cache[target].setChunk(i);

                inByteArray = d->cache.at(target).byteArray();

                if (d->chunks[i] >= OUT_BUFFER_SIZE )
                {
                    qDebug() << Q_FUNC_INFO << QString("chunks[%1] = %2 >= %3 (OUT_BUFFER_SIZE)").arg(i).arg(d->chunks[i]).arg(OUT_BUFFER_SIZE);
                }

                outByteArray = QByteArray::fromRawData(reinterpret_cast<char*>(d->start + d->offsets[i]), d->chunks[i]);

                d->zStream.next_in = (Bytef *)outByteArray.data();
                d->zStream.avail_in = d->chunks[i];
                d->zStream.next_out = (Bytef *)inByteArray.data();
                d->zStream.avail_out = IN_BUFFER_SIZE;

                if (inflate( &d->zStream, Z_PARTIAL_FLUSH ) != Z_OK)
                {
                    qWarning() << Q_FUNC_INFO << QString("inflate: %1").arg(d->zStream.msg);
                }

                if (d->zStream.avail_in)
                {
                    qWarning() << Q_FUNC_INFO << QString("inflate did not flush (%1 pending, %2 avail)").arg(d->zStream.avail_in).arg(d->zStream.avail_out);
                }

                count = IN_BUFFER_SIZE - d->zStream.avail_out;

                d->cache[target].setCount(count);
            }

            if (i == firstChunk)
            {
                if (i == lastChunk)
                {
                    inByteArray.append(inByteArray.mid(firstOffset, lastOffset - firstOffset));
                }
                else
                {
                    if (count != d->chunkLength )
                    {
                        qDebug() << Q_FUNC_INFO << QString("Length = %1 instead of %2").arg(count).arg(d->chunkLength);
                    }

                    inByteArray.append(inByteArray.mid(firstOffset, d->chunkLength - firstOffset));
                }
            }
            else if (i == lastChunk)
            {
                inByteArray.append(inByteArray.left(lastOffset));
            }
            else
            {
                Q_ASSERT(count == d->chunkLength);
                inByteArray.append(inByteArray.left(d->chunkLength));
            }
        }
        break;

    case DICTIONARY_UNKNOWN:
        qWarning() << Q_FUNC_INFO << "Cannot read unknown file type";
        break;
    }

    return resultString;
}
