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

#include "stardictdictionarymanager.h"

#include "distance.h"
#include "dictionary.h"
#include "file.h"

#include <QtCore/QtAlgorithms>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <zlib.h>

using namespace MulaPluginStarDict;

// Notice: read src/tools/DICTFILE_FORMAT for the dictionary
// file's format information!

static bool
isVowel(QChar inputChar)
{
    QChar ch = inputChar.toLower();
    return ( ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' );
}

static bool
isPureEnglish(const QByteArray& word)
{
    foreach (char c, word)
    {
        //if(str[i]<0)
        //if(str[i]<32 || str[i]>126) // tab equal 9,so this is not OK.
        // Better use isascii() but not str[i]<0 while char is default unsigned in arm
        if (!isascii(c))
            return false;
    }

    return true;
}

class StarDictDictionaryManager::Private
{
    public:
        Private()
           : found(false)
        {
        }

        ~Private()
        {
        }

        QVector<Dictionary *> dictionaries;
        progress_func_t progressFunction;

        QVector<Dictionary *> previous;
        QVector<Dictionary *> future;

        bool found;
        static const int maxMatchItemPerLib = 100;
        static const int maximumFuzzyDistance = 3; // at most MAX_FUZZY_DISTANCE-1 differences allowed when find similar words

};

StarDictDictionaryManager::StarDictDictionaryManager(progress_func_t progressFunction)
    : d(new Private)
{
    d->progressFunction = progressFunction;
}

StarDictDictionaryManager::~StarDictDictionaryManager()
{
    qDeleteAll(d->dictionaries);
    delete d;
}

void StarDictDictionaryManager::loadDictionary(const QString& url)
{
    Dictionary *dictionary = new Dictionary;
    if (dictionary->load(url))
    {
        d->dictionaries.append(dictionary);
    }
    else
    {
        qDebug() << "Could not load the dictionary:" << url;
        delete dictionary;
    }
}

long
StarDictDictionaryManager::articleCount(int index) const
{
    return d->dictionaries.at(index)->articleCount();
}

const QString
StarDictDictionaryManager::dictionaryName(int index) const
{
    return d->dictionaries.at(index)->dictionaryName();
}

int
StarDictDictionaryManager::dictionaryCount() const
{
    return d->dictionaries.size();
}

const QByteArray
StarDictDictionaryManager::poWord(long keyIndex, int libIndex) const
{
    return d->dictionaries.at(libIndex)->key(keyIndex).toUtf8();
}

QString
StarDictDictionaryManager::poWordData(long dataIndex, int libIndex)
{
    if (libIndex == invalidIndex)
        return NULL;

    return d->dictionaries.at(libIndex)->data(dataIndex);
}

bool
StarDictDictionaryManager::lookupWord(const char* searchWord, int& iWordIndex, int dictionaryIndex)
{
    return d->dictionaries.at(dictionaryIndex)->lookup(searchWord, iWordIndex);
}

template <typename Method>
void
StarDictDictionaryManager::recursiveTemplateHelper(const QString& directoryName, const QStringList& orderList, const QStringList& disableList,
                        Method method)
{
    QDir dir(directoryName);

    // Going through the subfolders
    foreach (const QString& entryName, dir.entryList(QDir::Dirs & QDir::NoDotAndDotDot))
    {
        QString absolutePath = dir.absoluteFilePath(entryName);
        recursiveTemplateHelper(absolutePath, orderList, disableList, method);
    }

    foreach (const QString& entryName, dir.entryList(QDir::Files & QDir::Drives & QDir::NoDotAndDotDot))
    {
        QString absolutePath = dir.absoluteFilePath(entryName);
        if (absolutePath.endsWith(QLatin1String(".ifo"))
                && qFind(orderList.begin(), orderList.end(), absolutePath) == orderList.end()
                && qFind(disableList.begin(), disableList.end(), absolutePath) == disableList.end())
        {
                (this->*method)(absolutePath);
        }
    }
}

void
StarDictDictionaryManager::load(const QStringList& dictionaryDirectoryList,
                                const QStringList& orderList,
                                const QStringList& disableList)
{
    foreach (const QString& string, orderList)
    {
        if (qFind(disableList.begin(), disableList.end(), string) == disableList.end())
            loadDictionary(string);
    }

    foreach (const QString& directoryName, dictionaryDirectoryList)
        recursiveTemplateHelper(directoryName, orderList, disableList, &StarDictDictionaryManager::loadDictionary);
}

Dictionary*
StarDictDictionaryManager::reloaderFind(const QString& url)
{
    QVector<Dictionary *>::iterator it;
    for (it = d->previous.begin(); it != d->previous.end(); ++it)
    {
        if ((*it)->ifoFilePath() == url)
            break;
    }

    if (it != d->previous.end())
    {
        Dictionary *result = *it;
        d->previous.erase(it);
        return result;
    }

    return NULL;
}

void
StarDictDictionaryManager::reloaderHelper(const QString &absolutePath)
{
    Dictionary *dictionary = reloaderFind(absolutePath);
    if (dictionary)
        d->future.append(dictionary);
    else
        loadDictionary(absolutePath);
}

void
StarDictDictionaryManager::reload(const QStringList& dictionaryDirectoryList,
                  const QStringList& orderList,
                  const QStringList& disableList)
{
    d->previous = d->dictionaries;
    d->dictionaries.clear();

    foreach (const QString& string, orderList)
    {
        if (qFind(disableList.begin(), disableList.end(), string) == disableList.end())
            reloaderHelper(string);
    }

    foreach (const QString& directoryName, dictionaryDirectoryList)
        recursiveTemplateHelper(directoryName, orderList, disableList, &StarDictDictionaryManager::reloaderHelper);

    qDeleteAll(d->previous);
}

QByteArray
StarDictDictionaryManager::poCurrentWord(int *iCurrent)
{
    const char *poCurrentWord = NULL;
    const char *word;

    for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        if (iCurrent[iLib] == invalidIndex)
            continue;

        if (iCurrent[iLib] >= articleCount(iLib) || iCurrent[iLib] < 0)
            continue;

        if (poCurrentWord == NULL )
        {
            poCurrentWord = poWord(iCurrent[iLib], iLib);
        }
        else
        {
            word = poWord(iCurrent[iLib], iLib);

            if (stardictStringCompare(poCurrentWord, word) > 0 )
                poCurrentWord = word;
        }
    }
    return poCurrentWord;
}

