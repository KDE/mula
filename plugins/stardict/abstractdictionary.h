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

    class AbstractDictionary
    {
        public:
            AbstractDictionary();
            virtual ~AbstractDictionary();

            const QByteArray wordData(quint32 indexItemOffset, qint32 indexItemSize);
            bool containFindData();
            bool findData(const QStringList &searchWords, qint32 indexItemOffset, qint32 indexItemSize, QByteArray& originalData);

            DictionaryZip* compressedDictionaryFile() const;
            QFile* dictionaryFile() const;

            QString& sameTypeSequence() const;
            void setSameTypeSequence(const QString& sameTypeSequence);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_ABSTRACtDICTIONARY_H
