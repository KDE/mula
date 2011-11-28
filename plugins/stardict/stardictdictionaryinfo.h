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

#ifndef MULA_PLUGIN_STARDICT_STARDICTDICTIONARYINFO_H
#define MULA_PLUGIN_STARDICT_STARDICTDICTIONARYINFO_H

#include <QtCore/QString>
#include <QtCore/QDateTime>

namespace MulaPluginStarDict
{
    /**
     * \brief Collection of all the information about the dictionary
     *
     * The ".ifo" file has the following format:
     *
     * StarDict's dict ifo file
     * version=2.4.2
     * [options]
     *
     * Note that the current "version" string must be "2.4.2" or "3.0.0".  If
     * it's not, then StarDict will refuse to read the file.
     * If version is "3.0.0", StarDict will parse the "idxoffsetbits" option.
     *
     * [options]
     * ---------
     * In the example above, [options] expands to any of the following lines
     * specifying information about the dictionary.  Each option is a keyword
     * followed by an equal sign, then the value of that option, then a
     * newline. The options may appear in any order.
     *
     * Note that the dictionary must have at least a bookname, a wordcount and a
     * idxfilesize, otherwise the load will fail. All other information is
     * optional. All the string should be encoded in UTF-8.
     *
     * Available options:
     *
     * bookname=            // required
     * wordcount=           // required
     * synwordcount=        // required if ".syn" file exists.
     * idxfilesize=         // required
     * idxoffsetbits=       // New in 3.0.0
     * author=
     * email=
     * website=
     * description=         // You can use <br> for new line.
     * date=
     * sametypesequence=    // very important.
     *
     * wordcount is the count of the word entries in .idx file. It must be right.
     *
     * idxfilesize is the size (in bytes) of the ".idx" file, even the ".idx" is
     * compressed into a ".idx.gz" file. This entry must record the original size
     * of the ".idx" file. It must also be right. The ".gz" file does not contain
     * its original size information. However, knowing the original size can
     * speed up the extraction into the memory since realloc() does not need to
     * be called many times.
     *
     * The value of the idxoffsetbits can be 64 or 32. If "idxoffsetbits=64",
     * the offset field of the ".idx" file will be equal to 64 bits.
     *
     * The "sametypesequence" option is described in further detail below.
     *
     ****
     * sametypesequence
     *
     * You should first familiarize yourself with the ".dict" file format
     * described in the relevant section. After that, you can understand what
     * effect this option has on the ".dict" file.
     *
     * If the sametypesequence option is set, it tells StarDict that each
     * word data in the ".dict" file will have the same sequence of datatypes.
     * In this case, we expect a ".dict" file that hass been optimized in two
     * ways: the type identifiers should be omitted, so should the size marker
     * be omitted for the last data entry of each word.
     *
     * Let us consider some concrete examples of the sametypesequence option.
     *
     * Suppose that a dictionary records many ".wav" files, and such it sets:
     * sametypesequence=W
     * In this case, each word entry in the .dict file consists solely of a
     * wav file. The 'W' character would be left out in the ".dict" file
     * before each entry, and you would also omit the 32-bits integer at the
     * front of each ".wav" entry that would normally give the length of the
     * entry. You can do this since the length is known from the information in
     * the idx file.
     *
     * As another example, suppose a dictionary contains phonetic information
     * and a meaning for each word. The sametypesequence option for this
     * dictionary would be:
     * sametypesequence=tm
     * Once again, you can omit the 't' and 'm' characters before each data
     * entry in the ".dict" file. In addition, you should omit the terminating
     * '\0' for the 'm' entry for each word in the .dict file, as the length
     * of the meaning string can be inferred from the length of the phonetic
     * string (still indicated by a terminating '\0') and the length of the
     * entire word entry (listed in the ".idx" file).
     *
     * So for cases where the last data entry for each word normally requires
     * a terminating '\0' character. That character should be omitted in the
     * dict file. In those cases, where the last data entry for each word
     * normally requires an initial 32-bits number giving the length of the
     * field (such as WAV and PNG entries), you must omit this number in the
     * dictionary.
     *
     * Every dictionary should try to use the sametypesequence feature to
     * save disk space.
     * ***
     *
     * \see Indexfile
     */

    class StarDictDictionaryInfo
    {
        public:
            /**
             * Constructor
             */
            StarDictDictionaryInfo();

