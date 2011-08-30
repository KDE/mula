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

#ifndef MULA_PLUGIN_STARDICT_DICTIONARY_ZIP_LIB
#define MULA_PLUGIN_STARDICT_DICTIONARY_ZIP_LIB

#include <QtCore/QTime>
#include <QtCore/QString>
#include <QtCore/QFile>

#include <zlib.h>

#define DICT_CACHE_SIZE 5

class DictionaryCache
{
    public:
        DictionaryCache();
        virtual ~DictionaryCache();

    private:
        int chunk;
        char *inBuffer;
        int stamp;
        int count;
};

class DictionaryData
{
    DictionaryData();
    virtual ~DictionaryData();

    bool open(const QString& fileName, int computeCRC);
    void close();
    void read(char *buffer, unsigned long start, unsigned long size);

private:
    int readHeader(const QString &filename, int computeCRC);

    const unsigned char *m_start;	    /* start of mmap'd area */
    const unsigned char *m_end;		    /* end of mmap'd area */
    unsigned long m_size;		        /* size of mmap */

    int type;
    z_stream zStream;
    int initialized;

    int m_headerLength;
    int m_extraLength;
    int m_subLength;
    int m_method;
    int m_flags;
    time_t m_mtime;
    int m_extraFlags;
    int m_os;
    int m_version;
    int m_chunkLength;
    int m_chunkCount;
    int *m_chunks;
    unsigned long *m_offsets;	/* Sum-scan of chunks. */
    QString m_originalFileName;
    QString m_comment;
    unsigned long m_crc;
    unsigned long m_originalLength;
    unsigned long m_compressedLength;
    DictionaryCache *m_cache;
    QFile m_mapFile;
};

#endif // MULA_PLUGIN_STARDICT_DICTIONARY_ZIP_LIB
