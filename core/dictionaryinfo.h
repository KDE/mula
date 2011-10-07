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

#include "mula_core_export.h"

#include <QtCore/QString>

namespace MulaCore
{
    /**
     * This class represents information about dictionary.
     */
    class MULA_CORE_EXPORT DictionaryInfo
    {
        public:
            /**
             * Construct an empty object.
             */
            DictionaryInfo();

            /**
             * Construct an object from the desired data.
             * @param plugin A plugin name
             * @param name A dictionary name
             * @param author A dictionary author
             * @param description A dictionary description
             * @param wordsCount A count of words that available in dictionary
             */
            DictionaryInfo(const QString &plugin, const QString &name, const QString &author = QString(),
                            const QString &description = QString(), long wordsCount = -1L);


            virtual ~DictionaryInfo();

            /**
             * Returns the plugin name of the Dictionary
             *
             * @return The plugin name of the Dictionary
             * @see setPlugin
             */
            const QString &plugin() const;

            /**
             * Returns the name of the Dictionary
             *
             * @return The name of the Dictionary
             * @see setName
             */
            const QString &name() const;

            /**
             * Returns the author of the Dictionary
             *
             * @return The author of the Dictionary
             * @see setAuthor
             */
            const QString &author() const;

            /**
             * Returns the description of the Dictionary
             *
             * @return The description of the Dictionary
             * @see setDescription
             */
            const QString &description() const;

            /**
             * Returns the word count of the Dictionary
             *
             * @return The word count of the Dictionary
             * @see setWordCount
             */
            long wordCount() const;

            /**
             * Set the plugin name of the Dictionary
             *
             * @param plugin The plugin name of the Dictionary
             * @see plugin
             */
            void setPlugin(const QString &plugin);

            /**
             * Set the name of the Dictionary
             *
             * @param author The name of the Dictionary
             * @see name
             */
            void setName(const QString &name);

            /**
             * Set the author of the Dictionary
             *
             * @param author The author of the Dictionary
             * @see author
             */
            void setAuthor(const QString &author);

            /**
             * Set the description of the Dictionary
             *
             * @param description The description of the Dictionary
             * @see description
             */
            void setDescription(const QString &description);

            /**
             * Set the word count of the Dictionary
             *
             * @param wordCount The word count of the Dictionary
             * @see wordCount
             */
            void setWordCount(long wordCount);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_CORE_DICTIONARYINFO_H

