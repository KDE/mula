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

#ifndef MULA_PLUGIN_STARDICT_WORDENTRY_H
#define MULA_PLUGIN_STARDICT_WORDENTRY_H

#include <QtCore/QByteArray>
#include <QtCore/QSharedPointer>

namespace MulaPluginStarDict
{
    /**
    * \brief Collection of data which describes the word entry
    *
    * A word entry is an object containing a set of data.
    * Each member describes the relevant information related to the desired
    * ".dict" file
    *
    * Each entry in the word list contains three fields, one after the other:
    * 1) The utf-8 string: It represents the string that is looked up by the
    * application. The length of the string should be less than 256. Two or more
    * entries may have the same string value with different offset and size.
    * This may be useful for some dictionaries, but this feature is only well
    * supported by StarDict-2.4.8 and newer versions.
    *
    * 2) The offset of the word data in the desired ".dict" file: If the version
    * is "3.0.0" and "idxoffsetbits=64": offset will be 64-bits unsigned number
    * in network byte order. Otherwise it will be 32-bits.
    *
    * 3) The total size of the word data in the desired ".dict" file: It should
    * be 32-bits unsigned number in network byte order.
    *
    * Note: It is possible the different strings have the same offset and size
    * value. In other words, multiple word indices point to the same definition.
    * This is not recommended, for multiple words have the same definition,
    * you may create a ".syn" file for them.
    *
    * \see Indexfile
    */
    class WordEntry
    {
        public:
            /**
             * Constructor.
             */
            WordEntry();

            /**
             * Copy Constructor.
             */
            WordEntry(const WordEntry &other);

            /**
             * Destructor.
             */
            virtual ~WordEntry();

            /**
             * Assignment operator.
             */
            WordEntry& operator=(const WordEntry &other);

            /**
             * Sets the data of the word entry representing the the utf-8 string
             * terminated by '\0' in the desired ".dict" file
             *
             * @param plugin The utf-8 string in a raw format
             * @see data
             */
            void setData(QByteArray data);

            /**
             * Returns the data of the word entry representing the utf-8 string
             * terminated by '\0' in the desired ".dict" file
             *
             * @return The utf-8 string of the word entry in a raw format
             * @see setData
             */
            QByteArray data() const;

            /**
             * Sets the offset of the word entry representing the offset of the
             * word data in the desired ".dict" file
             *
             * @param plugin The dataOffset of the word data
             * @see offset
             */
            void setDataOffset(quint32 dataOffset);

            /**
             * Returns the offset of the word entry representing the offset of
             * the word data in the desired ".dict" file
             *
             * @return The offset of the word data
             * @see setOffset
             */
            quint32 dataOffset() const;

            /**
             * Sets the size of the word entry in this word entry representing
             * the total size of the word data in the desired ".dict" file
             *
             * @param plugin The size of the word data
             * @see dataSize
             */
            void setDataSize(quint32 dataSize);

            /**
             * Returns the size of the word entry representing the total size
             * of the word data in the desired ".dict" file
             *
             * @return The size of the word data
             * @see setDataSize
             */
            quint32 dataSize() const;

        private:
            class Private;
            QSharedDataPointer<Private> d;
    };
}

#endif // MULA_PLUGIN_STARDICT_WORDENTRY_H
