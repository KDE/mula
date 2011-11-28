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
     * \brief The class is used for dealing cache file related operations.
     *
     * StarDict-2.4.8 started to support cache files. The cache file usage can
     * speed up the loading and save memory by mapping the cache file. The
     * cache file names are ".idx.oft" and ".syn.oft".
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
     * The class will try to create the ".oft" offset file, if failed, in the same
     * directory where the ".ifo" file can be found. The class will try to create
     * the cache file in the ${CACHE_LOCATION}/stardict/ folder where the cache
     * path is provided by QDesktopService class using the CacheLocation
     * argument.
     *
     * If two or more dictionaries have the same file name, StarDict will
     * create "somedict.idx.oft", "somedict(2).idx.oft", "somedict(3).idx.oft"
     * and the like, for them respectively. Each starts with different "url=" in
     * the beginning string.
     *
     * \see Indexfile
     */
    class OffsetCacheFile : public AbstractIndexFile
    {
        public:
            /**
             * Constructor
             */
            OffsetCacheFile();

            /**
             * Destructor
             */
            virtual ~OffsetCacheFile();

            // Loads the cache or creates if it does not exist
            /** Reimplemented from AbstractIndexFile::load() */
            bool load(const QString& completeFilePath, qulonglong filesize, int wordCount);

            /** Reimplemented from AbstractIndexFile::key() */
            QByteArray key(long index);

            /** Reimplemented from AbstractIndexFile::lookup() */
            bool lookup(const QByteArray& string, int &index);

        private:
            /**
             * Loads the word entries of relevant cache page into the internal
             * data storage. It will return the number of the word entries
             * loaded.
             *
             * \note It always loads the pageEntryNumber except the last page,
             * if that is not completely reserved. This method will just load the
             * last few entries then in that case. This method is only for
             * internal usage.
             *
             * @param   pageIndex   The index of the desired page
             *
             * @return  The number of the word entries loaded
             *
             * @see load
             */
            int loadPage(int pageIndex);

            /**
             * Looks up the word on the existing pages and then returns the
             * relevant page index where the word can be found. This method also
             * returns whether or not the desired page can be found.
             *
             * \note This method does not return the index of the word. It will
             * return the index of the page where the word occurs. This method
             * is only for internal usage.
             *
             * @param   word        The desired word to look up
             * @param   pageIndex   The index of the page where the word occurs
             *
             * @return True if the desired page can be found where the word
             * occurs, otherwise false.
             *
             * @see lookup
             */
            bool lookupPage(const QByteArray& word, int& pageIndex);

            /**
             * Returns the first word data of the desired page from the index
             * file. It is a bit slower since it starts seeking and reading
             * from the index file directly without using any caching mechanism
             *
             * \note This method is only for internal usage.
             *
             * @param   pageIndex The index of the desired page
             *
             * @return  The desired word data from the index file
             *
             * @see firstWordDataOnPage
             */
            QByteArray readFirstWordDataOnPage(long pageIndex);

            /**
             * Returns the first word data of the desired page from the index
             * file by using a caching approach to make it faster to look the
             * word up in certain cases. Hence, it is faster to use this method
             * for reading the first word data of the cache page.
             *
             * \note This method is only for internal usage.
             *
             * @param   pageIndex The index of the desired page
             *
             * @return  The desired word data from the index file
             *
             * @see readFirstWordDataOnPage
             */
            QByteArray firstWordDataOnPage(long pageIndex);

            /**
             * Returns a string list of the cache locations.
             * The method will return the location where the index file can be
             * found and the location ${CACHE_LOCATION}/stardict/ folder where
             * the cache path is provided by QDesktopService class using the
             * CacheLocation argument.
             *
             * \note This method is only for internal usage.
             *
             * @param   completeFilePath The complete file path
             *
             * @return  List of the cache locations
             *
             * @see saveCache, loadCache
             */
            QStringList cacheLocations(const QString& completeFilePath);

            /**
             * Loads the cache file according to the relevant index file path.
             * Returns true if the cache file loading is successful, otherwise
             * returns false.
             *
             * \note This method is only for internal usage.
             *
             * @param   completeFilePath The complete file path
             *
             * @return True if the cache loading was successful, otherwise
             * false.
             *
             * @see cacheLocations, saveCache
             */
            bool loadCache(const QString& completeFilePath);

            /**
             * Saves the cache file according to the relevant index file path.
             * Returns true if the cache file saving is successful, otherwise
             * returns false.
             *
             * \note This method is only for internal usage.
             *
             * @param   completeFilePath The complete file path
             *
             * @return True if the cache saving was successful, otherwise
             * false.
             *
             * @see cacheLocations, loadCache
             */
            bool saveCache(const QString& completeFilePath);

            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_OFFSETCACHEFILE_H