QByteArray
StarDictDictionaryManager::poNextWord(QByteArray searchWord, int* iCurrent)
{
    // the input can be:
    // (word,iCurrent),read word,write iNext to iCurrent,and return next word. used by TopWin::NextCallback();
    // (NULL,iCurrent),read iCurrent,write iNext to iCurrent,and return next word. used by AppCore::ListWords();
    QByteArray currentWord = NULL;
    QVector<Dictionary *>::size_type iCurrentLib = 0;
    const char *word;

    for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        if (!searchWord.isEmpty())
            d->dictionaries.at(iLib)->lookup(searchWord, iCurrent[iLib]);

        if (iCurrent[iLib] == invalidIndex)
            continue;

        if (iCurrent[iLib] >= articleCount(iLib) || iCurrent[iLib] < 0)
            continue;

        if (currentWord.isNull())
        {
            currentWord = poWord(iCurrent[iLib], iLib);
            iCurrentLib = iLib;
        }
        else
        {
            word = poWord(iCurrent[iLib], iLib);

            if (stardictStringCompare(currentWord, word) > 0 )
            {
                currentWord = word;
                iCurrentLib = iLib;
            }
        }
    }

    if (!currentWord.isEmpty())
    {
        iCurrent[iCurrentLib]
        ++;
        for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
        {
            if (iLib == iCurrentLib)
                continue;

            if (iCurrent[iLib] == invalidIndex)
                continue;

            if ( iCurrent[iLib] >= articleCount(iLib) || iCurrent[iLib] < 0)
                continue;

            if (strcmp(currentWord, poWord(iCurrent[iLib], iLib)) == 0 )
                iCurrent[iLib]++;
        }

        currentWord = poCurrentWord(iCurrent);
    }
    return currentWord;
}

