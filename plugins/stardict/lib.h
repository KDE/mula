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

#ifndef MULA_PLUGIN_STARDICT_LIB_H
#define MULA_PLUGIN_STARDICT_LIB_H

#include "dictziplib.h"

#include <QtCore/QList>
#include <QtCore/QStringList>

const int MAX_MATCH_ITEM_PER_LIB = 100;
const int MAX_FUZZY_DISTANCE = 3; // at most MAX_FUZZY_DISTANCE-1 differences allowed when find similar words

struct cacheItem
{
    //write code here to make it inline
    cacheItem()
        : data(NULL)
        , offset(0)
    {
    }

    ~cacheItem()
    {
        free(data);
    }

    quint32 offset;
    char *data;
};

const int WORDDATA_CACHE_NUM = 10;
const int INVALID_INDEX = -100;

class Libs
{
    public:
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

        qlong articleCount(int idict);

        const QString& dictionaryName(int idict);

        qint dictionaryCount();

        const char *poWord(qlong iIndex, int iLib);

        QString poWordData(qlong iIndex, int iLib);

        const char *poCurrentWord(qlong *iCurrent);
        const char *poNextWord(const char *word, qlong *iCurrent);
        const char *poPreviousWord(qlong *iCurrent);

        bool lookupWord(const char* sWorda, qlong& iWordIndex, int iLib);

        bool lookupSimilarWord(const char* sWord, glong & iWordIndex, int iLib);
        bool simpleLookupWord(const char* sWord, glong & iWordIndex, int iLib);

        bool lookupWithFuzzy(const char *sWord, QStringList reslist, qint reslistSize, gint iLib);
        qint lookupWithRule(const char *sWord, QStringList reslist);
        bool lookupData(const char *sWord, QStringList reslist);

    private:
        class Private;
        Private *const d;
};

enum QueryType {
    Simple,
    Regexp,
    Fuzzy,
    Data
};

extern QueryType analyzeQuery(const char *s, QString& res);

#endif // MULA_PLUGIN_STARDICT_LIB_H
