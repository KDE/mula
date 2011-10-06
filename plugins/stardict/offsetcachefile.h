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

#ifndef MULA_PLUGIN_STARDICT_OFFSETCACHEFILE_H
#define MULA_PLUGIN_STARDICT_OFFSETCACHEFILE_H

#include "abstractindexfile.h"

#include <QtCore/QStringList>

namespace MulaPluginStarDict
{
    class OffsetCacheFile : public AbstractIndexFile
    {
        public:
            OffsetCacheFile();
            virtual ~OffsetCacheFile();

            bool load(const QString& url, int wordCount, qulonglong fileSize);
            QByteArray key(long index);

            bool lookup(const QByteArray& string, long &index);

            virtual quint32 wordEntryOffset() const;
            virtual void setWordEntryOffset(quint32 wordEntryOffset);

            virtual quint32 wordEntrySize() const;
            virtual void setWordEntrySize(quint32 wordEntrySize);

        private:
            ulong loadPage(long pageIndex);
            QByteArray readFirstOnPageKey(long pageIndex);
            QByteArray firstOnPageKey(long pageIndex);
            bool loadCache(const QString& url);
            bool saveCache(const QString& url);
            static QStringList cacheLocations(const QString& url);

            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_OFFSETCACHEFILE_H