QByteArray
StarDictDictionaryManager::poPreviousWord(long *iCurrent)
{
    // used by TopWin::PreviousCallback(); the iCurrent is cached by AppCore::TopWinWordChange();
    QByteArray poCurrentWord = NULL;
    QVector<Dictionary *>::size_type iCurrentLib = 0;
    const char *word;

    for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        if (iCurrent[iLib] == invalidIndex)
            iCurrent[iLib] = articleCount(iLib);
        else
        {
            if ( iCurrent[iLib] > articleCount(iLib) || iCurrent[iLib] <= 0)
                continue;
        }

        if ( poCurrentWord.isNull() )
        {
            poCurrentWord = poWord(iCurrent[iLib] - 1, iLib);
            iCurrentLib = iLib;
        }
        else
        {
            word = poWord(iCurrent[iLib] - 1, iLib);
            if (stardictStringCompare(poCurrentWord, word) < 0 )
            {
                poCurrentWord = word;
                iCurrentLib = iLib;
            }
        }
    }

    if (!poCurrentWord.isEmpty())
    {
        iCurrent[iCurrentLib]--;
        for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
        {
            if (iLib == iCurrentLib)
                continue;

            if (iCurrent[iLib] > articleCount(iLib) || iCurrent[iLib] <= 0)
                continue;

            if (poCurrentWord == poWord(iCurrent[iLib] - 1, iLib))
            {
                iCurrent[iLib]--;
            }
            else
            {
                if (iCurrent[iLib] == articleCount(iLib))
                    iCurrent[iLib] = invalidIndex;
            }
        }
    }
    return poCurrentWord;
}

bool
StarDictDictionaryManager::lookupPattern(QString searchWord, int dictionaryIndex, QString suffix, int wordLength, int truncateLength, QString addition, bool check)
{
        int searchWordLength = searchWord.length();
        if (!d->found && searchWordLength > wordLength)
        {
            QString caseString;
            int index;
            bool isUpperCase = searchWord.endsWith(suffix.toUpper());
            if (isUpperCase || searchWord.endsWith(suffix.toLower()))
            {
                QString searchNewWord = searchWord.left(searchWordLength - truncateLength);
                int searchNewWordLength = searchNewWord.length();
                if ( check && searchNewWordLength > 3 && (searchNewWord.at(searchNewWordLength - 1) == searchNewWord.at(searchNewWordLength - 2))
                        && !isVowel(searchNewWord.at(searchNewWordLength - 2)) && isVowel(searchNewWord.at(searchNewWordLength - 3)))
                {  //doubled
                    searchNewWord.remove(searchNewWordLength - 1, 1);
                    if (d->dictionaries.at(dictionaryIndex)->lookup(searchNewWord, index))
                    {
                        d->found = true;
                    }
                    else
                    {
                        if (isUpperCase || searchWord.at(0).isUpper())
                        {
                            caseString = searchNewWord.toLower();
                            if (caseString.compare(searchNewWord))
                            {
                                if (d->dictionaries.at(dictionaryIndex)->lookup(caseString, index))
                                    d->found = true;
                            }
                        }

                        if (!d->found)
                            searchNewWord.append(searchNewWord.at(searchNewWordLength - 1));  //restore
                    }
                }

                if (!d->found)
                {
                    if (d->dictionaries.at(dictionaryIndex)->lookup(searchNewWord, index))
                    {
                        d->found = true;
                    }
                    else if (isUpperCase || searchWord.at(0).isUpper())
                    {
                        caseString = searchNewWord.toLower();
                        if (caseString.compare(searchNewWord))
                        {
                            if (d->dictionaries.at(dictionaryIndex)->lookup(caseString, index))
                                d->found = true;
                        }
                    }
                }

                if (!d->found && !addition.isNull())
                {
                    if (isUpperCase)
                        searchNewWord.append(addition.toUpper());
                    else
                        searchNewWord.append(addition.toLower());

                    if (d->dictionaries.at(dictionaryIndex)->lookup(searchNewWord, index))
                    {
                        d->found = true;
                    }
                    else if (isUpperCase || searchWord.at(0).isUpper())
                    {
                        caseString = searchNewWord.toLower();
                        if (caseString.compare(searchNewWord))
                        {
                            if (d->dictionaries.at(dictionaryIndex)->lookup(caseString, index))
                                d->found = true;
                        }
                    }
                }
            }
        }
        return d->found;
}

