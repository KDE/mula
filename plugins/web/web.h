/******************************************************************************
 * This file is part of the Mula project
 * Copyright (c) 2008 Alexander Rodin
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

#ifndef MULA_PLUGIN_WEB_WEB_H
#define MULA_PLUGIN_WEB_WEB_H

#include <core/dictionaryplugin.h>

#include <QtCore/QHash>

class Web: public QObject, public Mula::DictoinaryPlugin
{
    Q_OBJECT
    Q_INTERFACES(Mula::DictionaryPlugin)

    public:
        Web(QObject *parent = 0);
        virtual ~Web();

        QString name() const;
        QString version() const;
        QString description() const;
        QStringList authors() const;
        Features features() const;

        QStringList availableDicts() const;
        QStringList loadedDicts() const;
        void setLoadedDicts(const QStringList &dicts);
        DictInfo dictInfo(const QString &dict);

        bool isTranslatable(const QString &dict, const QString &word);
        Translation translate(const QString &dict, const QString &word);

        int execSettingsDialog(QWidget *parent);

        friend class SettingsDialog;

    private:
        struct QueryStruct
        {
            QString query;
            QByteArray codec;
        };
        QHash<QString, QueryStruct> m_loadedDicts;
};

#endif // MULA_PLUGIN_WEB_WEB_H

