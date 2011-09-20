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

#include "stardict.h"

#include "lib.h"
#include "settingsdialog.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QStack>
#include <QtCore/QDebug>

using namespace MulaPluginStarDict;

const int MaxFuzzy = 24;

class IfoListSetter
{
    public:
        IfoListSetter(QStringList bnList)
            : bookNameList(bnList)
        {
        }

        void operator () (const QString &fileName, bool)
        {
            DictionaryInfo info;
            if (info.loadFromIfoFile(fileName, false))
                bookNameList.append(info.bookName());
        }

    private:
        QStringList bookNameList;
};

class IfoFileFinder
{
    public:
        IfoFileFinder(const QString &bookName, QString fileName)
            : ifoBookName(bookName)
            , ifoFileName(fileName)
        {
        }

        void operator () (const QString &fileName, bool)
        {
            DictionaryInfo info;
            if (info.loadFromIfoFile(fileName, false) && info.bookName() == ifoBookName)
                ifoFileName = fileName;
        }

    private:
        QString ifoBookName;
        QString ifoFileName;
};

class StarDict::Private
{
    public:
        Private()
            : sdLibs(new Libs)
            , reformatLists(false)
            , expandAbbreviations(false)
        {   
        }

        ~Private()
        {
        }
 
        Libs *sdLibs;
        QStringList dictionaryDirs;
        QHash<QString, int> loadedDictionaries;
        bool reformatLists;
        bool expandAbbreviations;
};

StarDict::StarDict(QObject *parent)
    : QObject(parent)
{
    QSettings settings("mula","mula");

    d->dictionaryDirs = settings.value("StarDict/dictionaryDirs", m_dictionaryDirs).toStringList();
    d->reformatLists = settings.value("StarDict/reformatLists", true).toBool();
    d->expandAbbreviations = settings.value("StarDict/expandAbbreviations", true).toBool();
    if (d->dictionaryDirs.isEmpty())
    {
#ifdef Q_OS_UNIX
        m_dictionaryDirs.append("/usr/share/stardict/dic");
#else
        m_dictionaryDirs.append(QCoreApplication::applicationDirPath() + "/dic");
#endif			
        m_dictionaryDirs.append(QDir::homePath() + "/.stardict/dic");
    }
}

StarDict::~StarDict()
{
    QSettings settings("mula","mula");

    settings.setValue("StarDict/dictDirs", d->dictionaryDirs);
    settings.setValue("StarDict/reformatLists", d->reformatLists);
    settings.setValue("StarDict/expandAbbreviations", d->expandAbbreviations);

    delete d->sdLibs;
}

QString
StarDict::name() const
{ 
    return "stardict";
}

QString
StarDict::version() const
{
    return "0.1";
}

QString
StarDict::description() const
{
    return "The StarDict plugin";
}

QStringList
StarDict::authors() const
{
    return QStringList() << "Laszlo Papp <lpapp@kde.org>";
}

Features
StarDict::features() const
{
    return Features(SearchSimilar | SettingsDialog);
}

QStringList
StarDict::availableDictionaries() const
{
    QStringList result;
    IfoListSetter setter(&result);
    for_each_file(d->dictionaryDirs, ".ifo", QStringList(), QStringList(), setter);

    return result;
}

QStringList
StarDict::loadedDictionaries() const
{
    return d->loadedDictionaries.keys();
}

void
StarDict::setLoadedDictionaries(const QStringList &loadedDictionaries)
{
    QStringList available = availableDictionaries();
    QStringList disabled;
    for (QStringList::const_iterator i = available.begin(); i != available.end(); ++i)
    {
        if (!loadedDictionaries.contains(*i))
            disabled.append(i);
    }

    d->sdLibs->reload(d->dictionaryDirs, loadedDictionaries, disabled);

    d->loadedDictionaries.clear();
    for (int i = 0; i < d->sdLibs->ndicts(); ++i)
        d->loadedDictionaries[QString::fromUtf8(d->sdLibs->dict_name(i))] = i;
}

