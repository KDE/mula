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

#ifndef MULA_PLUGIN_STARDICT_DICTIONARYCACHE_H
#define MULA_PLUGIN_STARDICT_DICTIONARYCACHE_H

#include <QtCore/QByteArray>
#include <QtCore/QSharedPointer>

namespace MulaPluginStarDict
{
    class DictionaryCache
    {
        public:

            /**
             * Constructor
             */
            DictionaryCache();


            /**
             * Constructor
             *
             * @param chunk
             * @param byteArray
             * @param stamp
             * @param count
             */
            DictionaryCache(int chunk, QByteArray byteArray, int stamp, int count);

            /**
             * Copy Constructor
             */
            DictionaryCache(const DictionaryCache &other);

            /**
             * Destructor
             */
            virtual ~DictionaryCache();

            /**
             * Assignment operator
             */
            DictionaryCache& operator=(const DictionaryCache &other);

            /** 
             * Sets the chuck value
             *
             * @param chunk The chunk value
             *
             * @see chunk
             */
            void setChunk(int chunk);

            /**
             * Returns the chunk value
             *
             * @return The chunk value
             *
             * @see setChunk
             */
            int chunk() const;

            /** 
             * Sets the cache data
             *
             * @param data The input cache data
             *
             * @see byteArray
             */
            void setByteArray(QByteArray data);

            /**
             * Returns the cache data
             *
             * @return The cache data
             *
             * @see setByteArray
             */
            QByteArray byteArray() const;

            /** 
             * Sets the stamp value
             *
             * @param stamp The stamp value
             *
             * @see stamp
             */
            void setStamp(int stamp);

            /**
             * Returns the stamp value
             *
             * @return The stamp value
             *
             * @see setStamp
             */
            int stamp() const;

            /** 
             * Sets the cache count
             *
             * @param count The cache count
             *
             * @see count
             */
            void setCount(int count);

            /**
             * Returns the cache count
             *
             * @return The cache count
             *
             * @see setCount
             */
            int count() const;

        private:
            class Private;
            QSharedDataPointer<Private> d;
    };
}

#endif // MULA_PLUGIN_STARDICT_DICTIONARYCACHE_H
