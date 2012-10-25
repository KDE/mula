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

#ifndef MULA_PLUGIN_STARDICT_ABSTRACTINDEXFILE_H
#define MULA_PLUGIN_STARDICT_ABSTRACTINDEXFILE_H

#include <QtCore/QtGlobal>

class QString;

namespace MulaPluginStarDict
{
    class AbstractIndexFile
    {
        public:

            /**
             * Constructor
             */

            AbstractIndexFile();

            /**
             * Destructor
             */

            virtual ~AbstractIndexFile();

            /**
             * Loads the relevant index, cache file and so on. Since it is
             * just an abstract base class, it is up to the successor to decide
             * what method to use to actually interpret the loading.
             *
             * @param   filePath   The complete file path of the index file
             *
             * @return Whether or not the loading was successful
             */

            virtual bool load(const QString& filePath) = 0;

            /**
             * Returns the word data according to the relevant index
             *
             * \note Since it is an abstract base class, the real
             * implementation is up to the successors whether to return the
             * word directly, or to use some caching mechanism or something
             * else.
             *
             * @param   index   The index of the desired word
             *
             * @return  The word data
             */

            virtual QByteArray key(long index) = 0;

            /**
             * Returns the index of the word data where it has been found. The
             * method will also return the fact whether or not the desired word
             * could be found in the index file.
             *
             * \note Since it is an abstract base class, the real
             * implementation is up to the successors whether to return the
             * index and the found information directly, or to use some caching
             * mechanism or something else.
             *
             * @param   word    The word data to look up
             *
             * @return The index where the desired word occurs among the word
             * entries, or -1 if there is no such a word.
             */

            virtual int lookup(const QByteArray& word) = 0;

            virtual quint32 wordEntryOffset() const;
            virtual void setWordEntryOffset(quint32 wordEntryOffset);

            virtual quint32 wordEntrySize() const;
            virtual void setWordEntrySize(quint32 wordEntrySize);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_ABSTRACTINDEXFILE_H
