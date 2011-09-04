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

#include "lib.h"

#include "distance.h"

#include <QtAlgorithms>
#include <QtCore/QString>

#include <zlib.h>

// Notice: read src/tools/DICTFILE_FORMAT for the dictionary
// file's format information!

static bool isVowel(QChar inputChar)
{
    QChar ch = inputChar.toUpper();
    return ( ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U' );
}

static bool isPureEnglish(const QString str)
{
    // i think this should work even when it is UTF8 string :).
    for (int i = 0; str[i] != 0; i++)
        //if(str[i]<0)
        //if(str[i]<32 || str[i]>126) // tab equal 9,so this is not OK.
        // Better use isascii() but not str[i]<0 while char is default unsigned in arm
        if (!isascii(str[i]))
            return false;
    return true;
}

static inline qint stardictStringCompare(const QString str1, const QString str2)
{
    qint a = s1.compare(s2, Qt::CaseInsensitive);
    if (a == 0)
        return s1.compare(s2);
    else
        return a;
}

class Libs::Private
{
    public:
        Private()
            : iMaxFuzzyDistance(MAX_FUZZY_DISTANCE) // need to read from cfg
        {
        }   

        ~Private()
        {   
        }   
 
        QVector<Dictionary *> oLib; // word Libs.
        int iMaxFuzzyDistance;
        progress_func_t progress_func;
}

Libs::Libs(progress_func_t f)
    : d(new Private)
{
}

Libs::~Libs()
{
    for (QVector<Dict *>::iterator p = oLib.begin(); p != oLib.end(); ++p)
        delete *p;
}

void Libs::loadDictionary(const QString& url)
{
    Dictionary *lib = new Dictionary;
    if (lib->load(url))
        oLib.append(lib);
    else
        delete lib;
}

qlong
Libs::articleCount(int dictionaryIndex) const
{
    return oLib.at(dictionaryIndex)->articleCount();
}

const QString&
Libs::dictionaryName(int index) const
{
    return oLib.at(index)->dictionaryName();
}

qint
Libs::dictionaryCount() const
{
    return oLib.size();
}

const QByteArray
Libs::poWord(qlong keyIndex, int libIndex) const
{
    return oLib.at(libIndex)->key(keyIndex);
}

QString
Libs::poWordData(qlong dataIndex, int libIndex)
{
    if (iIndex == INVALID_INDEX)
        return NULL;

    return oLib.at(libIndex)->data(dataIndex);
}

bool
Libs::lookupWord(const char* sWorda, qlong& iWordIndex, int libIndex)
{
    return oLib.at(libIndex)->lookup(sWord, iWordIndex);
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
        DictionaryReLoader(QVector<Dict *> &p, QVector<Dict *> &f,
                     Libs& lib_)
            : prev(p)
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
                    future.append(dict);
                else
                    lib.loadDictionary(url);
            }
        }

    private:
        QVector<Dict *> &prev;
        QVector<Dict *> &future;
        Libs& lib;

        Dictionary *find(const QString& url)
        {
            QVector<Dict *>::iterator it;
            for (it = prev.begin(); it != prev.end(); ++it)
                if ((*it)->ifoFileName() == url)
                    break;

            if (it != prev.end())
            {
                Dict *res = *it;
                prev.erase(it);
                return res;
            }
            return NULL;
        }
};

void
Libs::reload(const QStringList& dictionaryDirs,
                  const QStringList& orderList,
                  const QStringList& disableList)
{
    QVector<Dict *> prev(oLib);
    oLib.clear();
    for_each_file(dictionaryDirs, ".ifo", orderList, disableList,
                  DictionaryReLoader(prev, oLib, *this));

    for (QVector<Dict *>::iterator it = prev.begin(); it != prev.end(); ++it)
        delete *it;
}

