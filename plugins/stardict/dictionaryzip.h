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

#ifndef MULA_PLUGIN_STARDICT_DICTIONARYZIP_LIB
#define MULA_PLUGIN_STARDICT_DICTIONARYZIP_LIB

#include <QtCore/QString>

namespace MulaPluginStarDict
{
{
    /** 
     * \brief The class can be used for compressing the .dict file
     *
     * "dictzip" uses the same compression algorithm and file format as gzip, 
     * but provides a table that can be used to randomly access compressed blocks 
     * in the file. The use of 50-64kB blocks for compression typically degrades 
     * the compression by less than 10%, while maintaining acceptable random
     * access capabilities for all the data in the file. In addition, the files 
     * compressed by using dictzip can be decompressed by using gunzip.
     * For more information about dictzip, refer to DICT project, please see:
     * http://www.dict.org
     *
     * \see Indexfile
     */

#define DICT_CACHE_SIZE 5

    class DictionaryZip
    {
        public:
            DictionaryZip();
            virtual ~DictionaryZip();

            bool open(const QString& fileName, int computeCRC);
            void close();

            QByteArray read(unsigned long start, unsigned long size);

        private:
            int readHeader(const QString &filename, int computeCRC);

            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_DICTIONARYZIP_LIB
