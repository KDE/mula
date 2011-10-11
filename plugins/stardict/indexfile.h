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

#ifndef MULA_PLUGIN_STARDICT_INDEXFILE_H
#define MULA_PLUGIN_STARDICT_INDEXFILE_H

#include "abstractindexfile.h"

namespace MulaPluginStarDict
{
    class IndexFile : public AbstractIndexFile
    {
        public:
            /**
             * Constructor.
             */
            IndexFile();

            /**
             * Destructor.
             */
            virtual ~IndexFile();

            /** Reimplemented from AbstractIndexFile::load() */
            bool load(const QString& filePath, qulonglong fileSize, int wordCount);

            /** Reimplemented from AbstractIndexFile::key() */
            QByteArray key(long index);

            /** Reimplemented from AbstractIndexFile::lookup() */
            bool lookup(const QByteArray& word, int &index);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_INDEXFILE_H
