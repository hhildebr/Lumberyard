/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "stdafx.h"
#include "AssetImporterDragAndDropHandler.h"
#include <AssetImporter/AssetImporterManager/AssetImporterManager.h>
#include <AzToolsFramework/AssetDatabase/AssetDatabaseConnection.h>
#include <AzToolsFramework/AssetBrowser/AssetBrowserEntry.h>
#include <QMimeData>
#include "MainWindow.h"

bool AssetImporterDragAndDropHandler::m_dragAccepted = false;

AssetImporterDragAndDropHandler::AssetImporterDragAndDropHandler(QObject* parent, AssetImporterManager* const assetImporterManager)
    : QObject(parent)
    , m_assetImporterManager(assetImporterManager)
{
    AzQtComponents::DragAndDropEventsBus::Handler::BusConnect(DragAndDropContexts::MainWindow);

    // They are used to prevent opening the Asset Importer by dragging and dropping files and folders to the Main Window when it is already running
    connect(m_assetImporterManager, &AssetImporterManager::StartAssetImporter, this, &AssetImporterDragAndDropHandler::OnStartAssetImporter);
    connect(m_assetImporterManager, &AssetImporterManager::StopAssetImporter, this, &AssetImporterDragAndDropHandler::OnStopAssetImporter);
}

AssetImporterDragAndDropHandler::~AssetImporterDragAndDropHandler()
{
    AzQtComponents::DragAndDropEventsBus::Handler::BusDisconnect(DragAndDropContexts::MainWindow);
}

void AssetImporterDragAndDropHandler::DragEnter(QDragEnterEvent* event)
{
    if (!m_isAssetImporterRunning)
    {
        ProcessDragEnter(event);
    }
}

void AssetImporterDragAndDropHandler::Drop(QDropEvent* event)
{
    if (!m_dragAccepted)
    {
        return;
    }

    QStringList fileList = GetFileList(event);

    if (!fileList.isEmpty())
    {
        Q_EMIT OpenAssetImporterManager(fileList);
    }

    // reset
    m_dragAccepted = false;
}

void AssetImporterDragAndDropHandler::ProcessDragEnter(QDragEnterEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    // if the event hasn't been accepted already and the mimeData hasUrls()
    if (!event->isAccepted() && mimeData->hasUrls())
    {
        // prevent users from dragging and dropping files from the Asset Browser
        if (mimeData->hasFormat(AzToolsFramework::AssetBrowser::AssetBrowserEntry::GetMimeType()))
        {
            m_dragAccepted = false;
            return;
        }
       
        QList<QUrl> urlList = mimeData->urls();
        int urlListSize = urlList.size();

        // runs through the file list first and checks for any "crate" files - if it finds ANY, return (and don't accept the event)
        for (int i = 0; i < urlListSize; ++i)
        {
            QUrl currentUrl = urlList.at(i);

            if (currentUrl.isLocalFile())
            {
                QString path = urlList.at(i).toLocalFile();

                if (ContainCrateFiles(path))
                {
                    return;
                }
            }
        }

        for (int i = 0; i < urlListSize; ++i)
        {
            // Get the local file path
            QString path = urlList.at(i).toLocalFile();

            QDir dir(path);
            QString relativePath = dir.relativeFilePath(path);
            QString absPath = dir.absolutePath();

            // check if the files/folders are under the game root directory
            QDir gameRoot = Path::GetEditingGameDataFolder().c_str();
            QString gameRootAbsPath = gameRoot.absolutePath();

            if (absPath.startsWith(gameRootAbsPath, Qt::CaseInsensitive))
            {
                m_dragAccepted = false;
                return;
            }

            QDirIterator it(absPath, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);

            QFileInfo info(absPath);
            QString extension = info.completeSuffix();

            // if it's not an empty folder directory or if it's a file,
            // then allow the drag and drop process.
            // Otherwise, prevent users from dragging and dropping empty folders
            if (it.hasNext() || !extension.isEmpty())
            {
                // this is used in Drop()
                m_dragAccepted = true;
            }
        }

        // at this point, all files should be legal to be imported
        // since they are not in the database
        if (m_dragAccepted)
        {
            event->acceptProposedAction();
        }
    }
}

QStringList AssetImporterDragAndDropHandler::GetFileList(QDropEvent* event)
{
    QStringList fileList;
    const QMimeData* mimeData = event->mimeData();

    QList<QUrl> urlList = mimeData->urls();

    for (int i = 0; i < urlList.size(); ++i)
    {
        QUrl currentUrl = urlList.at(i);

        if (currentUrl.isLocalFile())
        {
            QString path = urlList.at(i).toLocalFile();

            if (!ContainCrateFiles(path))
            {
                fileList.append(path);
            }
        }
    }

    return fileList;
}

void AssetImporterDragAndDropHandler::OnStartAssetImporter()
{
    m_isAssetImporterRunning = true;
}

void AssetImporterDragAndDropHandler::OnStopAssetImporter()
{
    m_isAssetImporterRunning = false;
}


bool AssetImporterDragAndDropHandler::ContainCrateFiles(QString path)
{
    QFileInfo fileInfo(path);

    if (fileInfo.isFile())
    {
        return isCrateFile(fileInfo);
    }
    else
    {
        QDirIterator it(path, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            QString str = it.next();
            QFileInfo info(str);

            if (isCrateFile(info))
            {
                return true;
            }
        }
    }

    return false;
}

bool AssetImporterDragAndDropHandler::isCrateFile(QFileInfo fileInfo)
{
    return QStringLiteral("crate").compare(fileInfo.suffix(), Qt::CaseInsensitive) == 0;
}

#include <AssetImporter/AssetImporterManager/AssetImporterDragAndDropHandler.moc>