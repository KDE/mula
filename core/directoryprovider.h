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

#ifndef MULA_CORE_DIRECTORYPROVIDER_H
#define MULA_CORE_DIRECTORYPROVIDER_H

#include "mula_core_export.h"
#include "core/mula_global.h"

#include "singleton.h"

#include <QtCore/QHash>

namespace MulaCore
{

    class MULA_CORE_EXPORT DirectoryProvider : public MulaCore::Singleton<DirectoryProvider>
    {
            Q_OBJECT
            MULA_SINGLETON( DirectoryProvider )

        public:
            QString installPrefix() const;

            QString dataDirectory() const;

            QString libDirectory() const;

            QString userDirectory( const QString& name );

            QStringList pluginDirectoryPaths() const;

        private:
            QString m_userDataPath;
            QHash<QString, QString> m_userDirs;
    };

}

#endif // MULA_CORE_DIRECTORYPROVIDER_H
