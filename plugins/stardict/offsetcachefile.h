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

#ifndef MULA_PLUGIN_STARDICT_OFFSETCACHEFILE_H
#define MULA_PLUGIN_STARDICT_OFFSETCACHEFILE_H

#include "abstractindexfile.h"

#include <QtCore/QStringList>

namespace MulaPluginStarDict
{
    /**
     * \brief The class is used for dealing the cache file related operations.
     *
     * StarDict-2.4.8 started to support cache files. The cache file usage can
     * speed up the loading and save memory by mapping the cache file. The
     * cache file names are .idx.oft and .syn.oft.
     * The cache file contains a utf-8 string terminated by '\0', and then
     * 32-bits numbers. These numbers are the offset value of the cache pages.
     * Each cache page contains "pageEntryNumber" word entries inside the index
     * file. They are not stored in network byte order. The index file does not
     * need to be parsed by going through every byte, thus it can provide better
     * performance this way.
     *
     * The string must begin with:
     * =====
     * StarDict's oft file
     * version=2.4.8
     * =====
     * The following line is something like this:
     * url=/usr/share/stardict/dic/stardict-somedict-2.4.2/somedict.idx
     * This line should end by '\n'.
     *
     * The class will try to create the .oft offset file, if failed, in the same
     * directory where the .ifo file can be found. The class will try to create
     * the cache file in the ${CACHE_LOCATION}/stardict/ folder where the cache
     * path is provided by QDesktopService class using the CacheLocation
     * argument.
     *
     * If two or more dictionaries have the same file name, StarDict will
     * create somedict.idx.oft, somedict(2).idx.oft, somedict(3).idx.oft and the
     * like, for them respectively. Each starts with different "url=" in the
     * beginning string.
     *
     * \see Indexfile
     */
    class OffsetCacheFile : public AbstractIndexFile
    {
        public:
            /**
             * Constructor.
             */
            OffsetCacheFile();

            /**
             * Destructor.
             */
            virtual ~OffsetCacheFile();

            // Loads the cache or creates if it does not exist
            bool load(const QString& completeFilePath, int wordCount, qulonglong fileSize);
            QByteArray key(long index);

            bool lookup(const QByteArray& string, long &index);

            virtual quint32 wordEntryOffset() const;
            virtual void setWordEntryOffset(quint32 wordEntryOffset);

            virtual quint32 wordEntrySize() const;
            virtual void setWordEntrySize(quint32 wordEntrySize);

        private:
            int loadPage(int pageIndex);
            QByteArray readFirstWordDataOnPage(long pageIndex);
            QByteArray firstWordDataOnPage(long pageIndex);
            QStringList cacheLocations(const QString& completeFilePath);
            bool loadCache(const QString& completeFilePath);
            bool saveCache(const QString& completeFilePath);

            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_OFFSETCACHEFILE_H
