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

#include "wordentry.h"

#include <QtCore/QString>

namespace MulaPluginStarDict
{
    class Dictionary : public AbstractDictionary
    {
        public:

            /**
             * Constructor
             */

            Dictionary();

            /**
             * Destructor
             */

            virtual ~Dictionary();

            /**
             * Loads the ".ifo" file
             *
             * @param ifoFilePath Path of the ".ifo" file
             *
             * @return True if the loading was successful, otherwise false.
             */

            bool load(const QString& ifoFilePath);

            /**
             * Returns the count of the word entries in the ".idx" file.
             *
             * @return The count of the word entries
             */

            int articleCount() const;

            /**
             * Returns the name of the dictionary
             *
             * @return The name of the dictionary
             */
            QString dictionaryName() const;

            /**
             * Returns the ".ifo" file path
             *
             * @return The ".ifo" file path
             *
             * @see loadIfoFile
             */

            QString ifoFilePath() const;

            /**
             * Returns the word data according to the relevant index
             *
             * \note Since it is an abstraction on top of index file,
             * this method does not care about the fact whether or not the word
             * is returned directly, or use some caching mechanism or something
             * else.
             *
             * @param   index   The index of the desired word
             *
             * @return  The desired word data
             *
             * @see data, wordEntry
             */

            QString key(long index) const;

            /**
             * Returns the desired word data of the dictionary with all its
             * fields
             *
             * \note The method takes care about the low-level details of
             * the same type sequence settings in the index file and so forth.
             * If the indexfile is not loaded properly yet, this method returns
             * an empty string.
             *
             * @param   index   The index of the desired word
             *
             * @return The desired word data with all its fields
             *
             * @see key, wordEntry
             */

            QString data(long index);

            /**
             * Returns the word entry according to the desired index value
             *
             * \note If the index file is not loaded properly yet, this method
             * returns an empty entry object.
             *
             * @param index The index of the desired word entry
             *
             * @return The word entry according to the given index
             *
             * @see key, data
             */

            WordEntry wordEntry(long index);

            /**
             * Returns the index of the word data where it has been found. The
             * method also returns the mere fact whether or not the desired word
             * could be found in the index file.
             *
             * @param   word    The word data to look up
             * @param   index   The index where the desired word occurs among
             * the word entries.
             *
             * @return The index where the desired word occurs among the word
             * entries, or -1 if there is no such a word.
             */

            int lookup(const QString& word);

            /**
             * Returns the list of indices matched against the desired word data
             * pattern in the dictionary. Note, this method returns maximum
             * that many elements that is passed as an arguement. Alternatively,
             * if it is a big enough value, it will return all the words.
             *
             * @param   pattern                 The pattern to look up
             * @param   maximumIndexListSize    The maximum index list count for
             * returning
             *
             * @return The indices where the pattern matches against the word
             * data
             */

            QVector<int> lookupPattern(const QString& pattern, int maximumIndexListSize);

        private:
            /**
             * Loads the ".ifo" file while marking the dictionary non-tree one
             *
             * \note This method is only for internal usage.
             *
             * @param ifoFilePath Path of the ".ifo" file
             *
             * @return True if the loading was successful, otherwise false.
             *
             * @see load, ifoFilePath
             */

            bool loadIfoFile(const QString& ifoFilePath);

            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_DICTIONARY_H