bool
StarDictDictionaryManager::lookupSimilarWord(QByteArray searchWord, int& iWordIndex, int iLib)
{
    int iIndex;
    bool found = false;

    // Upper case lookup
    if (d->dictionaries.at(iLib)->lookup(searchWord.toUpper(), iIndex))
        found = true;

    // Lower case lookup
    if (!found && d->dictionaries.at(iLib)->lookup(searchWord.toLower(), iIndex))
        found = true;

    // Upper the first character and lower others
    if (!found && d->dictionaries.at(iLib)->lookup(QString(QString(searchWord)[0].toUpper()) + searchWord.mid(1).toLower(), iIndex))
        found = true;

    if (isPureEnglish(searchWord))
    {
        QString caseString;
        // If not Found, try other status of searchWord.
        int searchWordLength = searchWord.size();
        bool isUpperCase;
        QByteArray searchNewWord;

        lookupPattern(searchWord, iLib, "S", 1, 1, QString(), false);
        lookupPattern(searchWord, iLib, "ED", 1, 1, QString(), false);
        lookupPattern(searchWord, iLib, "LY", 2, 2, QString(), true);
        lookupPattern(searchWord, iLib, "ING", 3, 3, "E", true);


        if (!found && searchWordLength > 3)
        {
            isUpperCase = (searchWord.endsWith("ES") //krazy:exclude=strings
                        && (searchWord.at(searchWordLength - 3) == 'S'
                        || searchWord.at(searchWordLength - 3) == 'X'
                        || searchWord.at(searchWordLength - 3) == 'O'
                        || (searchWordLength > 4 && searchWord.at(searchWordLength - 3) == 'H'
                        && (searchWord.at(searchWordLength - 4) == 'C'
                        || searchWord.at(searchWordLength - 4) == 'S'))));

            if (isUpperCase ||
                    (searchWord.endsWith("es") //krazy:exclude=strings
                     && (searchWord.at(searchWordLength - 3) == 's'
                     || searchWord.at(searchWordLength - 3) == 'x'
                     || searchWord.at(searchWordLength - 3) == 'o'
                     || (searchWordLength > 4 && searchWord.at(searchWordLength - 3) == 'h'
                     && (searchWord.at(searchWordLength - 4) == 'c'
                     || searchWord.at(searchWordLength - 4) == 's')))))
            {
                searchNewWord = searchWord.left(searchWordLength - 2);
                if (d->dictionaries.at(iLib)->lookup(searchNewWord, iIndex))
                {
                    found = true;
                }
                else if (isUpperCase || QString::fromUtf8(searchWord).at(0).isUpper())
                {
                    caseString = searchNewWord.toLower();
                    if (caseString.compare(searchNewWord))
                    {
                        if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                            found = true;
                    }
                }
            }
        }

        lookupPattern(searchWord, iLib, "ED", 3, 2, QString(), true);
        lookupPattern(searchWord, iLib, "IED", 3, 3, "Y", false);
        lookupPattern(searchWord, iLib, "IES", 3, 3, "Y", false);
        lookupPattern(searchWord, iLib, "ER", 2, 3, QString(), false);
        lookupPattern(searchWord, iLib, "EST", 3, 3, QString(), false);

    }

    if (d->found)
        iWordIndex = iIndex;
#if 0
    else
    {
        //don't change iWordIndex here.
        //when LookupSimilarWord all failed too, we want to use the old LookupWord index to list words.
        //iWordIndex = invalidIndex;
    }
#endif
    return d->found;
}

