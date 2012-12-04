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

#ifndef MULA_PLUGIN_STARDICT_STARDICT_H
#define MULA_PLUGIN_STARDICT_STARDICT_H

#include "stardictdictionarymanager.h"
#include "stardictdictionaryinfo.h"

#include <core/dictionaryplugin.h>

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QHash>

class QWidget;

namespace MulaPluginStarDict
{
    class StarDict : public QObject, public MulaCore::DictionaryPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.kde.DictionaryPluginInterface")
        Q_INTERFACES(MulaCore::DictionaryPlugin)

        public:

            /**
             * Constructor
             */

            StarDict(QObject *parent = 0);

            /**
             * Destructor
             */

            virtual ~StarDict();

            /** Reimplemented from DictionaryPlugin::name() */

            QString name() const;

            /** Reimplemented from DictionaryPlugin::version() */

            QString version() const;

            /** Reimplemented from DictionaryPlugin::description() */

            QString description() const;

            /** Reimplemented from DictionaryPlugin::authors() */

            QStringList authors() const;

            /** Reimplemented from DictionaryPlugin::features() */

            MulaCore::DictionaryPlugin::Features features() const;

            /** Reimplemented from DictionaryPlugin::availableDictionaryList() */

            QStringList availableDictionaryList();

            /** Reimplemented from DictionaryPlugin::loadedDictionaryList() */

            QStringList loadedDictionaryList() const;

            /** Reimplemented from DictionaryPlugin::setLoadedDictionaryList() */

            void setLoadedDictionaryList(const QStringList &loadedDictionaryList);

            /** Reimplemented from DictionaryPlugin::isTranslatable() */

            bool isTranslatable(const QString &dict, const QString &word);

            /** Reimplemented from DictionaryPlugin::translate() */

            MulaCore::Translation translate(const QString &dict, const QString &word);

            /** Reimplemented from DictionaryPlugin::findSimilarWords() */

            QStringList findSimilarWords(const QString &dict, const QString &word);

            /** Reimplemented from DictionaryPlugin::dictionaryInfo() */

            MulaCore::DictionaryInfo dictionaryInfo(const QString &dictionaryUrl);

            // int execSettingsDialog(QWidget *parent);

            friend class SettingsDialog;

        private:
            QString parseData(const QByteArray &data, int dictIndex = -1,
                    bool htmlSpaces = false, bool reformatLists = false, bool expandAbbreviations = false);

            QString findDictionary(const QString &name, const QStringList &dictDirs);
            static void xdxf2html(QString &str);

            /**
             * Finds the available dictionary from the desired ".ifo" file path
             * and then return the relevant book name
             *
             * @param absolutePath The complete path of the ".ifo" file
             *
             * @return The book name of the dictionary
             *
             * @see findIfoFile, availableDictionaryList, loadedDictionaryList,
             * setLoadedDictionaryList
             */

            QString findAvailableDictionary(const QString& absolutePath);

            QString findIfoFile(const QString& absolutePath);

            template <typename Method> QStringList recursiveTemplateFind(const QString& directoryPath, Method method);

            class Private;
            Private *const d;
    };
}

#endif // MULA_PLUGIN_STARDICT_STARDICT_H

