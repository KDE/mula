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

#ifndef MULA_CORE_DICTIONARYPLUGIN_H
#define MULA_CORE_DICTIONARYPLUGIN_H

#include "dictionaryinfo.h"
#include "translation.h"

#include <QtCore/QtPlugin>
#include <QtCore/QUrl>

namespace MulaCore
{
    /**
     * This is a base, interface class for all the dictionary plugins classes.
     */
    class MULA_CORE_EXPORT DictionaryPlugin
    {
        public:
            /**
             * This enum describes the features of a dictionary plugin.
             */
            enum Feature
            {
                /**
                 * No features.
                 */
                None,

                /**
                 * Dictionary plugin can search for similar words using
                 * fuzzy algorithms.
                 */
                SearchSimilar,

                /**
                 * Dictionary plugin has a settings dialog.
                 */
                SettingsDialog,
            };

            Q_DECLARE_FLAGS(Features, Feature)

            /**
             * Constructor
             */
            DictionaryPlugin();

            /**
             * Destructor
             */
            virtual ~DictionaryPlugin();

            /**
             * Returns the name of the dictionary plugin
             *
             * @return Name of the dictionary plugin
             */
            virtual QString name() const = 0;

            /**
             * Returns the version of the dictionary plugin
             *
             * @return Version of the dictionary plugin
             */
            virtual QString version() const = 0;

            /**
             * Returns the description of the dictionary plugin
             *
             * @return Description of the dictionary plugin
             */
            virtual QString description() const = 0;

            /**
             * Returns all the authors of the dictionary plugin
             *
             * @return List of the dictionary plugin authors
             */
            virtual QStringList authors() const = 0;

            /**
             * Returns the features supported by dictionary plugin.
             *
             * @return Features that are supported
             */
            virtual Features features() const;

            /**
             * Returns a list of the available dictionaries
             *
             * @return List of the available dictionaries
             *
             * @see loadedDictionaryList, setLoadedDictionaryList
             */
            virtual QStringList availableDictionaryList() = 0;

            /**
             * Returns a list of the loaded dictionaries
             *
             * @return List of the loaded dictionaries
             *
             * @see loadedDictionaryList, availableDictionaryList
             */
            virtual QStringList loadedDictionaryList() const = 0;

            /**
             * Sets a list of the loaded dictionaries
             *
             * @param loadedDictionaryList List of the loaded dictionaries
             *
             * @see loadedDictionaryList, availableDictionaryList
             */
            virtual void setLoadedDictionaryList(const QStringList &loadedDictionaryList) = 0;

            /**
             * Returns true if translation exists in dictionary, otherwise
             * returns false
             *
             * @param dictionary Name of the dictionary
             * @param word The word that is being looked up in the desired
             * dictionary
             *
             * @return Whether the translation exists in the desired dictionary
             *
             * @see translate
             */
            virtual bool isTranslatable(const QString &dictionary, const QString &word) = 0;

            /**
             * Returns translation for word from dictionary. If word not found
             * returns empty string.
             *
             * @param dictionary The name of the dictionary
             * @param word The word that is looked up in the desired dictionary
             *
             * @return A translation class object that represents the
             * translation result
             *
             * @see isTranslatable
             */
            virtual Translation translate(const QString &dictionary, const QString &word) = 0;

            /**
             * Returns a list of similar words from all the loaded dictionaries.
             * It works only if SearchSimilar feature is enabled.
             *
             * @param dictionary The name of the desired dictionary
             * @param word The word that is being looked up in the desired
             * dictionary
             *
             * @return The similar words in a list
             */
            virtual QStringList findSimilarWords(const QString &dictionary, const QString &word);

            /**
             * Returns information about the dictionary. The dictionary may be
             * not loaded but can be available.
             *
             * @param dictionary The name of the desired dictionary
             *
             * @return The DictionaryInfo class representing information
             * about the given dictionary
             */
            virtual DictionaryInfo dictionaryInfo(const QString &dictionary) = 0;

            /**
             * Runs a settings dialog and return QDialog::DialogCode.
             *
             * @param parent The parent object of the settings dialog
             *
             * @return The relevant QDialog::DialogCode
             */
            virtual int execSettingsDialog(QWidget *parent = 0);

        protected:
            /**
             * Returns a directory that contains the data of the plugin.
             *
             * @return The string value containing the dictionary path of the
             * plugin data
             */
            QString pluginDataPath() const;
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS(DictionaryPlugin::Features)
}

Q_DECLARE_INTERFACE(MulaCore::DictionaryPlugin, "org.mula.DictionaryPlugin/1.0")

#endif // MULA_CORE_DICTIONARYPLUGIN_H

