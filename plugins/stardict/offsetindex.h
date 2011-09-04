/******************************************************************************
 * This file is part of the MULA project
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

namespace MULAPluginStardict
{
    class OffsetIndex : public indexFile
    {
        public:
            offsetIndex();
            virtual ~offsetIndex();

            bool load(const QString& url, ulong wordCount, ulong fsize);
            const char *key(qlong index);

            void data(qlong index);
            const char *keyAndData(qlong index);
            bool lookup(const char *str, qlong &index);

        private:
            static const qint ENTR_PER_PAGE = 32;
            static const char *CACHE_MAGIC;

            QVector<quint32> wordOffset;
            QFile indexFile;
            ulong wordCount;

            char wordEntryBuf[256 + sizeof(quint32)*2]; // The length of "word_str" should be less than 256. See src/tools/DICTFILE_FORMAT.
            struct indexEntry
            {    
                qlong index;
                QString keyStr;
                void assign(qlong i, const QString& str) 
                {    
                    index = i; 
                    keystr.assign(str);
                }    
            };   
            indexEntry first, last, middle, realLast;

            struct pageEntry
            {    
                char *keyStr;
                quint32 off; 
                quint32 size;
            };   

            QByteArray pageData;
            struct page_t
            {    
                page_t()
                    :index(-1)
                {    
                }    

                void fill(gchar *data, gint nent, glong idx_);

                qlong index;
                pageEntry entries[ENTR_PER_PAGE];
            } page;

            ulong loadPage(qlong pageIndex);
            const char *readFirstOnPageKey(qlong pageIndex);
            const char *firstOnPageKey(qlong pageIndex);
            bool loadCache(const QString& url);
            bool saveCache(const QString& url);
            static QStringList cacheVariant(const QString& url);
    };
}

#endif // MULA_PLUGIN_STARDICT_OFFSETINDEX_H
