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

#ifndef MULA_PLUGIN_STARDICT_DICTIONARYMANAGER_H
#define MULA_PLUGIN_STARDICT_DICTIONARYMANAGER_H

#include "dictionaryzip.h"

#include <QtCore/QList>
#include <QtCore/QStringList>

namespace MulaPluginStarDict
{
    class Dictionary;
    class StarDictDictionaryManager
    {
        public:
            enum QueryType {
                SIMPLE,
                REGEXP,
                FUZZY,
                DATA,
            };

            typedef void (*progress_func_t)(void);

            /**
             * Constructor.
             */
            StarDictDictionaryManager(progress_func_t f = NULL);

            /**
             * Destructor.
             */
            virtual ~StarDictDictionaryManager();

            void loadDictionary(const QString& directoryFilePath);

            void load(const QStringList& dictionaryDirs,
                      const QStringList& orderList,
                      const QStringList& disableList);

            void reload(const QStringList& dictionaryDirs,
                        const QStringList& orderList,
                        const QStringList& disableList);

            /**
             * Returns the count of the word entries in the .idx file at the
             * given index of the dictionary list
             *
             * @param index The desired index in the list
             * @return The count of the word entries
             */
            long articleCount(int index) const;

            /**
             * Returns the name of the dictionary at the given index of the
             * dictionary list
             *
             * @param index The desired index in the list
             * @return The name of the dictionary
             */
            const QString dictionaryName(int index) const;

            int dictionaryCount() const;

            const QByteArray poWord(long iIndex, int iLib) const;

            QString poWordData(long iIndex, int iLib);

            QByteArray poCurrentWord(int *iCurrent);
            QByteArray poNextWord(QByteArray searchWord, int* iCurrent);
            QByteArray poPreviousWord(long *iCurrent);

            bool lookupPattern(QString searchWord, int dictionaryIndex, QString suffix, int wordLength, int truncateLength, QString addition, bool check);
            bool lookupWord(const char* sWorda, int& iWordIndex, int iLib);

            Dictionary *reloaderFind(const QString& url);
            void reloaderHelper(const QString &absolutePath);
            template <typename Method> void recursiveTemplateHelper(const QString& directoryName, const QStringList& orderList, const QStringList& disableList, Method method);

            bool lookupSimilarWord(QByteArray searchWord, int& iWordIndex, int iLib);
            bool simpleLookupWord(QByteArray searchWord, int& iWordIndex, int iLib);

            bool lookupWithFuzzy(QByteArray searchWord, QStringList resultList, int resultListSize, int iLib);
            int lookupPattern(QByteArray searchWord, QStringList resultList);
            bool lookupData(QByteArray searchWord, QStringList resultList);

            QueryType analyzeQuery(QString string, QString& result);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_DICTIONARYMANAGER_H
