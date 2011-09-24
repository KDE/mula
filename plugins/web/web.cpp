/******************************************************************************
 * This file is part of the Mula project
 * Copyright (C) 2008 Alexander Rodin
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

#include "web.h"

#include "settingsdialog.h"

#include <QBuffer>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QHttp>
#include <QSettings>
#include <QUrl>
#include <QTextCodec>

class Web::Private
{
    public:
        Private()
        {   
        }

        ~Private()
        {
        }

        struct QueryStruct
        {   
            QString query;
            QByteArray codec;
        };
        QHash<QString, QueryStruct> loadedDicts;
}

Web::Web(QObject *parent)
    : QObject(parent)
{
}

QStringList Web::availableDictionaries() const
{
    QStringList result = QDir(pluginDataPath()).entryList(QStringList("*.webdict"), QDir::Files, QDir::Name);
    result.replaceInStrings(".webdict", "");
    return result;
}

void Web::setLoadedDictionaries(const QStringList &dictionaries)
{
    for (QStringList::const_iterator i = dictionaries.begin(); i != dictionaries.end(); ++i)
    {
        QString filename = pluginDataPath() + "/" + *i + ".webdict";
        if (!QFile::exists(filename))
            continue;

        QSettings dictionary(filename, QSettings::IniFormat);
        QString query = dict.value("query").toString();
        if (!query.isEmpty())
        {
            d->loadedDictionaries[*i].query = query;
            d->loadedDictionaries[*i].codec = dictionary.value("charset").toByteArray();
        }
    }
}

Web::DictionaryInfo Web::dictionaryInfo(const QString &dictionary)
{
    QString filename = pluginDataPath() + "/" + dict + ".webdict";
    if (!QFile::exists(filename))
        return DictionaryInfo();

    QSettings dictFile(filename, QSettings::IniFormat);
    DictionaryInfo info(name(), dictionary, dictionaryFile.value("author").toString(),
            dictionaryFile.value("description").toString());

    return info;
}

bool Web::isTranslatable(const QString &dictionary, const QString &word)
{
    if (!d->loadedDictionaries.contains(dictionary))
        return false;

    // TODO: Implement the boolean query
    Q_UNUSED(word);
    return true;
}

Web::Translation Web::translate(const QString &dictionary, const QString &word)
{
    if (!d->loadedDictionaries.contains(dictionary))
        return Translation();

    QUrl url(d->loadedDictionaries[dictionary].query.replace("%s", word));
    QEventLoop loop;
    QHttp http(url.host(), url.port(80), &loop);
    connect(&http, SIGNAL(done(bool)), &loop, SLOT(quit()));
    http.get(url.path() + "?" + url.encodedQuery());
    loop.exec();

    QTextCodec *codec = QTextCodec::codecForName(m_loadedDicts[dict].codec);
    QString translation;
    if (codec)
        translation = codec->toUnicode(http.readAll());
    else
        translation = QString::fromUtf8(http.readAll());

    return Translation(dictionary, word, translation);
}

int Web::execSettingsDialog(QWidget *parent)
{
    ::SettingsDialog dialog(this, parent);
    return dialog.exec();
}

Q_EXPORT_PLUGIN2(web, Web)

