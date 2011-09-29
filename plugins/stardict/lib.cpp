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

#include "lib.h"

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
    QChar ch = inputChar.toUpper();
    return ( ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U' );
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

class Libs::Private
{
    public:
        Private()
            : maximumFuzzyDistance(MAX_FUZZY_DISTANCE) // need to read from cfg
        {
        }

        ~Private()
        {
        }

        QVector<Dictionary *> dictionaries; // word Libs.
        int maximumFuzzyDistance;
        progress_func_t progressFunction;

        QVector<Dictionary *> previous;
        QVector<Dictionary *> future;
};

Libs::Libs(progress_func_t progressFunction)
    : d(new Private)
{
    d->progressFunction = progressFunction;
}

Libs::~Libs()
{
    qDeleteAll(d->dictionaries);
    delete d;
}

void Libs::loadDictionary(const QString& url)
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
Libs::articleCount(int index) const
{
    return d->dictionaries.at(index)->articleCount();
}

const QString&
Libs::dictionaryName(int index) const
{
    return d->dictionaries.at(index)->dictionaryName();
}

int
Libs::dictionaryCount() const
{
    return d->dictionaries.size();
}

const QByteArray
Libs::poWord(long keyIndex, int libIndex) const
{
    return d->dictionaries.at(libIndex)->key(keyIndex).toUtf8();
}

QString
Libs::poWordData(long dataIndex, int libIndex)
{
    if (libIndex == invalidIndex)
        return NULL;

    return d->dictionaries.at(libIndex)->data(dataIndex);
}

bool
Libs::lookupWord(const char* searchWord, long& iWordIndex, int libIndex)
{
    return d->dictionaries.at(libIndex)->lookup(searchWord, iWordIndex);
}

template <typename Method>
void
Libs::recursiveTemplateHelper(const QString& directoryName, const QStringList& orderList, const QStringList& disableList,
                        Method method)
{
    QDir dir(directoryName);

    // Going through the subfolders
    foreach (QString entryName, dir.entryList(QDir::Dirs & QDir::NoDotAndDotDot))
    {
        QString absolutePath = dir.absoluteFilePath(entryName);
        recursiveTemplateHelper(absolutePath, orderList, disableList, method);
    }

    foreach (QString entryName, dir.entryList(QDir::Files & QDir::Drives & QDir::NoDotAndDotDot))
    {
        QString absolutePath = dir.absoluteFilePath(entryName);
        if (absolutePath.endsWith(".ifo")
                && qFind(orderList.begin(), orderList.end(), absolutePath) == orderList.end()
                && qFind(disableList.begin(), disableList.end(), absolutePath) == disableList.end())
        {
                (this->*method)(absolutePath);
        }
    }
}

void
Libs::load(const QStringList& dictionaryDirectoryList,
                const QStringList& orderList,
                const QStringList& disableList)
{
    foreach (const QString& string, orderList)
    {
        if (qFind(disableList.begin(), disableList.end(), string) == disableList.end())
            loadDictionary(string);
    }

    foreach (const QString& directoryName, dictionaryDirectoryList)
        recursiveTemplateHelper(directoryName, orderList, disableList, &Libs::loadDictionary);
}

Dictionary*
Libs::reloaderFind(const QString& url)
{
    QVector<Dictionary *>::iterator it;
    for (it = d->previous.begin(); it != d->previous.end(); ++it)
    {
        if ((*it)->ifoFileName() == url)
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
Libs::reloaderHelper(const QString &absolutePath)
{
    Dictionary *dictionary = reloaderFind(absolutePath);
    if (dictionary)
        d->future.append(dictionary);
    else
        loadDictionary(absolutePath);
}

void
Libs::reload(const QStringList& dictionaryDirectoryList,
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
        recursiveTemplateHelper(directoryName, orderList, disableList, &Libs::reloaderHelper);

    qDeleteAll(d->previous);
}

QByteArray
Libs::poCurrentWord(long *iCurrent)
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
Libs::poNextWord(QByteArray searchWord, long *iCurrent)
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
Libs::poPreviousWord(long *iCurrent)
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
Libs::lookupSimilarWord(QByteArray searchWord, long& iWordIndex, int iLib)
{
    long iIndex;
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

        // Cut "s" or "ed"
        if (!found && searchWordLength > 1)
        {
            isUpperCase = searchWord.endsWith('S') || searchWord.endsWith("ED");
            if (isUpperCase || searchWord.endsWith('s') || searchWord.endsWith("ed"))
            {
                searchNewWord = searchWord.left(searchWordLength - 1);
                // cut "s" or "d"
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

        //cut "ly"
        if (!found && searchWordLength > 2)
        {
            isUpperCase = searchWord.endsWith("LY");
            if (isUpperCase || searchWord.endsWith("ly"))
            {
                searchNewWord = searchWord.left(searchWordLength - 2); // cut "ly"
                if (searchWordLength > 3 && searchNewWord.at(searchWordLength - 1) == searchNewWord.at(searchWordLength - 2)
                        && !isVowel(searchNewWord.at(searchWordLength - 2)) && isVowel(searchNewWord.at(searchWordLength - 3)))
                { //doubled

                    searchNewWord.remove(searchNewWord.length() - 1, 1);
                    if ( d->dictionaries.at(iLib)->lookup(searchNewWord, iIndex) )
                    {
                        found = true;
                    }
                    else
                    {
                        if (isUpperCase || QString::fromUtf8(searchWord).at(0).isUpper())
                        {
                            caseString = searchNewWord.toLower();
                            if (caseString.compare(searchNewWord))
                            {
                                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }

                        if (!found)
                            searchNewWord.append(searchNewWord.at(searchNewWord.length() - 1));  //restore
                    }
                }

                if (!found)
                {
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
        }

        //cut "ing"
        if (!found && searchWordLength > 3)
        {
            isUpperCase = searchWord.endsWith("ING");
            if (isUpperCase || searchWord.endsWith("ing"))
            {
                searchNewWord = searchWord.left(searchWordLength - 3);
                if ( searchWordLength > 3 && (searchNewWord.at(searchWordLength - 1) == searchNewWord.at(searchWordLength - 2))
                        && !isVowel(searchNewWord.at(searchWordLength - 2)) && isVowel(searchNewWord.at(searchWordLength - 3)))
                {  //doubled
                    searchNewWord.remove(searchNewWord.length() - 1, 1);
                    if (d->dictionaries.at(iLib)->lookup(searchNewWord, iIndex))
                        found = true;
                    else
                    {
                        if (isUpperCase || QString::fromUtf8(searchWord).at(0).isUpper())
                        {
                            caseString = searchNewWord.toLower();
                            if (caseString.compare(searchNewWord))
                            {
                                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }

                        if (!found)
                            searchNewWord.append(searchNewWord.at(searchNewWord.length() - 1));  //restore
                    }
                }

                if (!found)
                {
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

                if (!found)
                {
                    if (isUpperCase)
                        searchNewWord.append('E'); // add a char "E"
                    else
                        searchNewWord.append('e'); // add a char "e"

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
        }

        //cut two char "es"
        if (!found && searchWordLength > 3)
        {
            isUpperCase = (searchWord.endsWith("ES")
                        && (searchWord.at(searchWordLength - 3) == 'S'
                        || searchWord.at(searchWordLength - 3) == 'X'
                        || searchWord.at(searchWordLength - 3) == 'O'
                        || (searchWordLength > 4 && searchWord.at(searchWordLength - 3) == 'H'
                        && (searchWord.at(searchWordLength - 4) == 'C'
                        || searchWord.at(searchWordLength - 4) == 'S'))));

            if (isUpperCase ||
                    (searchWord.endsWith("es")
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

        //cut "ed"
        if (!found && searchWordLength > 3)
        {
            isUpperCase = searchWord.endsWith("ED");
            if (isUpperCase || searchWord.endsWith("ed"))
            {
                searchNewWord = searchWord.left(searchWordLength - 2);
                if (searchWordLength > 3 && (searchNewWord.at(searchWordLength - 1) == searchNewWord.at(searchWordLength - 2))
                        && !isVowel(searchNewWord.at(searchWordLength - 2)) && isVowel(searchNewWord.at(searchWordLength - 3)))
                { //doubled
                    searchNewWord.remove(searchNewWord.length() - 1, 1);
                    if (d->dictionaries.at(iLib)->lookup(searchNewWord, iIndex))
                    {
                        found = true;
                    }
                    else
                    {
                        if (isUpperCase || QString::fromUtf8(searchWord).at(0).isUpper())
                        {
                            caseString = searchNewWord.toLower();
                            if (caseString.compare(searchNewWord))
                            {
                                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }
                        if (!found)
                            searchNewWord.append(searchNewWord.at(searchNewWord.length() - 1));  //restore
                    }
                }

                if (!found)
                {
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
        }

        // cut "ied" , add "y".
        if (!found && searchWordLength > 3)
        {
            isUpperCase = searchWord.endsWith("IED");
            if (isUpperCase || searchWord.endsWith("ied"))
            {
                searchNewWord = searchWord.left(searchWordLength - 3);
                if (isUpperCase)
                {
                    searchNewWord.append('Y'); // add a char "Y"
                }
                else
                {
                    searchNewWord.append('y'); // add a char "y"
                }

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

        // cut "ies" , add "y".
        if (!found && searchWordLength > 3)
        {
            isUpperCase = searchWord.endsWith("IES");
            if (isUpperCase || searchWord.endsWith("ies"))
            {
                searchNewWord = searchWord.left(searchWordLength - 3);
                if (isUpperCase)
                {
                    searchNewWord.append('Y'); // add a char "Y"
                }
                else
                {
                    searchNewWord.append('y'); // add a char "y"
                }

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

        // cut "er".
        if (!found && searchWordLength > 2)
        {
            isUpperCase = searchWord.endsWith("ER");
            if (isUpperCase || searchWord.endsWith("er"))
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

        // cut "est".
        if (!found && searchWordLength > 3)
        {
            isUpperCase = searchWord.endsWith("EST");
            if (isUpperCase || searchWord.endsWith("est"))
            {
                searchNewWord = searchWord.left(searchWordLength - 3);
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
    }

    if (found)
        iWordIndex = iIndex;
#if 0
    else
    {
        //don't change iWordIndex here.
        //when LookupSimilarWord all failed too, we want to use the old LookupWord index to list words.
        //iWordIndex = invalidIndex;
    }
#endif
    return found;
}

bool
Libs::simpleLookupWord(QByteArray searchWord, long &iWordIndex, int iLib)
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
Libs::lookupWithFuzzy(QByteArray searchWord, QStringList resultList, int resultListSize, int iLib)
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
Libs::lookupWithRule(QByteArray patternWord, QStringList patternMatchWords)
{
    long aiIndex[MAX_MATCH_ITEM_PER_LIB + 1];
    int matchCount = 0;

    for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        //if(oLibs.LookdupWordsWithRule(pspec,aiIndex,MAX_MATCH_ITEM_PER_LIB+1-iMatchCount,iLib))
        // -iMatchCount,so save time,but may got less result and the word may repeat.

        if (d->dictionaries.at(iLib)->lookupWithRule(patternWord, aiIndex, MAX_MATCH_ITEM_PER_LIB + 1))
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
Libs::lookupData(QByteArray search_word, QStringList resultList)
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

        long wordSize = articleCount(i);
        QByteArray key;
        qint32 offset;
        qint32 size;
        for (long j = 0; j < wordSize; ++j)
        {
            d->dictionaries.at(i)->keyAndData(j, key, offset, size);
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

Libs::QueryType
Libs::analyzeQuery(QString string, QString& result)
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
