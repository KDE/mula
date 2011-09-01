/******************************************************************************
 * This file is part of the MULA project
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

#include "dictziplib.h"

#include <QtGlobal>

#include <QtCore/QDebug>>
#include <QtCore/QString>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
#ifdef Q_OS_WIN32
#include <io.h>
#endif
#include <limits.h>
#include <fcntl.h>

#include <sys/stat.h>

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
    DICT_UNKNOWN    = 0,
    DICT_TEXT       = 1,
    DICT_GZIP       = 2,
    DICT_DZIP       = 3,
}


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
            , mtime(0)
            , extraFlags(0)
            , os(0)
            , version(0)
            , chuckLength(0)
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
 
        const unsigned char *start;	    /* start of mmap'd area */
        const unsigned char *end;	    /* end of mmap'd area */
        unsigned long size;		        /* size of mmap */

        int type;
        z_stream zStream;
        int initialized;

        int headerLength;
        int extraLength;
        int subLength;
        int method;
        int flags;
        time_t mtime;
        int extraFlags;
        int os;
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
        DictionaryCache *cache;
        QFile mapFile;
}

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
    int id1;
    int id2;
    int si1;
    int si2;
    char buffer[BUFFERSIZE];
    int subLength;
    int i;
    char *pt;
    int c;
    struct stat sb;
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

    if (id1 != GZ_MAGIC1 || id2 != GZ_MAGIC2)
    {
        m_type = DICT_TEXT;
        m_compressedLength = m_originalLength = file.size();
        m_originalFilename = fileName;

        QFileInfo fileInfo(fileName)
        mtime = fileInfo.lastModified();
        if (computeCRC)
        {
            file.seek(0);
            while (!file.atEnd())
            {
                if ((count = file.read( buffer, 1*BUFFERSIZE)))
                    crc = crc32(crc, (Bytef *)buffer, count);
            }
        }
        m_crc = crc;
        file.close();
        return 0;
    }

    m_type = DICT_GZIP;

    if (file.read( &m_method, 1 ) < 0) {
        qWarning() << "Invalid ZIP file. Unexpected end of file.";
        return -1;
    }

    if (file.read( &m_flags, 1 ) < 0) {
        qWarning() << "Invalid ZIP file. Unexpected end of file.";
        return -1;
    }

    time_t mtime;
    for(int i = 0, int mtime = 0, m_mtime = 0; i < 4; ++i) {
        if (file.read( &mtime, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        } else {
            m_mtime |= mtime << i*8;
        }
    }

    if (file.read( &m_extraFlags, 1 ) < 0) {
        qWarning() << "Invalid ZIP file. Unexpected end of file.";
        return -1;
    }

    if (file.read( &m_os, 1 ) < 0) {
        qWarning() << "Invalid ZIP file. Unexpected end of file.";
        return -1;
    }

    if (m_flags & GZ_FEXTRA)
    {
        for(int i = 0, int extraLength, m_extraLength = 0; i < 2; ++i) {
            if (file.read( &extraLength, 1 ) < 0) {
                qWarning() << "Invalid ZIP file. Unexpected end of file.";
                return -1;
            } else {
                m_extraLength |= extraLength << i*8;
            }
        }

        m_headerLength += extraLength + 2;

        if (file.read( &si1, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        }

        if (file.read( &si2, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        }

        if (si1 == GZ_RND_S1 || si2 == GZ_RND_S2)
        {

            for(int i = 0, int subLength, m_subLength = 0; i < 2; ++i) {
                if (file.read( &subLength, 1 ) < 0) {
                    qWarning() << "Invalid ZIP file. Unexpected end of file.";
                    return -1;
                } else {
                    m_subLength |= subLength << i*8;
                }
            }

            for(int i = 0, int version, m_version = 0; i < 2; ++i) {
                if (file.read( &version, 1 ) < 0) {
                    qWarning() << "Invalid ZIP file. Unexpected end of file.";
                    return -1;
                } else {
                    m_version |= version << i*8;
                }
            }

            if (m_version != 1) 
            {
                qDebug() << Q_FUNC << QString("dzip header version %1 not supported").arg(m_version);
            }

            for(int i = 0, m_chunkLength = 0; i < 2; ++i) {
                if (file.read( &chunkLength, 1 ) < 0) {
                    qWarning() << "Invalid ZIP file. Unexpected end of file.";
                    return -1;
                } else {
                    m_chunkLength |= chunkLength << i*8;
                }
            }

            for(int i = 0, int chunkCount, m_chunkCount = 0; i < 2; ++i) {
                if (file.read( &chunkCount, 1 ) < 0) {
                    qWarning() << "Invalid ZIP file. Unexpected end of file.";
                    return -1;
                } else {
                    m_chunkCount |= chunkCount << i*8;
                }
            }

            if (m_chunkCount <= 0)
            {
                file.close();
                return 5; // TODO: const or enum value for this ?
            }

            m_chunks = (int *)malloc(sizeof( m_chunks[0] ) * m_chunkCount );

            for (int j = 0, int chunk = 0, m_chunks[j] = 0; j < m_chunkCount; ++j)
            {
                for(int i = 0; i < 2; ++i) {
                    if (file.read( &chunk, 1 ) < 0) {
                        qWarning() << "Invalid ZIP file. Unexpected end of file.";
                        return -1;
                    } else {
                        m_chunks[j] |= chunk << i*8;
                    }
                }
            }
            m_type = DICT_DZIP;
        }
        else
        {
            file.seek(m_headerLength);
        }
    }

    if (m_flags & GZ_FNAME)
    { /* FIXME! Add checking against header len */
        int i = 0;
        while ((file.read(&buffer[i++], 1) && !file.atEnd());
        buffer[i] = '\0';

        m_originalFileName = buffer;
        m_headerLength += m_originalFileName.length() + 1;
    }
    else
    {
        m_origFilename = "";
    }

    if (m_flags & GZ_COMMENT)
    { /* FIXME! Add checking for header len */
        int i = 0;
        while ((file.read(&buffer[i++], 1) && !file.atEnd());
        buffer[i] = '\0';
        m_comment = buffer;
        m_headerLength += m_comment.length() + 1;
    }
    else
    {
        comment = "";
    }

    if (m_flags & GZ_FHCRC)
    {
        file.read(2)
        m_headerLength += 2;
    }

    if (file.pos() != m_headerLength + 1)
    {
        qDebug() << Q_FUNC << QString("File position (%1) != header length + 1 (%2)").arg(file.pos()).arg(m_headerLength + 1 ) << endl; 
    }

    file.seek(file.size() - 8);

    for(int i = 0, int crc = 0, m_crc = 0; i < 4; ++i) {
        if (file.read(&crc, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        } else {
            m_crc |= crc << i*8;
        }
    }

    for(int i = 0, int length = 0, m_length = 0; i < 4; ++i) {
        if (file.read( &length, 1 ) < 0) {
            qWarning() << "Invalid ZIP file. Unexpected end of file.";
            return -1;
        } else {
            m_originalLength |= length << i*8;
        }
    }

    m_compressedLength = file.pos();

    /* Compute offsets */
    m_offsets = (unsigned long *)malloc( sizeof( m_offsets[0] ) * m_chunkCount );
    
    for (offset = m_headerLength + 1, i = 0; i < m_chunkCount; ++i)
    {
        m_offsets[i] = offset;
        offset += m_chunks[i];
    }

    file.close();
    return 0;
}

bool
DictionaryZip::open(const QString& fileName, int computeCRC)
{
    struct stat sb;
    int j;

    m_initialized = 0;

#ifdef Q_OS_UNIX
    if (stat(fname.c_str(), &sb) || !S_ISREG(sb.st_mode))
#elif defined(Q_OS_WIN32)
    if (_stat(fname.c_str(), &sb) || !(sb.stMode & _S_IFREG))
#endif
    {
        qDebug() << Q_FUNC << QString("%1 is not a regular file -- ignoring").arg(fname) << endl;
        return false;
    }

    if (readHeader(fileName, computeCRC))
    {
        qDebug() << Q_FUNC << QString("\"%1\" not in text or dzip format").arg(fileName) << endl;
        return false;
    }

    QFile file(fileName);
    if( !file.open( QIODevice::ReadOnly ) ) 
    {   
        qDebug() << "Failed to open file:" << fileName;
        return -1;
    }

    m_size = file.size();

    m_mapFile.setFileName(fileName);
    if( !m_mapFile.open( QIODevice::ReadOnly ) ) 
    {   
        qDebug() << "Failed to open file:" << fileName;
        return -1; 
    }   
 
    data = QFile.map(0, m_size);
    if (data == NULL)
    {   
        QDebug() << Q_FUNC() << QString("Mapping the file %1 failed!").arg(m_indexFileName) << endl;
        return false;
    }   

    m_start = data;
    m_end = m_start + m_size;

    for (j = 0; j < DICT_CACHE_SIZE; ++j)
    {
        cache[j].chunk = -1;
        cache[j].stamp = -1;
        cache[j].inBuffer = NULL;
        cache[j].count = 0;
    }

    return true;
}

void
DictionaryZip::close()
{
    int i;

    if (m_chunks)
        free(m_chunks);

    if (m_offsets)
        free(m_offsets);

    if (m_initialized)
    {
        if (inflateEnd( &m_zStream ))
        {
            qDebug() << Q_FUNC << QString("Cannot shut down inflation engine: %1").arg(m_zStream.msg) << endl;
        }
    }

    for (i = 0; i < DICT_CACHE_SIZE; ++i)
    {
        if (m_cache[i].inBuffer)
            free(m_cache[i].inBuffer);
    }
}

void
DictionaryZip::read(char *buffer, unsigned long start, unsigned long size)
{
    char *pt;
    unsigned long end;
    int count;
    char *inBuffer;
    char outBuffer[OUT_BUFFER_SIZE];
    int firstChunk, lastChunk;
    int firstOffset, lastOffset;
    int i, j;
    int found, target, lastStamp;
    static int stamp = 0;

    end = start + size;

    switch (m_type)
    {
    case DICTIONARY_GZIP:
        qWarning() << Q_FUNC << "Cannot seek on pure gzip format files." << endl
        << "Use plain text (for performance) or dzip format (for space savings)." << endl;
        break;
    case DICTIONARY_TEXT:
        memcpy( buffer, m_start + start, size );
        //buffer[size] = '\0';
        break;
    case DICTIONARY_DZIP:
        if (!m_initialized)
        {
            ++m_initialized;
            m_zStream.zalloc = NULL;
            m_zStream.zfree = NULL;
            m_zStream.opaque = NULL;
            m_zStream.next_in = 0;
            m_zStream.avail_in = 0;
            m_zStream.next_out = NULL;
            m_zStream.avail_out = 0;

            if (inflateInit2( &m_zStream, -15 ) != Z_OK)
            {
                qWarning() << Q_FUNC << QString("Cannot initialize inflation engine: %1").arg(m_zStream.msg) << endl;
            }
        }

        firstChunk = start / m_chunkLength;
        firstOffset = start - firstChunk * m_chunkLength;
        lastChunk = end / m_chunkLength;
        lastOffset = end - lastChunk * m_chunkLength;
        //PRINTF(DBG_UNZIP,
        // ("   start = %lu, end = %lu\n"
        //"firstChunk = %d, firstOffset = %d,"
        //" lastChunk = %d, lastOffset = %d\n",
        //start, end, firstChunk, firstOffset, lastChunk, lastOffset ));
        for (pt = buffer, i = firstChunk; i <= lastChunk; i++)
        {
            /* Access cache */
            found = 0;
            target = 0;
            lastStamp = INT_MAX;
            for (int j = 0; j < DICT_CACHE_SIZE; ++j)
            {
#if USE_CACHE
                if (m_cache[j].chunk == i)
                {
                    found = 1;
                    target = j;
                    break;
                }
#endif
                if (m_cache[j].stamp < lastStamp)
                {
                    lastStamp = m_cache[j].stamp;
                    target = j;
                }
            }

            m_cache[target].stamp = ++stamp;
            if (found)
            {
                count = m_cache[target].count;
                inBuffer = m_cache[target].inBuffer;
            }
            else
            {
                m_cache[target].chunk = i;
                if (!m_cache[target].inBuffer)
                    m_cache[target].inBuffer = (char *)malloc( IN_BUFFER_SIZE );
                inBuffer = m_cache[target].inBuffer;

                if (m_chunks[i] >= OUT_BUFFER_SIZE )
                {
                    //err_internal( __FUNCTION__,
                    //    "this->chunks[%d] = %d >= %ld (OUT_BUFFER_SIZE)\n",
                    //  i, this->chunks[i], OUT_BUFFER_SIZE );
                }
                memcpy( outBuffer, m_start + m_offsets[i], m_chunks[i] );

                m_zStream.next_in = (Bytef *)outBuffer;
                m_zStream.avail_in = m_chunks[i];
                m_zStream.next_out = (Bytef *)inBuffer;
                m_zStream.avail_out = IN_BUFFER_SIZE;
                if (inflate( &m_zStream, Z_PARTIAL_FLUSH ) != Z_OK)
                {
                    qWarning() << Q_FUNC << QString("inflate: %1").arg(zStream.msg) << endl;
                }
                if (m_zStream.avail_in)
                {
                    qWarning() << Q_FUNC << QString("inflate did not flush (%1 pending, %2 avail)").arg(m_zStream.avail_in).arg(m_zStream.avail_out) << endl;
                }

                count = IN_BUFFER_SIZE - m_zStream.avail_out;

                m_cache[target].count = count;
            }

            if (i == firstChunk)
            {
                if (i == lastChunk)
                {
                    memcpy( pt, inBuffer + firstOffset, lastOffset - firstOffset);
                    pt += lastOffset - firstOffset;
                }
                else
                {
                    if (count != m_chunkLength )
                    {
                        //err_internal( __FUNCTION__,
                        //	"Length = %d instead of %d\n",
                        //count, this->chunkLength );
                    }
                    memcpy( pt, inBuffer + firstOffset,
                            m_chunkLength - firstOffset );
                    pt += m_chunkLength - firstOffset;
                }
            }
            else if (i == lastChunk)
            {
                memcpy( pt, inBuffer, lastOffset );
                pt += lastOffset;
            }
            else
            {
                assert( count == m_chunkLength );
                memcpy( pt, inBuffer, m_chunkLength );
                pt += m_chunkLength;
            }
        }
        //*pt = '\0';
        break;
    case DICTIONARY_UNKNOWN:
        qWarning() << Q_FUNC << "Cannot read unknown file type" << endl;
        break;
    }
}
