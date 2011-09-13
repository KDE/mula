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

namespace MulaPluginStardict
{
    class Dictionary : public DictionaryBase
    {
        public:
            Dictionary() {}
            virtual ~Dictionary() {}

            bool load(const QString& ifoFileName);

            ulong articlesCount();

            const QString& dictionaryName();

            const QString& ifoFileName();

            const QString key(qlong index);

            QString data(qlong index);

            void keyAndData(qlong index, const QStringList key, quint32 *offset, quint32 *size);

            bool lookup(const QString str, qlong &index);

            bool lookupWithRule(GPatternSpec *pspec, glong *aIndex, int iBuffLen);

        private:
            bool loadIfoFile(const QString& ifoFileName, ulong &indexFileSize);

            QString m_ifoFileName;
            ulong m_wordCount;
            QString m_bookName;

            QSharedPointer<indexFile> m_indexFile;
    };
}

#endif // MULA_PLUGIN_STARDICT_DICTIONARY_H
