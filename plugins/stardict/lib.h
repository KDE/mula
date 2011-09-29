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

#ifndef MULA_PLUGIN_STARDICT_LIB_H
#define MULA_PLUGIN_STARDICT_LIB_H

#include "dictionaryzip.h"

#include <QtCore/QList>
#include <QtCore/QStringList>

const int MAX_MATCH_ITEM_PER_LIB = 100;
const int MAX_FUZZY_DISTANCE = 3; // at most MAX_FUZZY_DISTANCE-1 differences allowed when find similar words

namespace MulaPluginStarDict
{
    class Dictionary;
    class Libs
    {
        public:
            enum QueryType {
                SIMPLE,
                REGEXP,
                FUZZY,
                DATA,
            };

            typedef void (*progress_func_t)(void);

            Libs(progress_func_t f = NULL);
            virtual ~Libs();

            void loadDictionary(const QString& url);
            void load(const QStringList& dictionaryDirs,
                      const QStringList& orderList,
                      const QStringList& disableList);
            void reload(const QStringList& dictionaryDirs,
                        const QStringList& orderList,
                        const QStringList& disableList);

            long articleCount(int index) const;

            const QString& dictionaryName(int index) const;

            int dictionaryCount() const;

            const QByteArray poWord(long iIndex, int iLib) const;

            QString poWordData(long iIndex, int iLib);

            QByteArray poCurrentWord(long *iCurrent);
            QByteArray poNextWord(QByteArray searchWord, long *iCurrent);
            QByteArray poPreviousWord(long *iCurrent);

            bool lookupPattern(QString searchWord, int dictionaryIndex, QString suffix, int wordLength, int truncateLength, QString addition, bool check);
            bool lookupWord(const char* sWorda, long& iWordIndex, int iLib);

            Dictionary *reloaderFind(const QString& url);
            void reloaderHelper(const QString &absolutePath);
            template <typename Method> void recursiveTemplateHelper(const QString& directoryName, const QStringList& orderList, const QStringList& disableList, Method method);

            bool lookupSimilarWord(QByteArray searchWord, long& iWordIndex, int iLib);
            bool simpleLookupWord(QByteArray searchWord, long& iWordIndex, int iLib);

            bool lookupWithFuzzy(QByteArray searchWord, QStringList resultList, int resultListSize, int iLib);
            int lookupWithRule(QByteArray searchWord, QStringList resultList);
            bool lookupData(QByteArray searchWord, QStringList resultList);

            QueryType analyzeQuery(QString string, QString& result);

        private:
            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_LIB_H
