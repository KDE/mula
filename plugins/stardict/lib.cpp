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
#include <QtCore/QDebug>

#include <zlib.h>

using namespace MulaPluginStarDict;

// Notice: read src/tools/DICTFILE_FORMAT for the dictionary
// file's format information!

static bool isVowel(QChar inputChar)
{
    QChar ch = inputChar.toUpper();
    return ( ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U' );
}

static bool isPureEnglish(const QByteArray& word)
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

static inline int stardictStringCompare(const QString string1, const QString string2)
{
    int retval = string1.compare(string2, Qt::CaseInsensitive);
    if (retval == 0)
        return string1.compare(string2);
    else
        return retval;
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
 
        QList<Dictionary *> dictionaries; // word Libs.
        int maximumFuzzyDistance;
        progress_func_t progress_func;
};

Libs::Libs(progress_func_t f)
    : d(new Private)
{
    Q_UNUSED(f);
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

ulong
Libs::articleCount(int index) const
{
    return d->dictionaries.at(index)->articlesCount();
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
Libs::poWord(ulong keyIndex, int libIndex) const
{
    return d->dictionaries.at(libIndex)->key(keyIndex).toUtf8();
}

QString
Libs::poWordData(ulong dataIndex, int libIndex)
{
    if (iIndex == INVALID_INDEX)
        return NULL;

    return d->dictionaries.at(libIndex)->data(dataIndex);
}

bool
Libs::lookupWord(const char* searchWord, ulong& iWordIndex, int libIndex)
{
    return d->dictionaries.at(libIndex)->lookup(searchWord, iWordIndex);
}

class DictionaryLoader
{
    public:
        DictionaryLoader(Libs& lib_): lib(lib_)
        {
        }

        virtual ~DictionaryLoader()
        {
        }

        void operator() (const QString& url, bool enable)
        {
            if (enable)
                lib.loadDictionary(url);
        }

    private:
        Libs& lib;
};

void Libs::load(const QStringList& dictionaryDirs,
                const QStringList& orderList,
                const QStringList& disableList)
{
    for_each_file(dictionaryDirs, ".ifo", orderList, disableList,
                  DictionaryLoader(*this));
}

class DictionaryReLoader
{
    public:
        DictionaryReLoader(QVector<Dictionary *> &p, QVector<Dictionary *> &f, Libs& lib_)
            : previous(p)
            , future(f)
            , lib(lib_)
        {
        }

        void operator() (const QString& url, bool enable)
        {
            if (enable)
            {
                Dictionary *dictionary = find(url);
                if (dictionary)
                    future.append(dictionary);
                else
                    lib.loadDictionary(url);
            }
        }

    private:
        QVector<Dictionary *> &future;
        Libs& lib;

        Dictionary *find(const QString& url)
        {
            QVector<Dictionary *>::iterator it;
            for (it = previous.begin(); it != previous.end(); ++it)
                if ((*it)->ifoFileName() == url)
                    break;

            if (it != previous.end())
            {
                Dictionary *result = *it;
                previous.erase(it);
                return result;
            }
            return NULL;
        }
};

void
Libs::reload(const QStringList& dictionaryDirs,
                  const QStringList& orderList,
                  const QStringList& disableList)
{
    QVector<Dictionary *> previous(d->dictionaries);
    d->dictionaries.clear();
    for_each_file(dictionaryDirs, ".ifo", orderList, disableList,
                  DictionaryReLoader(previous, d->dictionaries, *this));

    qDeleteAll(previous)
}

const char *
Libs::poCurrentWord(ulong *iCurrent)
{
    const char *poCurrentWord = NULL;
    const char *word;

    for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        if (iCurrent.at(iLib) == INVALID_INDEX)
            continue;

        if (iCurrent[iLib] >= articlesCount(iLib) || iCurrent[iLib] < 0)
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

const char *
Libs::poNextWord(const char *sWord, ulong *iCurrent)
{
    // the input can be:
    // (word,iCurrent),read word,write iNext to iCurrent,and return next word. used by TopWin::NextCallback();
    // (NULL,iCurrent),read iCurrent,write iNext to iCurrent,and return next word. used by AppCore::ListWords();
    const char *poCurrentWord = NULL;
    QVector<Dictionary *>::size_type iCurrentLib = 0;
    const char *word;

    for (QVector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        if (sWord)
            d->dictionaries.at(iLib)->lookup(sWord, iCurrent[iLib]);

        if (iCurrent[iLib] == INVALID_INDEX)
            continue;

        if (iCurrent[iLib] >= articleCount(iLib) || iCurrent[iLib] < 0)
            continue;

        if (poCurrentWord == NULL )
        {
            poCurrentWord = poWord(iCurrent[iLib], iLib);
            iCurrentLib = iLib;
        }
        else
        {
            word = poWord(iCurrent[iLib], iLib);

            if (stardictStringCompare(poCurrentWord, word) > 0 )
            {
                poCurrentWord = word;
                iCurrentLib = iLib;
            }
        }
    }

    if (poCurrentWord)
    {
        iCurrent[iCurrentLib]
        ++;
        for (std::vector<Dictionary *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
        {
            if (iLib == iCurrentLib)
                continue;

            if (iCurrent[iLib] == INVALID_INDEX)
                continue;

            if ( iCurrent[iLib] >= narticles(iLib) || iCurrent[iLib] < 0)
                continue;

            if (strcmp(poCurrentWord, poWord(iCurrent[iLib], iLib)) == 0 )
                iCurrent[iLib]++;
        }

        poCurrentWord = poCurrentWord(iCurrent);
    }
    return poCurrentWord;
}

const char *
Libs::poPreviousWord(ulong *iCurrent)
{
    // used by TopWin::PreviousCallback(); the iCurrent is cached by AppCore::TopWinWordChange();
    const char *poCurrentWord = NULL;
    QVector<Dictionary *>::size_type iCurrentLib = 0;
    const char *word;

    for (QVector<Dict *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        if (iCurrent[iLib] == INVALID_INDEX)
            iCurrent[iLib] = narticles(iLib);
        else
        {
            if ( iCurrent[iLib] > narticles(iLib) || iCurrent[iLib] <= 0)
                continue;
        }

        if ( poCurrentWord == NULL )
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

    if (poCurrentWord)
    {
        iCurrent[iCurrentLib]--;
        for (QVector<Dict *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
        {
            if (iLib == iCurrentLib)
                continue;

            if (iCurrent[iLib] > narticles(iLib) || iCurrent[iLib] <= 0)
                continue;

            if (poCurrentWord.compare(poWord(iCurrent[iLib] - 1, iLib)) == 0)
            {
                iCurrent[iLib]--;
            }
            else
            {
                if (iCurrent[iLib] == narticles(iLib))
                    iCurrent[iLib] = INVALID_INDEX;
            }
        }
    }
    return poCurrentWord;
}

bool Libs::removePattern(QString pattern, QString originalWord, bool found)
{
        //cut "ly"
        int wordLength = originalWord.length();
        if (!found && wordLength > pattern.length())
        {
            originalWordPart = originalWord.mid([wordLength - 2], 2);
            bool isUpperCase = !strncmp(&sWord[wordLength - 2], "LY", 2);
            if (!originiWordPart.compare("LY", Qt::CaseSensitive) || !originiWordPart.compare("ly", Qt::CaseSensitive))
            {
                sNewWord = sWord;
                sNewWord[iWordLen - 2] = '\0';  // cut "ly"
                if (iWordLen > 5 && sNewWord[iWordLen - 3] == sNewWord[iWordLen - 4]
                        && !isVowel(sNewWord[iWordLen - 4]) && isVowel(sNewWord[iWordLen - 5]))
                { //doubled

                    sNewWord[iWordLen - 3] = '\0';
                    if ( d->dictionaries.at(iLib)->lookup(sNewWord, iIndex) )
                        bFound = true;
                    else
                    {
                        if (isUpperCase || sWord.at(0).isUpper())
                        {
                            caseString = sNewWord.toLower();
                            if (caseString.compare(sNewWord))
                            {
                                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }
                        if (!found)
                            sNewWord[iWordLen - 3] = sNewWord[iWordLen - 4];  //restore
                    }
                }

                if (!found)
                {
                    if (d->dictionaries.at(iLib)->lookup(sNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || sWord.at(0).isUpper())
                    {
                        caseString = sNewWord.toLower();
                        if (caseString.compare(sNewWord))
                        {
                            if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                found = true;
                        }
                    }
                }
            }
        }
}

bool Libs::lookupSimilarWord(const QString sWord, ulong & iWordIndex, int iLib)
{
    ulong iIndex;
    bool found = false;
    QString caseString;

    if (!found)
    {
        // to lower case.
        caseString = sWord.toLower();
        if (caseString.compare(sWord))
        {
            if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                found = true;
        }

        // to upper case.
        if (!found)
        {
            caseString = sWord.toUpper();
            if (caseString.compare(sWord))
            {
                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                    found = true;
            }
        }

        // Upper the first character and lower others.
        if (!found)
        {
            caseString = sWord.toLower();
            caseString[0] = caseString[0].toUpper();
            if (caseString.compare(sWord))
            {
                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                    found = true;
            }
        }
    }

    if (isPureEnglish(similarWord))
    {
        // If not Found, try other status of sWord.
        int similarWordLength = similarWord.size();
        bool isUpperCase;

        QString similarNewWord = similarWord.left(similarWordLength - 1); //cut one char "s" or "d"
        if (!found && similarWordLength > 1)
        {
            isUpperCase = similarWord.endsWith('S') || similarWord.endsWith("ED");
            if (isUpperCase || similarWordLength.endsWith('s') || similarWord.endsWith("ed"))
            {
                similarNewWord = similarWord.left(similarWordLength - 1);
                // cut "s" or "d"
                if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                {
                    found = true;
                }
                else if (isUpperCase || similarWord.at(0).isUpper())
                {
                    caseString = similarNewWord.toLower();
                    if (caseString.compare(similarNewWord))
                    {
                        if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                            found = true;
                    }
                }
            }
        }

        //cut "ly"
        if (!found && similarWordLength > 2)
        {
            isUpperCase = similarWord.endsWith("LY");
            if (isUpperCase || similarWord.endsWith("ly"))
            {
                similarNewWord = similarWord.left(similarWordLength - 2); // cut "ly"
                if (similarWordLength > 5 && similarNewWord.at(similarWordLength - 3) == similarNewWord.at(similarWordLength - 4)
                        && !isVowel(similarNewWord.at(similarWordLength - 4)) && isVowel(similarNewWord.at(similarWordLength - 5)))
                { //doubled

                    similarNewWord.remove(similarWordLength - 3);
                    if ( d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex) )
                    {
                        found = true;
                    }
                    else
                    {
                        if (isUpperCase || similarWord.at(0).isUpper())
                        {
                            caseString = similarNewWord.toLower();
                            if (caseString.compare(similarNewWord))
                            {
                                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }

                        if (!found)
                            similarNewWord.append(similarNewWord(similarNewWord.length() -1));  //restore
                    }
                }

                if (!found)
                {
                    if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || similarWord.at(0).isUpper())
                    {
                        caseString = similarNewWord.toLower();
                        if (caseString.compare(similarNewWord))
                        {
                            if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                found = true;
                        }
                    }
                }
            }
        }

        //cut "ing"
        if (!found && similarWordLength > 3)
        {
            isUpperCase = similarWord.endsWith("ING");
            if (isUpperCase || similarWord.endsWith("ing"))
            {
                similarNewWord = similarWord.left(similarWordLength - 3);
                if ( similarWordLength > 6 && (similarNewWord.at(similarWordLength - 4) == similarNewWord.at(similarWordLength - 5))
                        && !isVowel(similarNewWord.at(similarWordLength - 5)) && isVowel(similarNewWord.at(similarWordLength - 6)))
                {  //doubled
                    similarNewWord.remove(similarNewWord.length() - 1);
                    if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                        found = true;
                    else
                    {
                        if (isUpperCase || similarWord.at(0).isUpper())
                        {
                            caseString = similarNewWord.toLower();
                            if (caseString.compare(similarNewWord))
                            {
                                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }

                        if (!found)
                            similarNewWord.append(similarNewWord(similarNewWord.length() - 1));  //restore
                    }
                }

                if (!found)
                {
                    if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || similarWord.at(0).isUpper())
                    {
                        caseString = similarNewWord.toLower();
                        if (caseString.compare(similarNewWord))
                        {
                            if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                found = true;
                        }
                    }
                }

                if (!found)
                {
                    if (isUpperCase)
                        sNewWord += "E"; // add a char "E"
                    else
                        sNewWord += "e"; // add a char "e"

                    if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || sWord.at(0).isUpper())
                    {
                        caseString = sNewWord.toLower();
                        if (caseString.compare(sNewWord))
                        {
                            if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                found = true;
                        }
                    }
                }
            }
        }

        //cut two char "es"
        if (!found && similarWordLength > 3)
        {
            isUpperCase = (similarWord.endsWith("ES")
                        && similarWord.at(similarWordLength - 3) == 'S'
                        || similarWord.at(similarWordLength - 3) == 'X'
                        || similarWord.at(similarWordLength - 3) == 'O'
                        || (similarWordLenth > 4 && similarWord.at(similarWordLength - 3) == 'H'
                        && (similarWord.at(similarWordLength - 4) == 'C'
                        || similarWord.at(similarWordLength - 4) == 'S')));

            if (isUpperCase ||
                    (similarWord.endsWith("es")
                     && (similarWord.at(similarWordLength - 3) == 's'
                     || similarWord.at(similarWordLength - 3) == 'x'
                     || similarWord.at(similarWordLength - 3) == 'o'
                     || (similarWordLength > 4 && similarWord.at(similarWordLength - 3) == 'h'
                     && (similarWord.at(similarWordLength - 4) == 'c'
                     || similarWord.at(similarWordLength - 4) == 's')))))
            {
                similarNewWord = similarWord.left(similarWordLength - 2);
                if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                {
                    found = true;
                }
                else if (isUpperCase || sWord.at(0).isUpper())
                {
                    caseString = similarNewWord.toLower();
                    if (caseString.compare(similarNewWord))
                    {
                        if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                            found = true;
                    }
                }
            }
        }

        //cut "ed"
        if (!found && similarWordLength > 3)
        {
            isUpperCase = similar.endsWith("ED");
            if (isUpperCase || similarWord.endsWith("ed"))
            {
                similarNewWord = similarWord.left(similarWordLength - 2);
                if (similarWordLength > 5 && (similarNewWord.at(similarWordLength - 3) == similarNewWord.at(similarWordLength - 4))
                        && !isVowel(similarNewWord.at(similarWordLength - 4)) && isVowel(similarNewWord.at(similarWordLength - 5)))
                { //doubled
                    similarNewWord.remove(similarNewWord.length() - 1);
                    if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                    {
                        found = true;
                    }
                    else
                    {
                        if (isUpperCase || sWord.at(0).isUpper())
                        {
                            caseString = similarNewWord.toLower();
                            if (caseString.compare(similarNewWord))
                            {
                                if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }
                        if (!found)
                            similarNewWord.append(similarNewWord.at(similarNewWord.length() - 1);  //restore
                    }
                }

                if (!found)
                {
                    if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || similarWord.at(0).isUpper())
                    {
                        caseString = similarNewWord.toLower();
                        if (caseString.compare(similarNewWord))
                        {
                            if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                                found = true;
                        }
                    }
                }
            }
        }

        // cut "ied" , add "y".
        if (!found && similarWordLength > 3)
        {
            isUpperCase = similarWord.endsWith("IED");
            if (isUpperCase || similarWord.endsWith("ied"))
            {
                sNewWord = sWord.left(similarWordLength() - 3);
                if (isUpperCase)
                {
                    similarNewWord.append('Y'); // add a char "Y"
                }
                else
                {
                    similarNewWord.append('y'); // add a char "y"
                }

                if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                {
                    found = true;
                }
                else if (isUpperCase ||sWord.at(0).isUpper())
                {
                    caseString = similarNewWord.toLower();
                    if (caseString.compare(similarNewWord))
                    {
                        if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                            bound = true;
                    }
                }
            }
        }

        // cut "ies" , add "y".
        if (!bound && similarWordLength > 3)
        {
            isUpperCase = similarWord.endsWith("IES");
            if (isUpperCase || similarWord.endsWith("ies"))
            {
                similarNewWord = similarWord.left(similarWordLength - 3);
                if (isUpperCase)
                {
                    similarNewWord.append('Y'); // add a char "Y"
                }
                else
                { 
                    similarNewWord.append('y'); // add a char "y"
                }

                if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                {
                    found = true;
                }
                else if (isUpperCase || similarWord.at(0).isUpper())
                {
                    caseString = similarNewWord.toLower();
                    if (caseString.compare(similarNewWord))
                    {
                        if (d->dictionaries.at(iLib)->lookup(caseString, iIndex))
                            bound = true;
                    }
                }
            }
        }

        // cut "er".
        if (!found && similarWordLength > 2)
        {
            isUpperCase = similarWord.endsWith("ER");
            if (isUpperCase || similarWord.endsWith("er"))
            {
                similarNewWord = similarWord.left(similarWordLength - 2);
                if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                {
                    found = true;
                }
                else if (isUpperCase || similarWord.at(0).isUpper())
                {
                    caseString = similarNewWord.toLower();
                    if (caseString.compare(similarNewWord))
                    {
                        if (d->dictionaries(iLib)->lookup(caseString, iIndex))
                            bound = true;
                    }
                }
            }
        }

        // cut "est".
        if (!bound && similarWordLength > 3)
        {
            isUpperCase = sWord.endsWith("EST");
            if (isUpperCase || similarWord.endsWith("est"))
            {
                similarNewWord = similarWord.left(similarWordLength - 3);
                if (d->dictionaries.at(iLib)->lookup(similarNewWord, iIndex))
                {
                    found = true;
                }
                else if (isUpperCase || similarWord.at(0).isUpper())
                {
                    caseString = similarNewWord.toLower();
                    if (caseString.compare(similarNewWord))
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
        //iWordIndex = INVALID_INDEX;
    }
#endif
    return found;
}

bool Libs::simpleLookupWord(const QString sWord, ulong &iWordIndex, int iLib)
{
    bool found = d->dictionaries.at(iLib)->lookup(sWord, iWordIndex);
    if (!found)
        found = lookupSimilarWord(sWord, iWordIndex, iLib);

    return found;
}

struct Fuzzystruct
{
    char * pMatchWord;
    int iMatchWordDistance;
};

inline bool operator<(const Fuzzystruct & lh, const Fuzzystruct & rh)
{
    if (lh.iMatchWordDistance != rh.iMatchWordDistance)
        return lh.iMatchWordDistance < rh.iMatchWordDistance;

    if (lh.pMatchWord && rh.pMatchWord)
        return stardict_strcmp(lh.pMatchWord, rh.pMatchWord) < 0;

    return false;
}

bool Libs::lookupWithFuzzy(const QString sWord, QString reslist[], int sresList, int iLib)
{
    if (sWord[0] == '\0')
        return false;

    Fuzzystruct *oFuzzystruct = new Fuzzystruct[reslist_size];

    for (int i = 0; i < sresList; ++i)
    {
        oFuzzystruct[i].pMatchWord = NULL;
        oFuzzystruct[i].iMatchWordDistance = iMaxFuzzyDistance;
    }

    int iMaxDistance = iMaxFuzzyDistance;
    int iDistance;
    bool found = false;
    EditDistance oEditDistance;

    ulong iCheckWordLen;
    const QString sCheck;
    QString ucs4Str1;
    QString ucs4Str2;
    long sucs4Str2;

    ucs4_str2 = g_utf8_to_ucs4_fast(sWord, -1, &ucs4_str2_len);
    unicode_strdown(ucs4_str2);

    if (progress_func)
        progress_func();

    //if (stardict_strcmp(sWord, poGetWord(0,iLib))>=0 && stardict_strcmp(sWord, poGetWord(narticles(iLib)-1,iLib))<=0) {
    //there are Chinese dicts and English dicts...
    if (TRUE)
    {
        const int iwords = narticles(iLib);
        for (int index = 0; index < iwords; index++)
        {
            sCheck = poWord(index, iLib);
            // tolower and skip too long or too short words
            iCheckWordLen = sCheck.length();
            if (iCheckWordLen - ucs4_str2_len >= iMaxDistance ||
                    ucs4_str2_len - iCheckWordLen >= iMaxDistance)
                continue;

            ucs4_str1 = g_utf8_to_ucs4_fast(sCheck, -1, NULL);
            if (iCheckWordLen > sucs4Str2)
                ucs4_str1[ucs4_str2_len] = 0;
            unicode_strdown(ucs4_str1);

            iDistance = oEditDistance.CalEditDistance(ucs4_str1, ucs4_str2, iMaxDistance);
            if (iDistance < iMaxDistance && iDistance < ucs4_str2_len)
            {
                // when ucs4_str2_len=1,2 we need less fuzzy.
                found = true;
                bool isAlreadyInList = false;
                int iMaxDistanceAt = 0;
                for (int j = 0; j < sresList; ++j)
                {
                    if (oFuzzystruct[j].pMatchWord
                        && strcmp(oFuzzystruct[j].pMatchWord, sCheck) == 0 )
                    { //already in list
                        isAlreadyInList = true;
                        break;
                    }
                    //find the position,it will certainly be found (include the first time) as iMaxDistance is set by last time.
                    if (oFuzzystruct[j].iMatchWordDistance == iMaxDistance )
                    {
                        iMaxDistanceAt = j;
                    }
                }
                if (!isAlreadyInList)
                {
                    if (oFuzzystruct[iMaxDistanceAt].pMatchWord)
                        g_free(oFuzzystruct[iMaxDistanceAt].pMatchWord);

                    oFuzzystruct[iMaxDistanceAt].pMatchWord = g_strdup(sCheck);
                    oFuzzystruct[iMaxDistanceAt].iMatchWordDistance = iDistance;
                    // calc new iMaxDistance
                    iMaxDistance = iDistance;
                    int tmpVal = iMaxDistance; //stupid workaround for gcc bug 44545
                    for (int j = 0; j < reslist_size; j++)
                    {
                        if (oFuzzystruct[j].iMatchWordDistance > iMaxDistance)
                            tmpVal = oFuzzystruct[j].iMatchWordDistance;
                    } // calc new iMaxDistance
                    iMaxDistance = tmpVal; // end of stupid workaround
                }   // add to list
            }   // find one
        }   // each word
    }   // ok for search
//    }   // each lib
    g_free(ucs4_str2);

    if (found) // sort with distance
        qSort(oFuzzystruct, oFuzzystruct + sresList);

    for (int i = 0; i < sresList; ++i)
        resList[i] = oFuzzystruct[i].pMatchWord;

    delete [] oFuzzystruct;

    return found;
}

inline bool lessForCompare(const char *lh, const char *rh)
{
    return stardict_strcmp(lh, rh) < 0;
}

int Libs::lookupWithRule(const gchar *word, gchar **ppMatchWord)
{
    ulong aiIndex[MAX_MATCH_ITEM_PER_LIB + 1];
    int iMatchCount = 0;
    GPatternSpec *pspec = g_pattern_spec_new(word);

    for (QVector<Dict *>::size_type iLib = 0; iLib < d->dictionaries.size(); ++iLib)
    {
        //if(oLibs.LookdupWordsWithRule(pspec,aiIndex,MAX_MATCH_ITEM_PER_LIB+1-iMatchCount,iLib))
        // -iMatchCount,so save time,but may got less result and the word may repeat.

        if (d->dictionaries.at(iLib)->lookupWithRule(pspec, aiIndex, MAX_MATCH_ITEM_PER_LIB + 1))
        {
            if (progress_func)
                progress_func();

            for (int i = 0; aiIndex[i] != -1; ++i)
            {
                const char *sMatchWord = poWord(aiIndex[i], iLib);
                bool isAlreadyInList = false;
                for (int j = 0; j < iMatchCount; ++j)
                {
                    if (ppMatchWord[j].compare(sMatchWord) == 0)
                    { //already in list
                        isAlreadyInList = true;
                        break;
                    }
                }
                if (!isAlreadyInList)
                    ppMatchWord[iMatchCount++] = g_strdup(sMatchWord);
            }
        }
    }
    g_pattern_spec_free(pspec);

    if (iMatchCount) // sort it.
        qSort(ppMatchWord, ppMatchWord + iMatchCount, less_for_compare);

    return iMatchCount;
}

bool Libs::lookupData(const QString sWord, QString resList)
{
    QStringList searchWords;
    QString searchWord;
    const char *p = sWord;
    while (*p)
    {
        if (*p == '\\')
        {
            p++;
            switch (*p)
            {
            case ' ':
                SearchWord += ' ';
                break;
            case '\\':
                SearchWord += '\\';
                break;
            case 't':
                SearchWord += '\t';
                break;
            case 'n':
                SearchWord += '\n';
                break;
            default:
                SearchWord += *p;
            }
        }
        else if (*p == ' ')
        {
            if (!SearchWord.empty())
            {
                SearchWords.push_back(SearchWord);
                SearchWord.clear();
            }
        }
        else
        {
            SearchWord += *p;
        }
        p++;
    }

    if (!searchWord.isEmpty())
    {
        searchWords.append(searchWord);
        searchWord.clear();
    }

    if (searchWords.isEmpty())
        return false;

    uint32 smax = 0;
    gchar *originalData = NULL;
    for (QVector<Dict *>::size_type i = 0; i < d->dictionaries.size(); ++i)
    {
        if (!d->dictionaries.at(i)->containSearchData())
            continue;

        if (progress_func)
            progress_func();

        const ulong iwords = narticles(i);
        const gchar *key;
        guint32 offset, size;
        for (ulong j = 0;
                j < iwords;
                ++j)
        {
            d->dictionaries(i)->keyAndData(j, &key, &offset, &size);
            if (size > smax)
            {
                origin_data = (gchar *)g_realloc(origin_data, size);
                smax = size;
            }
            if (d->dictionaries.at(i)->SearchData(SearchWords, offset, size, origin_data))
                reslist[i].push_back(g_strdup(key));
        }
    }
    g_free(origin_data);

    QVector<Dict *>::size_type i;
    for (i = 0; i < d->dictionaries.size(); ++i)
        if (!resList[i].empty())
            break;

    return i != d->dictionaries.size();
}

query_t analyzeQuery(const QString string, QString& result)
{
    if (string.isEmpty() || result.isEmpty())
    {
        result = "";
        return qtSIMPLE;
    }

    if (string.startsWith('/'))
    {
        result = str.mid(1);
        return qtFUZZY;
    }
    else if (string.startsWith('|'))
    {
        result = string.mid(1);
        return qtDATA;
    }

    string.remove('\\');

    if (string.contains('*') || string.contains('?'))
        return qtREGEXP;

    return qtSIMPLE;
}
