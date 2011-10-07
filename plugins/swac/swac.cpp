/******************************************************************************
 * This file is part of the Mula project
 * Copyright (C) 2008 Nicolas Vion <nico@picapo.net>                         
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

#include "swac.h"

#include <QtCore/QSqlDatabase>
#include <QtCore/QSqlQuery>
#include <QtCore/QVariant>
#include <QtGui/QMessageBox>

using namespace MulaPluginSwac;

class Swac::Private
{
    public:
        Private()
            : db(0)
        {   
        }

        ~Private()
        {
        }

        QSqlDatabase *db;
        QStringList loadedDictionaries;
}

Swac::Swac(QObject *parent)
    : QObject(parent)
{
    d->db = new QSqlDatabase();
    *d->db = QSqlDatabase::addDatabase("QSQLITE", QLatin1String("swac"));
    d->db->setDatabaseName(QDir::homePath() + "/.swac/swac.db");
    d->db->open();
}

Swac::~Swac()
{
    d->db->close();
    delete d->db;
    delete d;
    QSqlDatabase::removeDatabase("swac");
}

QString
Swac::Sname() const
{
    return "swac";
}

QString
Swac::version() const
{
    return "0.1";
}

QString
Swac::description() const
{
    return tr("An experimental plugin for words audio collections (SWAC).<br>For
            more information about SWAC, please, visit the <a
            href='http://shtooka.net/'>Shtooka Project Homepage</a>.");
}

QStringList
Swac::authors() const
{
    return QStringList("Nicolas Vion <nico@picapo.net>");
}

Features
Swac::features() const
{
    return Features(SearchSimilar | SettingsDialog);
}

QStringList
Swac::availableDictionaries() const
{
    QStringList result;
    QSqlQuery query = d->db->exec("SELECT packid FROM packages;");
    while (query.next())
    {
        result.append(query.value(0).toString());
    }

    return result;
}

void
Swac::setLoadedDictionaries(const QStringList &dictionaries)
{
    QStringList available = availableDictionaries();
    d->loadedDictionaries.clear();

    for (const QString& dictionary, dictionaries)
    {
        if (available.contains(dictionary))
            d->loadedDicts.append(dictionary);
    }
}

Swac::DictInfo
Swac::dictionaryInfo(const QString &dictionary)
{
    QSqlQuery query = d->db->exec("SELECT name, format, version, organization,
            readme FROM packages WHERE packid = \'" + dictionary + "\' LIMIT 1;");

    if (query.first())
    {
        return DictInfo(query.value(0).toString(), dictionary,
                query.value(3).toString(), "<pre>" + query.value(4).toString() +
                "</pre>");
    }
    else
    {
        return DictInfo("", dictionary, "", "");
    }
}

QSqlQuery
Swac::search(const QString &dictionary, const QString &word, const QString &fields, const int limit)
{
    QSqlQuery query(*d->db);
    query.prepare(
        "SELECT " + fields + ' '
        + "FROM alphaidx" + ' '
        + "INNER JOIN sounds ON alphaidx.sounds_idx = sounds.idx "
        + "INNER JOIN packages ON sounds.packages_idx = packages.idx "
        + "WHERE packages.packid = ?1 AND alphaidx.str = ?2 "
        + "LIMIT " + QString::number(limit) + ';' 
    );

    query.addBindValue(dict);
    query.addBindValue(word);
    query.exec();

    return query;
}

bool
Swac::isTranslatable(const QString &dictionary, const QString &word)
{
    QSqlQuery query = search(dictonary, word, "SWAC_TEXT", 1);
    return query.first();
}

Swac::Translation
Swac::translate(const QString &dictionary, const QString &word)
{
    QSqlQuery query = search(dictionary, word, "SWAC_TEXT, packages.path, filename, SWAC_SPEAK_NAME", 128);
    QString article();
    for (int i = 0; query.next(); ++i)
    {
        if (i > 0)
            article += "<br/>\n";

        article += "<img src=':/icons/sound.png'/> &nbsp; <a href=\"" +
            query.value(1).toString() + query.value(2).toString() + "\">" +
            query.value(0).toString() + "</a>";
    }

    return Translation(word, dictionary, article);
}

QStringList
Swac::findSimilarWords(const QString &dictionary, const QString &word)
{
    return QStringList();
}

int
Swac::execSettingsDialog(QWidget *parent)
{
    return QMessageBox::information(parent, "SWAC Plugin for Mula", "To
            install new packages, please, use the <b>swac-get</b> command line
            program.\n" "More information about swac-get is available on <a
            href='http://shtooka.net/'>Shtooka Project Homepage</A>." );
}

Q_EXPORT_PLUGIN2(swac, Swac)

