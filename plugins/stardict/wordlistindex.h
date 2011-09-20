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

#ifndef MULA_PLUGIN_STARDICT_WORDLISTINDEX_H
#define MULA_PLUGIN_STARDICT_WORDLISTINDEX_H

#include "indexfile.h"

namespace MulaPluginStarDict
{
    class WordListIndex : public IndexFile
    {       
        public:
            WordListIndex();
            virtual ~WordListIndex();

            bool load(const QString& url, long wc, qulonglong sffile);
            QByteArray key(long index);
            void data(long index);
            QByteArray keyAndData(long index);
            bool lookup(const QByteArray& string, long &index);

            virtual quint32 wordEntryOffset() const;
            virtual void setWordEntryOffset(quint32 offset);

            virtual quint32 wordEntrySize() const;
            virtual void setWordEntrySize(quint32 size);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_WORDLISTINDEX_H