bool
StarDictDictionaryManager::simpleLookupWord(QByteArray searchWord, int &iWordIndex, int iLib)
{
    bool found = d->dictionaries.at(iLib)->lookup(searchWord, iWordIndex);
    if (!found)
        found = lookupSimilarWord(searchWord, iWordIndex, iLib);

    return found;
}

struct
Fuzzystruct
{
    QByteArray pMatchWord;
    int matchWordDistance;
};

inline bool
operator<(const Fuzzystruct & lh, const Fuzzystruct & rh)
{
    if (lh.matchWordDistance != rh.matchWordDistance)
        return lh.matchWordDistance < rh.matchWordDistance;

    if (!lh.pMatchWord.isNull() && !rh.pMatchWord.isNull())
        return stardictStringCompare(lh.pMatchWord, rh.pMatchWord) < 0;

    return false;
}

bool
StarDictDictionaryManager::lookupWithFuzzy(QByteArray searchWord, QStringList resultList, int resultListSize, int iLib)
{
    if (searchWord.isEmpty())
        return false;

    Fuzzystruct *oFuzzystruct = new Fuzzystruct[resultListSize];

    for (int i = 0; i < resultListSize; ++i)
    {
        oFuzzystruct[i].pMatchWord = NULL;
        oFuzzystruct[i].matchWordDistance = d->maximumFuzzyDistance;
    }

    int maximumDistance = d->maximumFuzzyDistance;
    int iDistance;
    bool found = false;
    EditDistance oEditDistance;

    long searchCheckWordLength;
    long searchWordLength;
    QString searchCheckWord;

    searchWord.toLower();

    if (d->progressFunction)
        d->progressFunction();


    //if (stardict_strcmp(searchWord, poGetWord(0,iLib))>=0 && stardict_strcmp(searchWord, poGetWord(articleCount(iLib)-1,iLib))<=0) {
    //there are Chinese dicts and English dicts...
    if (true)
    {
        int wordNumber = articleCount(iLib);
        for (int index = 0; index < wordNumber; ++index)
        {
            searchCheckWord = poWord(index, iLib);
            // tolower and skip too long or too short words
            searchCheckWordLength = searchCheckWord.length();
            searchWordLength = searchWord.length();
            if (searchCheckWordLength - searchWordLength >= maximumDistance
                    || searchWordLength - searchCheckWordLength >= maximumDistance)
                continue;

            searchCheckWord.toLower();

            iDistance = oEditDistance.calEditDistance(searchCheckWord, searchWord, maximumDistance);
            if (iDistance < maximumDistance && iDistance < searchWordLength)
            {
                // when searchWordLength=1,2 we need less fuzzy.
                found = true;
                bool isAlreadyInList = false;
                int maximumDistanceAt = 0;
                for (int j = 0; j < resultListSize; ++j)
                {
                    if (!oFuzzystruct[j].pMatchWord.isNull() && searchCheckWord.compare(oFuzzystruct[j].pMatchWord) == 0)
                    { //already in list
                        isAlreadyInList = true;
                        break;
                    }

                    //find the position,it will certainly be found (include the first time) as iMaxDistance is set by last time.
                    if (oFuzzystruct[j].matchWordDistance == maximumDistance )
                    {
                        maximumDistanceAt = j;
                    }
                }

                if (!isAlreadyInList)
                {
                    oFuzzystruct[maximumDistanceAt].pMatchWord = searchCheckWord.toUtf8();
                    oFuzzystruct[maximumDistanceAt].matchWordDistance = iDistance;
                    // calc new iMaxDistance
                    maximumDistance = iDistance;
                    int tmpVal = maximumDistance; //stupid workaround for gcc bug 44545
                    for (int j = 0; j < resultListSize; ++j)
                    {
                        if (oFuzzystruct[j].matchWordDistance > maximumDistance)
                            tmpVal = oFuzzystruct[j].matchWordDistance;
                    } // calc new iMaxDistance

                    maximumDistance = tmpVal; // end of stupid workaround
                }   // add to list
            }   // find one
        }   // each word
    }   // ok for search
//    }   // each lib

    if (found) // sort with distance
        qSort(oFuzzystruct, oFuzzystruct + resultListSize);

    for (int i = 0; i < resultListSize; ++i)
        resultList[i] = oFuzzystruct[i].pMatchWord;

    delete [] oFuzzystruct;

    return found;
}

