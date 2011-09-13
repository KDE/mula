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

#ifndef MULA_PLUGIN_STARDICT_DICTIONARYINFO_H
#define MULA_PLUGIN_STARDICT_DICTIONARYINFO_H

#include <QtCore/QString>
#include <QtCore/QDateTime>

namespace MulaPluginStardict
{
    // This class contains all information about the dictionary
    // Note that the dictionary must have at least a bookname, a wordcount and a 
    // idxfilesize, or the load will fail.
    class DictonaryInfo
    {
        public:
            DictionaryInfo();
            virtual ~DictionaryInfo();

            bool loadFromIfoFile(const QString& ifoFileName, bool isTreeDict);

            void setIfoFileName(const QString& ifoFileName);
            QString ifoFileName() const;

            void setWordCount(quint32 wordCount);
            quint32 wordCount() const;

            void setBookName(const QString& bookname);
            QString bookname() const;

            void setAuthor(const QString& author);
            QString author() const

            void setEmail(const QString& email);
            QString email() const;

            void setWebsite(const QString website);
            QString website() const;

            void setDate(const QDateTime& dateTime);
            QDateTime dateTime() const;

            void setDescription(const QString& description);
            QString description() const;

            void setIndexFileSize(quint32 indexFileSize);
            quint32 indexFileSize() const;

            void setIndexOffsetBits(quint32 indexOffsetBits);
            quint32 indexOffsetBits() const;

            void setSameTypeSequence(const QString& sameTypeSequence);
            QString sameTypeSequence() const;

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_DICTIONARYINFO_H
