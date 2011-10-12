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

#include "settingsdialog.h"
#include "file.h"

#include <core/dictionaryplugin.h>

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

class StarDict::Private
{
    public:
        Private()
            : dictionaryManager(new StarDictDictionaryManager)
            , reformatLists(false)
            , expandAbbreviations(false)
        {
        }

        ~Private()
        {
        }

        StarDictDictionaryManager *dictionaryManager;
        QStringList dictionaryDirectoryList;
        QHash<QString, int> loadedDictionaries;
        bool reformatLists;
        bool expandAbbreviations;

        QString ifoBookName;
        QString ifoFileName;

        const static int maximumFuzzy = 24;
};

StarDict::StarDict(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    QSettings settings("mula","mula");

    d->dictionaryDirectoryList = settings.value("StarDict/dictionaryDirectoryList", d->dictionaryDirectoryList).toStringList();
    d->reformatLists = settings.value("StarDict/reformatLists", true).toBool();
    d->expandAbbreviations = settings.value("StarDict/expandAbbreviations", true).toBool();
    if (d->dictionaryDirectoryList.isEmpty())
    {
#ifdef Q_OS_UNIX
        d->dictionaryDirectoryList.append("/usr/share/stardict/dic");
#else
        d->dictionaryDirectoryList.append(QCoreApplication::applicationDirPath() + "/dic");
#endif
        d->dictionaryDirectoryList.append(QDir::homePath() + "/.stardict/dic");
    }
}