            /**
             * Destructor
             */
            virtual ~StarDictDictionaryInfo();

            /**
             * Loads all the information from the relevant ifo file considering
             * the fact whether or not it is a tree dictionary.
             *
             * @param ifoFilePath       The path of the ifo file
             * @param isTreeDictionary  Whether or not it is a tree dictionary
             *
             * @return True if the ifo file load was successful, otherwise
             * false.
             *
             * @see setIfoFilePath, ifoFilePath
             */
            bool loadFromIfoFile(const QString& ifoFileName, bool isTreeDictionary = false);

            /**
             * Sets the path of the ifo file
             *
             * @param ifoFilePath The path of the ifo file
             *
             * @see ifoFilePath, loadFromIfoFile
             */
            void setIfoFilePath(const QString& ifoFilePath);

            /**
             * Returns the path of the ifo file
             *
             * @return The path of the ifo file
             *
             * @see setIfoFilePath, loadFromIfoFile
             */
            QString ifoFilePath() const;

            /**
             * Sets the count of the word entries in the ".idx" file.
             *
             * @param wordCount The count of the word entries
             *
             * @see wordCount
             */
            void setWordCount(quint32 wordCount);

            /**
             * Returns the count of the word entries in the .idx file.
             *
             * @return The count of the word entries
             *
             * @see setWordCount
             */
            quint32 wordCount() const;

            /**
             * Sets the name of the book
             *
             * @param bookName The name of the book
             *
             * @see bookName
             */
            void setBookName(const QString& bookName);

            /**
             * Returns the name of the book
             *
             * @return The string value of the book name
             *
             * @see setBookName
             */
            QString bookName() const;

            /**
             * Sets the name of the dictionary author
             *
             * @param author The name of the author
             *
             * @see author
             */
            void setAuthor(const QString& author);

            /**
             * Returns the name of the dictionary author
             *
             * @return The name of the dictionary author
             *
             * @see setAuthor
             */
            QString author() const;

            /**
             * Sets the email address
             *
             * @param author The email address
             *
             * @see email
             */
            void setEmail(const QString& email);

            /**
             * Returns the email address
             *
             * @return The email address
             *
             * @see setEmail
             */
            QString email() const;

            /**
             * Sets the website of the dictionary
             *
             * @param website The website
             *
             * @see website
             */
            void setWebsite(const QString website);

            /**
             * Returns the website of the dictionary
             *
             * @return The website of the dictionary
             *
             * @see setWebsite
             */
            QString website() const;

            /**
             * Sets the date and time of the dictionary
             *
             * @param dateTime The date and time
             *
             * @see dateTime
             */
            void setDateTime(const QString& dateTime);

            /**
             * Returns the date and time of the dictionary
             *
             * @return The date and time  of the dictionary
             *
             * @see setDateTime
             */
            QString dateTime() const;

            /**
             * Sets the description of the dictionary. "<br>" can be used for
             * new lines.
             *
             * @param description The description
             *
             * @see description
             */
            void setDescription(const QString& description);

            /**
             * Returns the description of the dictionary. The return value can
             * also contain "<br>" for new lines and time of the dictionary
             *
             * @return The description of the dictionary
             *
             * @see setDescription
             */
            QString description() const;

            /**
             * Sets the value of the index file size.
             *
             * @param indexFileSize The index file size.
             *
             * @see indexFileSize
             */
            void setIndexFileSize(quint32 indexFileSize);

            /**
             * Returns the value of the index file size.
             *
             * @return The index file size
             *
             * @see setIndexFileSize
             */
            quint32 indexFileSize() const;

            /**
             * Sets the value of the index offset bits
             *
             * @param indexOffsetBits The index offset bits
             *
             * @see indexOffsetBits
             */
            void setIndexOffsetBits(quint32 indexOffsetBits);

            /**
             * Returns the value of the index offset bits.
             *
             * @return The value of the index offset bits.
             *
             * @see setIndexOffsetBits
             */
            quint32 indexOffsetBits() const;

            /**
             * Sets the value of the same type sequence
             *
             * @param sameTypeSequence The same type sequence
             *
             * @see sameTypeSequence
             */
            void setSameTypeSequence(const QString& sameTypeSequence);

            /**
             * Returns the value of the same type sequence
             *
             * @return The value of the same typesequence
             *
             * @see setSameTypeSequence
             */
            QString sameTypeSequence() const;

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_STARDICTDICTIONARYINFO_H
