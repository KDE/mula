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

#ifndef MULA_CORE_DICTIONARYINFO_H
#define MULA_CORE_DICTIONARYINFO_H

#include <QtCore/QString>

namespace MulaCore
{
    /**
     * This class represents information about dictionary.
     */
    class DictionaryInfo
    {
        public:
            /**
             * Construct an empty DictionaryData object.
             */
            DictionaryInfo();

            /**
             * Construct a DictionaryData object from the desired data.
             * @param plugin A plugin name
             * @param name A dictionary name
             * @param author A dictionary author
             * @param desription A dictionary description
             * @param wordsCount A count of words that available in dictionary
             */
            DictionaryInfo(const QString &plugin, const QString &name, const QString &author = QString(), 
                            const QString &description = QString(), long wordsCount = -1L);
            ~DictionaryInfo();

            const QString &plugin() const;

            const QString &name() const;

            const QString &author() const;

            const QString &description() const;

            long wordCount() const;

            void setPlugin(const QString &plugin);

            void setName(const QString &name);

            void setAuthor(const QString &author);

            void setDescription(const QString &description);

            void setWordCount(long wordCount);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_CORE_DICTIONARYINFO_H