StarDict::~StarDict()
{
    QSettings settings("mula","mula");

    settings.setValue("StarDict/dictionaryDirectoryList", d->dictionaryDirectoryList);
    settings.setValue("StarDict/reformatLists", d->reformatLists);
    settings.setValue("StarDict/expandAbbreviations", d->expandAbbreviations);

    delete d->dictionaryManager;
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

MulaCore::DictionaryPlugin::Features
StarDict::features() const
{
    return MulaCore::DictionaryPlugin::Features(SearchSimilar | SettingsDialog);
}

QString
StarDict::findAvailableDictionary(const QString& absolutePath)
{
    StarDictDictionaryInfo info;
    if (info.loadFromIfoFile(absolutePath, false))
        return info.bookName();

    return QString();
}

QString
StarDict::findIfoFile(const QString& absolutePath)
{
    StarDictDictionaryInfo info;
    if (info.loadFromIfoFile(absolutePath, false) && info.bookName() == d->ifoBookName)
        d->ifoFileName = absolutePath;

    return QString();
}

template <typename Method>
QStringList
StarDict::recursiveTemplateFind(const QString& directoryPath, Method method)
{
    QDir dir(directoryPath);
    QStringList result;

    // Going through the subfolders
    foreach (const QString& entryName, dir.entryList(QDir::Dirs & QDir::NoDotAndDotDot))
    {
        QString absolutePath = dir.absoluteFilePath(entryName);
        result.append(recursiveTemplateFind(absolutePath, method));
    }

    foreach (const QString& entryName, dir.entryList(QDir::Files & QDir::Drives & QDir::NoDotAndDotDot))
    {
        QString absolutePath = dir.absoluteFilePath(entryName);
        if (absolutePath.endsWith(QLatin1String(".ifo")))
            result.append((this->*method)(absolutePath));
    }

    return result;
}

QStringList
StarDict::availableDictionaryList()
{
    QStringList result;

    foreach (const QString& directoryPath, d->dictionaryDirectoryList)
       result.append(recursiveTemplateFind(directoryPath, &StarDict::findAvailableDictionary));

    return result;
}

QStringList
StarDict::loadedDictionaryList() const
{
    return d->loadedDictionaries.keys();
}

void
StarDict::setLoadedDictionaryList(const QStringList &loadedDictionaryList)
{
    QStringList availableDictionaries = availableDictionaryList();
    QStringList disabledDictionaries;
    foreach (const QString& dictionary, availableDictionaries)
    {
        if (!loadedDictionaryList.contains(dictionary))
            disabledDictionaries.append(dictionary);
    }

    d->dictionaryManager->reload(d->dictionaryDirectoryList, loadedDictionaryList, disabledDictionaries);

    d->loadedDictionaries.clear();
    for (int i = 0; i < d->dictionaryManager->dictionaryCount(); ++i)
        d->loadedDictionaries[d->dictionaryManager->dictionaryName(i)] = i;
}

MulaCore::DictionaryInfo
StarDict::dictionaryInfo(const QString &dictionary)
{
    StarDictDictionaryInfo nativeInfo;
    nativeInfo.setWordCount(0);
    if (!nativeInfo.loadFromIfoFile(findDictionary(dictionary, d->dictionaryDirectoryList), false))
        return MulaCore::DictionaryInfo();

    MulaCore::DictionaryInfo result(name(), dictionary);
    result.setAuthor(nativeInfo.author());
    result.setDescription(nativeInfo.description());
    result.setWordCount(nativeInfo.wordCount() ? static_cast<long>(nativeInfo.wordCount()) : -1);
    return result;
}

bool
StarDict::isTranslatable(const QString &dictionary, const QString &word)
{
    if (!d->loadedDictionaries.contains(dictionary))
        return false;

    int index;
    return d->dictionaryManager->simpleLookupWord(word.toUtf8().data(), index, d->loadedDictionaries[dictionary]);
}

MulaCore::Translation
StarDict::translate(const QString &dictionary, const QString &word)
{
    if (!d->loadedDictionaries.contains(dictionary) || word.isEmpty())
        return MulaCore::Translation();

    int dictionaryIndex = d->loadedDictionaries[dictionary];
    int index;

    if (!d->dictionaryManager->simpleLookupWord(word.toUtf8().data(), index, d->loadedDictionaries[dictionary]))
        return MulaCore::Translation();

    return MulaCore::Translation(QString::fromUtf8(d->dictionaryManager->poWord(index, dictionaryIndex)),
            d->dictionaryManager->dictionaryName(dictionaryIndex),
            parseData(d->dictionaryManager->poWordData(index, dictionaryIndex).toUtf8(), dictionaryIndex, true,
                d->reformatLists, d->expandAbbreviations));
}

QStringList
StarDict::findSimilarWords(const QString &dictionary, const QString &word)
{
    if (!d->loadedDictionaries.contains(dictionary))
        return QStringList();

    QStringList fuzzyList;
    if (!d->dictionaryManager->lookupWithFuzzy(word.toUtf8(), fuzzyList, d->maximumFuzzy, d->loadedDictionaries[dictionary]))
        return QStringList();

    fuzzyList.reserve(d->maximumFuzzy);

    return fuzzyList;
}

// int
// StarDict::execSettingsDialog(QWidget *parent)
// {
    // MulaPluginStarDict::SettingsDialog dialog(this, parent);
    // return dialog.exec();
// }

QString
StarDict::parseData(const QByteArray &data, int dictionaryIndex, bool htmlSpaces, bool reformatLists, bool expandAbbreviations)
{
    Q_UNUSED(expandAbbreviations);

    QString result;
    int position;

    foreach (char ch, data)
    {
        switch (ch)
        {
            case 'm':
            case 'l':
            case 'g':
            {
                position += data.length() + 1;
                result.append(QString::fromUtf8(data));
                break;
            }

            case 't':
            {
                position += data.length() + 1;
                result.append("<font class=\"example\">");
                result.append(QString::fromUtf8(data));
                result.append("</font>");
                break;
            }

            case 'x':
            {
                QString string = QString::fromUtf8(data);
                position += data.length() + 1;
                xdxf2html(string);
                result.append(string);
                break;
            }

            case 'y':
            {
                position += data.length() + 1;
                break;
            }

            case 'W':
            case 'P':
            {
                position += *reinterpret_cast<const quint32*>(data.data()) + sizeof(quint32);
                break;
            }
            default:
                ; // nothing
        }
    }

    if (d->expandAbbreviations)
    {
        QRegExp regExp("_\\S+[\\.:]");
        int position = 0;
        while ((position = regExp.indexIn(result, position)) != -1)
        {
            int index;
            if (d->dictionaryManager->simpleLookupWord(result.mid(position, regExp.matchedLength()).toUtf8().data(), index, dictionaryIndex))
            {
                QString expanded = "<font class=\"explanation\">";
                expanded += parseData(d->dictionaryManager->poWordData(index, dictionaryIndex).toUtf8());
                if (result[position + regExp.matchedLength() - 1] == ':')
                    expanded += ':';

                expanded += "</font>";
                result.replace(position, regExp.matchedLength(), expanded);
                position += expanded.length();
            }
            else
                position += regExp.matchedLength();
        }
    }

    if (reformatLists)
    {
        int position = 0;
        QStack<QChar> openedLists;
        while (position < result.length())
        {
            if (result[position].isDigit())
            {
                int n = 0;
                while (result[position + n].isDigit())
                    ++n;

                position += n;
                if (result[position] == '&' && result.mid(position + 1, 3) == "gt;")
                    result.replace(position, 4, ">");

                QChar marker = result[position];
                QString replacement;
                if (marker == '>' || marker == '.' || marker == ')')
                {
                    if (n == 1 && result[position - 1] == '1') // open new list
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
                    position -= n;
                    n += position;

                    while (result[position - 1].isSpace())
                        --position;

                    while (result[n + 1].isSpace())
                        ++n;

                    result.replace(position, n - position + 1, replacement);
                    position += replacement.length();
                }
                else
                    ++position;
            }
            else
                ++position;
        }
        while (openedLists.size())
        {
            result.append("</li></ol>");
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

        for (int position = 0; position < result.length();)
        {
            switch (result[position].toAscii())
            {
                case '[':
                    result.insert(position, "<font class=\"transcription\">");
                    position += 28 + 1; // sizeof "<font class=\"transcription\">" + 1
                    break;
                case ']':
                    result.insert(position + 1, "</font>");
                    position += 7 + 1; // sizeof "</font>" + 1
                    break;
                case '\t':
                    result.insert(position, "&nbsp;&nbsp;&nbsp;&nbsp;");
                    position += 24 + 1; // sizeof "&nbsp;&nbsp;&nbsp;&nbsp;" + 1
                    break;
                case '\n':
                {
                    int count = 1;
                    n = 1;
                    while (result[position + n].isSpace())
                    {
                        if (result[position + n] == '\n')
                            ++count;
                        ++n;
                    }

                    if (count > 1)
                        result.replace(position, n, "</p><p>");
                    else
                        result.replace(position, n, "<br>");

                    break;
                }
                default:
                    ++position;
            }
        }
    }
    return result;
}

QString
StarDict::findDictionary(const QString &name, const QStringList &dictionaryDirectoryList)
{
    d->ifoBookName = name;
    foreach (const QString& directoryPath, dictionaryDirectoryList)
       recursiveTemplateFind(directoryPath, &StarDict::findIfoFile);

    return d->ifoFileName;
}

void
StarDict::xdxf2html(QString &string)
{
    string.replace("<abr>", "<font class=\"abbreviature\">");
    string.replace("<tr>", "<font class=\"transcription\">[");
    string.replace("</tr>", "]</font>");
    string.replace("<ex>", "<font class=\"example\">");
    string.remove(QRegExp("<k>.*<\\/k>"));
    string.replace(QRegExp("(<\\/abr>)|(<\\ex>)"), "</font");
}

Q_EXPORT_PLUGIN2(stardict, StarDict)

#include "stardict.moc"