MulaCore::DictionaryInfo
StarDict::dictionaryInfo(const QString &dictionary)
{
    DictionaryInfo nativeInfo;
    nativeInfo.wordcount = 0;
    if (!nativeInfo.loadFromIfoFile(findDictionary(dictionary, d->dictionaryDirs), false))
        return DictionaryInfo();

    DictionaryInfo result(name(), dictionary);
    result.setAuthor(QString::fromUtf8(nativeInfo.autho()));
    result.setDescription(QString::fromUtf8(nativeInfo.description()));
    result.setWordsCount(nativeInfo.wordcount() ? static_cast<long>(nativeInfo.wordcount()) : -1);
    return result;
}

bool
StarDict::isTranslatable(const QString &dictionary, const QString &word)
{
    if (!d->loadedDictonaries.contains(dictionary))
        return false;

    long ind;
    return d->sdLibs->simpleLookupWord(word.toUtf8().data(), ind, d->loadedDicts[dictionary]);
}

StarDict::Translation
StarDict::translate(const QString &dictionary, const QString &word)
{
    if (!d->loadedDicts.contains(dictionary))
        return Translation();

    if (word.isEmpty())
        return Translation();

    int dictionaryIndex = m_loadedDicts[dictionary];
    long ind;

    if (!d->sdLibs->simpleLookupWord(word.toUtf8().data(), ind, d->loadedDicts[dict]))
        return Translation();

    return Translation(QString::fromUtf8(d->sdLibs->poWord(ind, dictionaryIndex)),
            QString::fromUtf8(d->sdLibs->dict_name(dictionaryIndex)),
            parseData(d->sdLibs->poWordData(ind, dictionaryIndex), dictionaryIndex, true,
                d->reformatLists, d->expandAbbreviations));
}

QStringList
StarDict::findSimilarWords(const QString &dictionary, const QString &word)
{
    if (!d->loadedDictionaries.contains(dictionary))
        return QStringList();

    char *fuzzy_res[MaxFuzzy];
    if (!d->sdLibs->LookupWithFuzzy(word.toUtf8().data(), fuzzy_res, MaxFuzzy, m_loadedDicts[dict]))
        return QStringList();

    QStringList result;
    for (char **p = fuzzy_res, **end = fuzzy_res + MaxFuzzy; p != end && *p; ++p)
    {
        result << QString::fromUtf8(*p);
        g_free(*p);
    }

    return result;
}

int
StarDict::execSettingsDialog(QWidget *parent)
{
    SettingsDialog dialog(this, parent);
    return dialog.exec();
}

