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

#ifndef MULA_PLUGIN_SWAC_SWAC_H
#define MULA_PLUGIN_SWAC_SWAC_H

#include <core/dictionaryplugin.h>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

namespace MulaPluginSwac {
    class Swac: public QObject, public QStarDict::DictPlugin
    {
        Q_OBJECT
            Q_INTERFACES(QStarDict::DictPlugin)

        public:
            Swac(QObject *parent = 0);
            virtual ~Swac();

            QString name() const;
            QString version() const;
            QString description() const;
            QStringList authors() const;
            Features features() const;

            QStringList availableDictionaries() const;
            QStringList loadedDictionaries() const;
            void setLoadedDictionaries(const QStringList &dictionaries);
            DictInfo dictionaryInfo(const QString &dictionary);

            bool isTranslatable(const QString &dictionary, const QString &word);
            Translation translate(const QString &dictionary, const QString &word);
            QStringList findSimilarWords(const QString &dictionary, const QString &word);

            int execSettingsDialog(QWidget *parent);

        private:
            QSqlQuery search(const QString &dictionary, const QString &word, const QString &fields, const int limit);
    };
}

#endif // MULA_PLUGIN_SWAC_SWAC_H
