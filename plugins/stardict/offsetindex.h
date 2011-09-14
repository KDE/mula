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

#ifndef MULA_PLUGIN_STARDICT_OFFSETINDEX_H
#define MULA_PLUGIN_STARDICT_OFFSETINDEX_H

namespace MulaPluginStarDict
{
    class OffsetIndex : public IndexFile
    {
        public:
            OffsetIndex();
            virtual ~OffsetIndex();

            bool load(const QString& url, ulong wordCount, qulonglong sfile);
            const QString& key(ulong index);

            void data(ulong index);
            const QString& keyAndData(ulong index);
            bool lookup(const QString &string, ulong &index);

            virtual quint32 wordEntryOffset() const;
            virtual quint32 wordEntrySize() const;

        private:
            ulong loadPage(ulong pageIndex);
            const QString& readFirstOnPageKey(ulong pageIndex);
            const QString& firstOnPageKey(ulong pageIndex);
            bool loadCache(const QString& url);
            bool saveCache(const QString& url);
            static QStringList cacheVariant(const QString& url);
    };
}

#endif // MULA_PLUGIN_STARDICT_OFFSETINDEX_H