inline bool
lessForCompare(QString lh, QString rh)
{
    return stardictStringCompare(lh, rh) < 0;
}

int
StarDictDictionaryManager::lookupWithRule(QByteArray patternWord, QStringList patternMatchWords)
{
    long aiIndex[d->maxMatchItemPerLib + 1];
    int matchCount = 0;

    for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        //if(oStarDictDictionaryManager.LookdupWordsWithRule(pspec,aiIndex,MAX_MATCH_ITEM_PER_LIB+1-iMatchCount,iLib))
        // -iMatchCount,so save time,but may got less result and the word may repeat.

        if (d->dictionaries.at(iLib)->lookupWithRule(patternWord, aiIndex, d->maxMatchItemPerLib + 1))
        {
            if (d->progressFunction)
                d->progressFunction();

            for (int i = 0; aiIndex[i] != -1; ++i)
            {
                QByteArray searchMatchWord = poWord(aiIndex[i], iLib);

                if (!patternMatchWords.contains(searchMatchWord))
                    patternMatchWords.append(searchMatchWord);
            }
        }
    }

    if (matchCount) // sort it.
        qSort(patternMatchWords.begin(), patternMatchWords.end(), lessForCompare);

    return matchCount;
}

bool
StarDictDictionaryManager::lookupData(QByteArray search_word, QStringList resultList)
{
    QStringList searchWords;
    QString searchWord;
    foreach (char ch, search_word)
    {
        if (ch == '\\')
        {
            switch (ch)
            {
            case ' ':
                searchWord.append(' ');
                break;
            case '\\':
                searchWord.append('\\');
                break;
            case 't':
                searchWord.append('\t');
                break;
            case 'n':
                searchWord.append('\n');
                break;
            default:
                searchWord.append(ch);
            }
        }
        else if (ch == ' ')
        {
            if (!searchWord.isEmpty())
            {
                searchWords.append(searchWord);
                searchWord.clear();
            }
        }
        else
        {
            searchWord.append(ch);
        }
    }

    if (!searchWord.isEmpty())
    {
        searchWords.append(searchWord);
        searchWord.clear();
    }

    if (searchWords.isEmpty())
        return false;

    int maximumSize = 0;
    QByteArray originalData = NULL;
    for (QVector<Dictionary *>::size_type i = 0; i < d->dictionaries.size(); ++i)
    {
        if (!d->dictionaries.at(i)->containFindData())
            continue;

        if (d->progressFunction)
            d->progressFunction();

        int wordSize = articleCount(i);
        QByteArray key;
        qint32 offset;
        qint32 size;
        for (int j = 0; j < wordSize; ++j)
        {
            d->dictionaries.at(i)->wordEntry(j, key, offset, size);
            if (size > maximumSize)
            {
                maximumSize = size;
            }

            if (d->dictionaries.at(i)->findData(searchWords, offset, size, originalData))
                resultList[i].append(key);
        }
    }

    QVector<Dictionary *>::size_type i;
    for (i = 0; i < d->dictionaries.size(); ++i)
        if (!resultList[i].isEmpty())
            break;

    return i != d->dictionaries.size();
}

StarDictDictionaryManager::QueryType
StarDictDictionaryManager::analyzeQuery(QString string, QString& result)
{
    if (string.isNull() || result.isNull())
    {
        result = "";
        return SIMPLE;
    }

    if (string.startsWith('/'))
    {
        result = string.mid(1);
        return FUZZY;
    }
    else if (string.startsWith('|'))
    {
        result = string.mid(1);
        return DATA;
    }

    string.remove('\\');

    if (string.contains('*') || string.contains('?'))
        return REGEXP;

    return SIMPLE;
}
