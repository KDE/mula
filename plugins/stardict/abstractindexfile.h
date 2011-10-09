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

#ifndef MULA_PLUGIN_STARDICT_ABSTRACTINDEXFILE_H
#define MULA_PLUGIN_STARDICT_ABSTRACTINDEXFILE_H

#include <QtCore/QtGlobal>

class QString;

namespace MulaPluginStarDict
{
    class AbstractIndexFile
    {
        public:
            /**
             * Constructor.
             */
            AbstractIndexFile();

            /**
             * Destructor.
             */
            virtual ~AbstractIndexFile();

            virtual bool load(const QString& url, qulonglong fileSize, int wc) = 0;

            /**
             * Returns the word data according to the relevant index
             *
             * \note Since it it an abstract base class, the real
             * implementation is up to the successors whether to return the
             * word directly, use some caching mechanism or something else.
             *
             * @param   index   The index of the desired word
             * @return  The word data
             */
            virtual QByteArray key(long index) = 0;

            virtual bool lookup(const QByteArray& string, long &index) = 0;

            virtual quint32 wordEntryOffset() const;
            virtual void setWordEntryOffset(quint32 wordEntryOffset);

            virtual quint32 wordEntrySize() const;
            virtual void setWordEntrySize(quint32 wordEntrySize);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_ABSTRACTINDEXFILE_H
