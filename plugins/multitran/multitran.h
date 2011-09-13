/******************************************************************************
 * This file is part of the Mula project
 * Copyright (C) 2008 Nick Shaforostoff
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

#ifndef MULA_PLUGIN_MULTITRAN_MULTITRAN_H
#define MULA_PLUGIN_MULTITRAN_MULTITRAN_H

#include "dictionaryplugin.h"

#include <QtCore/QVector>
#include <QtCore/QHash>

class Multitran: public QObject, public Mula::DictionaryPlugin
{
    Q_OBJECT
    Q_INTERFACES(Mula::DictionaryPlugin)

    public:
        Multitran(QObject *parent = 0);
        virtual ~Multitran();

        QString name() const;
        QString version() const;
        QString description() const;
        QStringList authors() const;
        Features features() const;

        QStringList availableDicts() const;
        QStringList loadedDicts() const {return QStringList("Multitran");}//{ return m_loadedDicts.keys(); }
        void setLoadedDicts(const QStringList &loadedDicts);
        DictInfo dictInfo(const QString &dict);

        bool isTranslatable(const QString &dict, const QString &word);
        Translation translate(const QString &dict, const QString &word);
        virtual QStringList findSimilarWords(const QString &dict, const QString &word);

        int execSettingsDialog(QWidget *parent);

    private:
        class Private;
        Private *const d;
};

#endif // MULA_PLUGIN_MULTITRAN_MULTITRAN_H

