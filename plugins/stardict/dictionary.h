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

#ifndef MULA_PLUGIN_STARDICT_DICTIONARY_H
#define MULA_PLUGIN_STARDICT_DICTIONARY_H

#include "abstractdictionary.h"

#include <QtCore/QString>

namespace MulaPluginStarDict
{
    class Dictionary : public AbstractDictionary
    {
        public:
            Dictionary();
            virtual ~Dictionary();

            /**
             * Loads the ifo file
             *
             * @param ifoFilePath The path of the ifo file
             * @return True if the loading went fine, otherwise false.
             */
            bool load(const QString& ifoFilePath);

            /**
             * Returns the count of the word entries in the .idx file.
             *
             * @return The count of the word entries
             */
            int articleCount() const;

            const QString dictionaryName() const;

            const QString ifoFilePath() const;

            const QString key(long index) const;

            QString data(long index);

            void keyAndData(long index, QByteArray key, qint32& offset, qint32 &size);

            bool lookup(const QString word, int &index);

            bool lookupWithRule(const QString& pattern, long *aIndex, int iBuffLen);

        private:
            /**
             * Loads the ifo file. Only for internal use.
             *
             * @param ifoFilePath The path of the ifo file
             * @return True if the loading went fine, otherwise false.
             */
            bool loadIfoFile(const QString& ifoFilePath);

            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_DICTIONARY_H