const char *
Libs::poCurrentWord(qlong * iCurrent)
{
    const char *poCurrentWord = NULL;
    const char *word;

    for (QVector<Dict *>::size_type iLib = 0; iLib<oLib.size(); iLib++)
    {
        if (iCurrent[iLib] == INVALID_INDEX)
            continue;

        if (iCurrent[iLib] >= narticles(iLib) || iCurrent[iLib] < 0)
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
Libs::poNextWord(const char *sWord, qlong *iCurrent)
{
    // the input can be:
    // (word,iCurrent),read word,write iNext to iCurrent,and return next word. used by TopWin::NextCallback();
    // (NULL,iCurrent),read iCurrent,write iNext to iCurrent,and return next word. used by AppCore::ListWords();
    const char *poCurrentWord = NULL;
    QVector<Dict *>::size_type iCurrentLib = 0;
    const char *word;

    for (QVector<Dict *>::size_type iLib = 0;iLib<oLib.size(); ++iLib)
    {
        if (sWord)
            oLib[iLib]->Lookup(sWord, iCurrent[iLib]);

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

            if (stardictStrcmp(poCurrentWord, word) > 0 )
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
        for (std::vector<Dict *>::size_type iLib = 0;iLib<oLib.size();iLib++)
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
Libs::poPreviousWord(qlong *iCurrent)
{
    // used by TopWin::PreviousCallback(); the iCurrent is cached by AppCore::TopWinWordChange();
    const gchar *poCurrentWord = NULL;
    QVector<Dict *>::size_type iCurrentLib = 0;
    const char *word;

    for (QVector<Dict *>::size_type iLib = 0;iLib<oLib.size(); ++iLib)
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
        for (QVector<Dict *>::size_type iLib = 0;iLib<oLib.size(); ++iLib)
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
            bool isUpperCase = !strncmp(`&sWord[wordLength - 2], "LY", 2);
            if (!originiWordPart.compare("LY", Qt::CaseSensitive) || !originiWordPart.compare("ly", Qt::CaseSensitive))
            {
                sNewWord = sWord;
                sNewWord[iWordLen - 2] = '\0';  // cut "ly"
                if (iWordLen > 5 && sNewWord[iWordLen - 3] == sNewWord[iWordLen - 4]
                        && !isVowel(sNewWord[iWordLen - 4]) && isVowel(sNewWord[iWordLen - 5]))
                { //doubled

                    sNewWord[iWordLen - 3] = '\0';
                    if ( oLib[iLib]->Lookup(sNewWord, iIndex) )
                        bFound = true;
                    else
                    {
                        if (isUpperCase || sWord.at(0).isUpper())
                        {
                            caseString = sNewWord.toLower();
                            if (caseString.compare(sNewWord))
                            {
                                if (oLib[iLib]->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }
                        if (!found)
                            sNewWord[iWordLen - 3] = sNewWord[iWordLen - 4];  //restore
                    }
                }

                if (!found)
                {
                    if (oLib[iLib]->lookup(sNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || sWord.at(0).isUpper())
                    {
                        caseString = sNewWord.toLower();
                        if (caseString.compare(sNewWord))
                        {
                            if (oLib[iLib]->lookup(caseString, iIndex))
                                found = true;
                        }
                    }
                }
            }
        }
}

bool Libs::lookupSimilarWord(const QString sWord, qlong & iWordIndex, int iLib)
{
    qlong iIndex;
    bool found = false;
    QString caseString;

    if (!found)
    {
        // to lower case.
        caseString = sWord.toLower();
        if (caseString.compare(sWord))
        {
            if (oLib[iLib]->lookup(caseString, iIndex))
                found = true;
        }

        // to upper case.
        if (!found)
        {
            caseString = sWord.toUpper();
            if (caseString.compare(sWord))
            {
                if (oLib[iLib]->Lookup(caseString, iIndex))
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
                if (oLib[iLib]->lookup(caseString, iIndex))
                    found = true;
            }
        }
    }

    if (isPureEnglish(sWord))
    {
        // If not Found, try other status of sWord.
        int iWordLen = strlen(sWord);
        bool isupcase;

        gchar *sNewWord = (gchar *)g_malloc(iWordLen + 1);

        //cut one char "s" or "d"
        if (!found && iWordLen > 1)
        {
            isUpperCase = sWord[iWordLen - 1] == 'S' || !strncmp(&sWord[iWordLen - 2], "ED", 2);
            if (isUpperCase || sWord[iWordLen - 1] == 's' || !strncmp(&sWord[iWordLen - 2], "ed", 2))
            {
                strcpy(sNewWord, sWord);
                sNewWord[iWordLen - 1] = '\0'; // cut "s" or "d"
                if (oLib[iLib]->Lookup(sNewWord, iIndex))
                {
                    found = true;
                }

                else if (isupcase || g_ascii_isupper(sWord[0]))
                {
                    caseString = sNewWord.toLower();
                    if (caseStrin.compare(sNewWord))
                    {
                        if (oLib[iLib]->lookup(caseString, iIndex))
                            found = true;
                    }
                }
            }
        }

        //cut "ly"
        if (!found && iWordLen > 2)
        {
            isupcase = !strncmp(&sWord[iWordLen - 2], "LY", 2);
            if (isupcase || (!strncmp(&sWord[iWordLen - 2], "ly", 2)))
            {
                sNewWord = sWord;
                sNewWord[iWordLen - 2] = '\0';  // cut "ly"
                if (iWordLen > 5 && sNewWord[iWordLen - 3] == sNewWord[iWordLen - 4]
                        && !isVowel(sNewWord[iWordLen - 4]) && isVowel(sNewWord[iWordLen - 5]))
                { //doubled

                    sNewWord[iWordLen - 3] = '\0';
                    if ( oLib[iLib]->Lookup(sNewWord, iIndex) )
                        bFound = true;
                    else
                    {
                        if (isUpperCase || sWord.at(0).isUpper())
                        {
                            caseString = sNewWord.toLower();
                            if (caseString.compare(sNewWord))
                            {
                                if (oLib[iLib]->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }
                        if (!found)
                            sNewWord[iWordLen - 3] = sNewWord[iWordLen - 4];  //restore
                    }
                }

                if (!found)
                {
                    if (oLib[iLib]->lookup(sNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || sWord.at(0).isUpper())
                    {
                        caseString = sNewWord.toLower();
                        if (caseString.compare(sNewWord))
                        {
                            if (oLib[iLib]->lookup(caseString, iIndex))
                                found = true;
                        }
                    }
                }
            }
        }

        //cut "ing"
        if (!found && iWordLen > 3)
        {
            isupcase = !strncmp(&sWord[iWordLen - 3], "ING", 3);
            if (isupcase || !strncmp(&sWord[iWordLen - 3], "ing", 3) )
            {
                strcpy(sNewWord, sWord);
                sNewWord[iWordLen - 3] = '\0';
                if ( iWordLen > 6 && (sNewWord[iWordLen - 4] == sNewWord[iWordLen - 5])
                        && !bIsVowel(sNewWord[iWordLen - 5]) &&
                        bIsVowel(sNewWord[iWordLen - 6]))
                {  //doubled
                    sNewWord[iWordLen - 4] = '\0';
                    if (oLib[iLib]->Lookup(sNewWord, iIndex))
                        bFound = true;
                    else
                    {
                        if (isupcase || g_ascii_isupper(sWord[0]))
                        {
                            casestr = g_ascii_strdown(sNewWord, -1);
                            if (strcmp(casestr, sNewWord))
                            {
                                if (oLib[iLib]->Lookup(casestr, iIndex))
                                    bFound = true;
                            }
                            g_free(casestr);
                        }
                        if (!bFound)
                            sNewWord[iWordLen - 4] = sNewWord[iWordLen - 5];  //restore
                    }
                }

                if ( !found )
                {
                    if (oLib[iLib]->Lookup(sNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || sWord[0].isUpper())
                    {
                        caseString = sNewWord.toLower();
                        if (caseString.compare(sNewWord))
                        {
                            if (oLib[iLib]->lookup(caseString, iIndex))
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

                    if (oLib[iLib]->lookup(sNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || sWord[0].isUpper())
                    {
                        caseString = sNewWord.toLower();
                        if (caseString.compare(sNewWord))
                        {
                            if (oLib[iLib]->lookup(caseString, iIndex))
                                found = true;
                        }
                    }
                }
            }
        }

        //cut two char "es"
        if (!found && iWordLen > 3)
        {
            isUpperCase = (!sWord.mid(iWordLen - 2, 2).compare("ES")
                        && sWord.at(iWordLen - 3) == 'S'
                        || sWord.at(iWordLen - 3) == 'X'
                        || sWord.at(iWordLen - 3) == 'O'
                        || (iWordLen > 4 && sWord.at(iWordLen - 3) == 'H'
                        && (sWord.at(iWordLen - 4) == 'C'
                        || sWord.at(iWordLen - 4) == 'S')));

            if (isUpperCase ||
                    (!sWord.mid(iWordLen - 2, 2).compare("es")
                     && (sWord.at(iWordLen - 3) == 's' || sWord.at(iWordLen - 3) == 'x'
                     || sWord.at(iWordLen - 3) == 'o'
                     || (iWordLen > 4 && sWord.at(iWordLen - 3) == 'h'
                     && (sWord.at(iWordLen - 4) == 'c' || sWord.at(iWordLen - 4) == 's')))))
            {
                sNewWord = sWord;
                sNewWord[iWordLen - 2] = '\0';
                if (oLib[iLib]->lookup(sNewWord, iIndex))
                {
                    found = true;
                }
                else if (isUpperCase || sWord[0].isUpper())
                {
                    caseString = sNewWord.toLower();
                    if (caseString.compare(sNewWord))
                    {
                        if (oLib[iLib]->lookup(caseString, iIndex))
                            found = true;
                    }
                }
            }
        }

        //cut "ed"
        if (!found && iWordLen > 3)
        {
            isUpperCase = !sWord.mid(iWordLen - 2, 2).compare("ED");
            if (isUpperCase || !sWord.mid(iWordLen - 2, 2).compare("ed"))
            {
                strcpy(sNewWord, sWord);
                sNewWord[iWordLen - 2] = '\0';
                if (iWordLen > 5 && (sNewWord[iWordLen - 3] == sNewWord[iWordLen - 4])
                        && !bIsVowel(sNewWord[iWordLen - 4]) &&
                        bIsVowel(sNewWord[iWordLen - 5]))
                { //doubled
                    sNewWord[iWordLen - 3] = '\0';
                    if (oLib[iLib]->lookup(sNewWord, iIndex))
                    {
                        found = true;
                    }
                    else
                    {
                        if (isUpperCase || sWord[0].isUpper())
                        {
                            caseString = sNewWord.toLower();
                            if (caseString.compare(sNewWord))
                            {
                                if (oLib[iLib]->lookup(caseString, iIndex))
                                    found = true;
                            }
                        }
                        if (!found)
                            sNewWord[iWordLen - 3] = sNewWord[iWordLen - 4];  //restore
                    }
                }

                if (!found)
                {
                    if (oLib[iLib]->Lookup(sNewWord, iIndex))
                    {
                        found = true;
                    }
                    else if (isUpperCase || sWord[0].isUpper())
                    {
                        caseString = sNewWord.toLower();
                        if (caseString.compare(sNewWord))
                        {
                            if (oLib[iLib]->lookup(casestring, iIndex))
                                found = true;
                        }
                    }
                }
            }
        }

        // cut "ied" , add "y".
        if (!bFound && iWordLen > 3)
        {
            isupcase = !strncmp(&sWord[iWordLen - 3], "IED", 3);
            if (isupcase || (!strncmp(&sWord[iWordLen - 3], "ied", 3)))
            {
                strcpy(sNewWord, sWord);
                sNewWord[iWordLen - 3] = '\0';
                if (isupcase)
                    strcat(sNewWord, "Y"); // add a char "Y"
                else
                    strcat(sNewWord, "y"); // add a char "y"
                if (oLib[iLib]->Lookup(sNewWord, iIndex))
                    bFound = true;
                else if (isupcase || g_ascii_isupper(sWord[0]))
                {
                    casestr = g_ascii_strdown(sNewWord, -1);
                    if (strcmp(casestr, sNewWord))
                    {
                        if (oLib[iLib]->Lookup(casestr, iIndex))
                            bFound = true;
                    }
                    g_free(casestr);
                }
            }
        }

        // cut "ies" , add "y".
        if (!bFound && iWordLen > 3)
        {
            isupcase = !strncmp(&sWord[iWordLen - 3], "IES", 3);
            if (isupcase || (!strncmp(&sWord[iWordLen - 3], "ies", 3)))
            {
                strcpy(sNewWord, sWord);
                sNewWord[iWordLen - 3] = '\0';
                if (isupcase)
                    strcat(sNewWord, "Y"); // add a char "Y"
                else
                    strcat(sNewWord, "y"); // add a char "y"
                if (oLib[iLib]->Lookup(sNewWord, iIndex))
                    bFound = true;
                else if (isupcase || g_ascii_isupper(sWord[0]))
                {
                    casestr = g_ascii_strdown(sNewWord, -1);
                    if (strcmp(casestr, sNewWord))
                    {
                        if (oLib[iLib]->Lookup(casestr, iIndex))
                            bFound = true;
                    }
                    g_free(casestr);
                }
            }
        }

        // cut "er".
        if (!bFound && iWordLen > 2)
        {
            isupcase = !strncmp(&sWord[iWordLen - 2], "ER", 2);
            if (isupcase || (!strncmp(&sWord[iWordLen - 2], "er", 2)))
            {
                strcpy(sNewWord, sWord);
                sNewWord[iWordLen - 2] = '\0';
                if (oLib[iLib]->Lookup(sNewWord, iIndex))
                    bFound = true;
                else if (isupcase || g_ascii_isupper(sWord[0]))
                {
                    casestr = g_ascii_strdown(sNewWord, -1);
                    if (strcmp(casestr, sNewWord))
                    {
                        if (oLib[iLib]->Lookup(casestr, iIndex))
                            bFound = true;
                    }
                    g_free(casestr);
                }
            }
        }

        // cut "est".
        if (!bFound && iWordLen > 3)
        {
            isupcase = !strncmp(&sWord[iWordLen - 3], "EST", 3);
            if (isupcase || (!strncmp(&sWord[iWordLen - 3], "est", 3)))
            {
                strcpy(sNewWord, sWord);
                sNewWord[iWordLen - 3] = '\0';
                if (oLib[iLib]->Lookup(sNewWord, iIndex))
                    bFound = true;
                else if (isupcase || g_ascii_isupper(sWord[0]))
                {
                    casestr = g_ascii_strdown(sNewWord, -1);
                    if (strcmp(casestr, sNewWord))
                    {
                        if (oLib[iLib]->Lookup(casestr, iIndex))
                            bFound = true;
                    }
                }
            }
        }
    }

    if (bFound)
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

bool Libs::simpleLookupWord(const QString sWord, qlong &iWordIndex, int iLib)
{
    bool found = oLib[iLib]->lookup(sWord, iWordIndex);
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

    qlong iCheckWordLen;
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

qint Libs::lookupWithRule(const gchar *word, gchar **ppMatchWord)
{
    glong aiIndex[MAX_MATCH_ITEM_PER_LIB + 1];
    gint iMatchCount = 0;
    GPatternSpec *pspec = g_pattern_spec_new(word);

    for (QVector<Dict *>::size_type iLib = 0; iLib<oLib.size(); ++iLib)
    {
        //if(oLibs.LookdupWordsWithRule(pspec,aiIndex,MAX_MATCH_ITEM_PER_LIB+1-iMatchCount,iLib))
        // -iMatchCount,so save time,but may got less result and the word may repeat.

        if (oLib[iLib]->LookupWithRule(pspec, aiIndex, MAX_MATCH_ITEM_PER_LIB + 1))
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
    for (QVector<Dict *>::size_type i = 0; i<oLib.size(); ++i)
    {
        if (!oLib[i]->
                containSearchData())
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
            oLib[i]
            ->keyAndData(j, &key, &offset, &size);
            if (size > smax)
            {
                origin_data = (gchar *)g_realloc(origin_data, size);
                smax = size;
            }
            if (oLib[i]->SearchData(SearchWords, offset, size, origin_data))
                reslist[i].push_back(g_strdup(key));
        }
    }
    g_free(origin_data);

    QVector<Dict *>::size_type i;
    for (i = 0; i<oLib.size(); ++i)
        if (!resList[i].empty())
            break;

    return i != oLib.size();
}

query_t analyzeQuery(const QString str, QString& res)
{
    if (str.isEmpty() || res.isEmpty())
    {
        res = "";
        return qtSIMPLE;
    }

    if (str.startsWith('/'))
    {
        res = str.mid(1);
        return qtFUZZY;
    }
    else if (str.startsWith('|'))
    {
        res = str.mid(1);
        return qtDATA;
    }

    str.remove('\\');

    if (str.contains('*') || str.contains('?'))
        return qtREGEXP;

    return qtSIMPLE;
}
