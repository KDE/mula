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
    /** 
     * \brief Represents the ".dict" file format. The .dict file is a pure data
     * sequence, as the offset and size of each word is recorded in the
     * corresponding .idx file
     *
     * If the "sametypesequence" option is not used in the .ifo file, then
     * the .dict file has fields in the following order:
     * ==============
     * word_1_data_1_type; // a single char identifying the data type
     * word_1_data_1_data; // the data
     * word_1_data_2_type;
     * word_1_data_2_data;
     * ...... // the number of data entries for each word is determined by
     *        // word_data_size in .idx file
     * word_2_data_1_type;
     * word_2_data_1_data;
     * ......
     * ==============
     * It's important to note that each field in each word indicates its
     * own length, as described below.  The number of possible fields per
     * word is also not fixed, and is determined by simply reading data until
     * you've read word_data_size bytes for that word.
     * 
     * 
     * Suppose the "sametypesequence" option is used in the .idx file, and
     * the option is set like this:
     * sametypesequence=tm
     * Then the .dict file will look like this:
     * ==============
     * word_1_data_1_data
     * word_1_data_2_data
     * word_2_data_1_data
     * word_2_data_2_data
     * ......
     * ==============
     * The first data entry for each word will have a terminating '\0', but
     * the second entry will not have a terminating '\0'.  The omissions of
     * the type chars and of the last field's size information are the
     * optimizations required by the "sametypesequence" option described
     * above.
     * 
     * If "idxoffsetbits=64", the file size of the .dict file will be bigger 
     * than 4G. Because we often need to mmap this large file, and there is 
     * a 4G maximum virtual memory space limit in a process on the 32 bits 
     * computer, which will make we can get error, so "idxoffsetbits=64" 
     * dictionary can't be loaded in 32 bits machine in fact, StarDict will 
     * simply print a warning in this case when loading. 64-bits computers 
     * should haven't this limit.
     * 
     * Type identifiers
     * ----------------
     * Here are the single-character type identifiers that may be used with
     * the "sametypesequence" option in the .idx file, or may appear in the
     * dict file itself if the "sametypesequence" option is not used.
     * 
     * Lower-case characters signify that a field's size is determined by a
     * terminating '\0', while upper-case characters indicate that the data
     * begins with a network byte-ordered guint32 that gives the length of 
     * the following data's size(NOT the whole size which is 4 bytes bigger).
     *
     *'m'
     * Word's pure text meaning.
     * The data should be a utf-8 string ending with '\0'.
     * 
     * 'l'
     * Word's pure text meaning.
     * The data is NOT a utf-8 string, but is instead a string in locale
     * encoding, ending with '\0'.  Sometimes using this type will save disk
     * space, but its use is discouraged.
     * 
     * 'g'
     * A utf-8 string which is marked up with the Pango text markup language.
     * For more information about this markup language, See the "Pango
     * Reference Manual."
     * You might have it installed locally at:
     * file:///usr/share/gtk-doc/html/pango/PangoMarkupFormat.html
     * 
     * 't'
     * English phonetic string.
     * The data should be a utf-8 string ending with '\0'.
     * 
     * Here are some utf-8 phonetic characters:
     * θʃŋʧðʒæıʌʊɒɛəɑɜɔˌˈːˑṃṇḷ
     * æɑɒʌәєŋvθðʃʒɚːɡˏˊˋ
     * 
     * 'x'
     * A utf-8 string which is marked up with the xdxf language.
     * See http://xdxf.sourceforge.net
     * StarDict have the following extensions:
     * <rref> can have "type" attribute, it can be "image", "sound", "video" 
     * and "attach".
     * <kref> can have "k" attribute.
     * 
     * 'y'
     * Chinese YinBiao or Japanese KANA.
     * The data should be a utf-8 string ending with '\0'.
     * 
     * 'k'
     * KingSoft PowerWord's data. The data is a utf-8 string ending with '\0'.
     * It is in XML format.
     * 
     * 'w'
     * MediaWiki markup language.
     * See http://meta.wikimedia.org/wiki/Help:Editing#The_wiki_markup
     * 
     * 'h'
     * Html codes.
     * 
     * 'r'
     * Resource file list.
     * The content can be:
     * img:pic/example.jpg     // Image file
     * snd:apple.wav           // Sound file
     * vdo:film.avi            // Video file
     * att:file.bin            // Attachment file
     * More than one line is supported as a list of available files.
     * StarDict will find the files in the Resource Storage.
     * The image will be shown, the sound file will have a play button.
     * You can "save as" the attachment file and so on.
     * 
     * 'W'
     * wav file.
     * The data begins with a network byte-ordered guint32 to identify the wav
     * file's size, immediately followed by the file's content.
     * 
     * 'P'
     * Picture file.
     * The data begins with a network byte-ordered guint32 to identify the picture
     * file's size, immediately followed by the file's content.
     * 
     * 'X'
     * this type identifier is reserved for experimental extensions.
     *
     * \see Indexfile
     */

    class AbstractDictionary
    {
        public:
            /**
             * Constructor
             */
            AbstractDictionary();

            /**
             * Destructor
             */
            virtual ~AbstractDictionary();

            /**
             * Returns the desired word data of the dictionary with all its
             * fields
             *
             * \note The method takes care about the low-level the details of
             * the same type sequence settings in the index file.
             *
             * @param indexItemOffset   The offset value in the dictionary file
             * @param indexItemSize     The size of the desired word data
             *
             * @return The desired word data with all its fields
             *
             * @see findData, containFindData
             */
            const QByteArray wordData(quint32 indexItemOffset, qint32 indexItemSize);

            /**
             * Returns whether the dictionary contains any of the given same
             * type sequence characters
             *
             * @return True if any of the given same type sequence characters is
             * contained by the dictionary, otherwise false.
             *
             * @see findData, wordData
             */
            bool containFindData();

            /**
             * Returns true if the dictionary contains all the desired words
             * according to the relevant offset and index item size, otherwise
             * false.
             *
             * @param searchWords       The desired words to look up
             * @param indexItemOffset   The index item offset
             * @param indexItemSize     The index item size
             *
             * @return True if all the desired words can be found in the
             * dictionary, otherwise false.
             *
             * @see containData, wordData
             */
            bool findData(const QStringList &searchWords, qint32 indexItemOffset, qint32 indexItemSize);

            /**
             * Returns the compressed ".dict.dz" dictionary file
             *
             * @return The compressed dictionary file
             *
             * @see dictionaryFile
             */
            DictionaryZip* compressedDictionaryFile() const;

            /**
             * Returns the ".dict" dictionary file
             *
             * @return The dictionary file
             *
             * @see compressedDictionaryFile
             */
            QFile* dictionaryFile() const;

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

#endif // MULA_PLUGIN_STARDICT_ABSTRACtDICTIONARY_H