QString
StarDict::parseData(const char *data, int dictIndex, bool htmlSpaces, bool reformatLists, bool expandAbbreviations)
{
    QString result;
    quint32 dataSize = *reinterpret_cast<const quint32*>(data);
    const char *dataEnd = data + dataSize;
    const char *ptr = data + sizeof(quint32);
    while (ptr < dataEnd)
    {
        switch (*ptr++)
        {
            case 'm':
            case 'l':
            case 'g':
            {
                QString str = QString::fromUtf8(ptr);
                ptr += str.toUtf8().length() + 1;
                result += str;
                break;
            }
            case 'x':
            {
                QString str = QString::fromUtf8(ptr);
                ptr += str.toUtf8().length() + 1;
                xdxf2html(str);
                result += str;
                break;
            }
            case 't':
            {
                QString str = QString::fromUtf8(ptr);
                ptr += str.toUtf8().length() + 1;
                result += "<font class=\"example\">";
                result += str;
                result += "</font>";
                break;
            }
            case 'y':
            {
                ptr += strlen(ptr) + 1;
                break;
            }
            case 'W':
            case 'P':
            {
                ptr += *reinterpret_cast<const quint32*>(ptr) + sizeof(quint32);
                break;
            }
            default:
                ; // nothing
        }
    }

    if (d->expandAbbreviations)
    {
        QRegExp regExp("_\\S+[\\.:]");
        int pos = 0;
        while ((pos = regExp.indexIn(result, pos)) != -1)
        {
            long ind;
            if (m_sdLibs->SimpleLookupWord(result.mid(pos, regExp.matchedLength()).toUtf8().data(), ind, dictIndex))
            {
                QString expanded = "<font class=\"explanation\">";
                expanded += parseData(m_sdLibs->poGetWordData(ind, dictIndex));
                if (result[pos + regExp.matchedLength() - 1] == ':')
                    expanded += ':';
                expanded += "</font>";
                result.replace(pos, regExp.matchedLength(), expanded);
                pos += expanded.length();
            }
            else
                pos += regExp.matchedLength();
        }
    }
    if (reformatLists)
    {
        int pos = 0;
        QStack<QChar> openedLists;
        while (pos < result.length())
        {
            if (result[pos].isDigit())
            {
                int n = 0;
                while (result[pos + n].isDigit())
                    ++n;
                pos += n;
                if (result[pos] == '&' && result.mid(pos + 1, 3) == "gt;")
                    result.replace(pos, 4, ">");
                QChar marker = result[pos];
                QString replacement;
                if (marker == '>' || marker == '.' || marker == ')')
                {
                    if (n == 1 && result[pos - 1] == '1') // open new list
                    {
                        if (openedLists.contains(marker))
                        {
                            replacement = "</li></ol>";
                            while (openedLists.size() && openedLists.top() != marker)
                            {
                                replacement += "</li></ol>";
                                openedLists.pop();
                            }
                        }
                        openedLists.push(marker);
                        replacement += "<ol>";
                    }
                    else
                    {
                        while (openedLists.size() && openedLists.top() != marker)
                        {
                            replacement += "</li></ol>";
                            openedLists.pop();
                        }
                        replacement += "</li>";
                    }
                    replacement += "<li>";
                    pos -= n;
                    n += pos;

                    while (result[pos - 1].isSpace())
                        --pos;

                    while (result[n + 1].isSpace())
                        ++n;

                    result.replace(pos, n - pos + 1, replacement);
                    pos += replacement.length();
                }
                else
                    ++pos;
            }
            else
                ++pos;
        }
        while (openedLists.size())
        {
            result += "</li></ol>";
            openedLists.pop();
        }
    }

    if (htmlSpaces)
    {
        int n = 0;
        while (result[n].isSpace())
            ++n;

        result.remove(0, n);
        n = 0;
        while (result[result.length() - 1 - n].isSpace())
            ++n;

        result.remove(result.length() - n, n);

        for (int pos = 0; pos < result.length();)
        {
            switch (result[pos].toAscii())
            {
                case '[':
                    result.insert(pos, "<font class=\"transcription\">");
                    pos += 28 + 1; // sizeof "<font class=\"transcription\">" + 1
                    break;
                case ']':
                    result.insert(pos + 1, "</font>");
                    pos += 7 + 1; // sizeof "</font>" + 1
                    break;
                case '\t':
                    result.insert(pos, "&nbsp;&nbsp;&nbsp;&nbsp;");
                    pos += 24 + 1; // sizeof "&nbsp;&nbsp;&nbsp;&nbsp;" + 1
                    break;
                case '\n':
                {
                    int count = 1;
                    n = 1;
                    while (result[pos + n].isSpace())
                    {
                        if (result[pos + n] == '\n')
                            ++count;
                        ++n;
                    }
                    if (count > 1)
                        result.replace(pos, n, "</p><p>");
                    else
                        result.replace(pos, n, "<br>");

                    break;
                }
                default:
                    ++pos;
            }
        }
    }
    return result;
}

QString
StarDict::findDictionary(const QString &name, const QStringList &dictionaryDirs)
{
    QString filename;
    IfoFileFinder finder(name, &filename);
    for_each_file(d->dictionaryDirs, ".ifo", QStringList(), QStringList(), finder);
    return filename;
}

void
QString::xdxf2html(QString &str)
{
    str.replace("<abr>", "<font class=\"abbreviature\">");
    str.replace("<tr>", "<font class=\"transcription\">[");
    str.replace("</tr>", "]</font>");
    str.replace("<ex>", "<font class=\"example\">");
    str.replace(QRegExp("<k>.*<\\/k>"), "");
    str.replace(QRegExp("(<\\/abr>)|(<\\ex>)"), "</font");
}

Q_EXPORT_PLUGIN2(stardict, StarDict)
