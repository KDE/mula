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

#ifndef MULA_PLUGIN_STARDICT_ABSTRACTDICTIONARY_H
#define MULA_PLUGIN_STARDICT_ABSTRACTDICTIONARY_H

#include <QtCore/QStringList>

class QFile;

namespace MulaPluginStarDict
{
    class DictionaryZip;

    class AbstractDictionary
    {
        public:
            /**
             * Constructor.
             */
            AbstractDictionary();

            /**
             * Destructor.
             */
            virtual ~AbstractDictionary();

            const QByteArray wordData(quint32 indexItemOffset, qint32 indexItemSize);

            /**
             * Returns whether the dictionary contains any of the given same
             * type sequence characters
             *
             * @return True if the any of the given same same type sequence
             * characters is contained by the dictionary, otherwise false.
             * @see finData, wordData
             */
            bool containFindData();

            bool findData(const QStringList &searchWords, qint32 indexItemOffset, qint32 indexItemSize, QByteArray& originalData);

            /**
             * Returns the compressed ".dict.dz" dictionary file
             *
             * @return The compressed dictionary file
             * @see dictionaryFile
             */
            DictionaryZip* compressedDictionaryFile() const;

            /**
             * Returns the ".dict" dictionary file
             *
             * @return The dictionary file
             * @see compressedDictionaryFile
             */
            QFile* dictionaryFile() const;

            /**
             * Sets the value of the same type sequence
             *
             * @param sameTypeSequence The same type sequence
             * @see sameTypeSequence
             */
            void setSameTypeSequence(const QString& sameTypeSequence);

            /**
             * Returns the value of the same type sequence
             *
             * @return The value of the same typesequence
             * @see setSameTypeSequence
             */
            QString& sameTypeSequence() const;

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_ABSTRACtDICTIONARY_H
