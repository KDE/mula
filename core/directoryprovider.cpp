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

#include "directoryprovider.h"

#include <QtGui/QDesktopServices>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QSettings>

using namespace MulaCore;

MULA_DEFINE_SINGLETON( DirectoryProvider )

DirectoryProvider::DirectoryProvider( QObject* parent )
    : MulaCore::Singleton< MulaCore::DirectoryProvider >( parent )
{
    m_userDataPath = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
    m_userDataPath.chop( QString(QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName()).size() );
    m_userDataPath.append( "mula" );

    //Define standard dirs Mula recommends
    m_userDirs["data"] =  QDir::fromNativeSeparators( m_userDataPath + "/data" );

    //Create standard dirs Mula recommends
    QDir dir;
    foreach( const QString& dirPath, m_userDirs )
    {
        dir.mkpath( dirPath );
    }
}

QString DirectoryProvider::installPrefix() const
{
#ifdef Q_OS_WIN
    QSettings *settings;
    if (MULA_ARCHITECTURE == "32")
        settings = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Mula\\Mula-" + MULA_VERSION_STRING + "\\", QSettings::NativeFormat);
    else if (MULA_ARCHITECTURE == "64")
        settings = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Mula\\Mula-" + MULA_VERSION_STRING + "\\", QSettings::NativeFormat);

    QString installPath = settings->value("Default").toString();
    delete settings;
    return installPath.isEmpty() ? MULA_INSTALL_PREFIX : installPath;
#else
    return MULA_INSTALL_PREFIX;
#endif
}

QString DirectoryProvider::dataDirectory() const
{
    return installPrefix() + "/" + MULA_SHARE_INSTALL_DIR;
}

QString DirectoryProvider::libDirectory() const
{
    return installPrefix() + "/" + MULA_LIB_INSTALL_DIR;
}

QString DirectoryProvider::userDirectory( const QString& name )
{
    if( !m_userDirs.contains( name ) )
    {
        QString path = QDir::fromNativeSeparators( m_userDataPath + name );
        m_userDirs[name] = path;
        QDir dir;
        dir.mkpath( path );
    }

    return m_userDirs[name];
}

QStringList DirectoryProvider::pluginDirectoryPaths() const
{
    QStringList pluginDirectoryPaths;

    QString pluginDirectoryPath = QCoreApplication::applicationDirPath();

#if defined(Q_OS_WIN)
    if( pluginDirectoryPath.endsWith("/debug", Qt::CaseInsensitive) )
        pluginDirectoryPath.chop(QByteArray("/debug").size());

    else if( pluginDirectoryPath.endsWith("release", Qt::CaseInsensitive) )
        pluginDirectoryPath.chop(QByteArray("/release").size());

#elif defined(Q_OS_MAC)
    if( pluginDirectoryPath.endsWith("/MacOS") )
        pluginDirectoryPath.chop(QByteArray("/MacOS").size());

#endif

    if( QFile( pluginDirectoryPath + "/PlugIns" ).exists() )
        pluginDirectoryPaths.append( pluginDirectoryPath );

    QString libraryDirectoryPath = libDirectory();

    if( QFile( libraryDirectoryPath ).exists() )
        pluginDirectoryPaths.append( libraryDirectoryPath );

    // This is the plugin dir on windows
    if( QFile( libraryDirectoryPath + "/kde4" ).exists() )
        pluginDirectoryPaths.append( libraryDirectoryPath + "/kde4" );

    if( QFile( libraryDirectoryPath + "/mula" ).exists() )
        pluginDirectoryPaths.append( libraryDirectoryPath + "/mula" );

    if( QFile( QDir::homePath() + "/mulaplugins" ).exists() )
        pluginDirectoryPaths.append( QDir::homePath() + "/mulaplugins" );

    return pluginDirectoryPaths;
}

#include "directoryprovider.moc"
