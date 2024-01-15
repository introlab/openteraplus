/* Webcamoid, webcam capture application.
 * Copyright (C) 2011-2017  Gonzalo Exequiel Pedone
 *
 * Webcamoid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Webcamoid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Webcamoid. If not, see <http://www.gnu.org/licenses/>.
 *
 * Web-Site: http://webcamoid.github.io/
 */

#include <QRegularExpression>
#include <QMetaMethod>
#include <QPluginLoader>
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDataStream>
#include <QCryptographicHash>

#include "ak.h"

#define SUBMODULES_PATH "submodules"

class AkPluginInfoPrivate
{
    public:
        QString m_id;
        QString m_path;
        QVariantMap m_metaData;
        bool m_used;
};

class AkElementPrivate
{
    public:
        QString m_pluginId;
        QString m_pluginPath;
        QString m_pluginFilePattern;
        QStringList m_pluginsSearchPaths;
        QStringList m_defaultPluginsSearchPaths;
        QStringList m_pluginsBlackList;
        QList<AkPluginInfoPrivate> m_pluginsList;
        QString m_subModulesPath;
        QDir m_applicationDir;
        AkElement::ElementState m_state;
        bool m_recursiveSearchPaths;
        bool m_pluginsScanned;

        AkElementPrivate()
        {
            this->m_recursiveSearchPaths = false;
            this->m_pluginsScanned = false;

            this->m_defaultPluginsSearchPaths << QString("%1/%2")
                                                 .arg(LIBDIR)
                                                 .arg(COMMONS_TARGET);

#ifdef Q_OS_OSX
            QString defaultPath;

            if (QCoreApplication::applicationDirPath()
                    .endsWith(".app/Contents/MacOS")) {
                QDir appDir(QCoreApplication::applicationDirPath());
                appDir.cd(QString("../Plugins/%1").arg(COMMONS_TARGET));
                defaultPath = appDir.absolutePath();
            }
#else
            QString defaultPath = QString("%1/../lib/%2")
                                  .arg(QCoreApplication::applicationDirPath())
                                  .arg(COMMONS_TARGET);
#endif

            if (!defaultPath.isEmpty())
                this->m_defaultPluginsSearchPaths << this->convertToAbsolute(defaultPath);

            this->m_applicationDir.setPath(QCoreApplication::applicationDirPath());
            this->m_subModulesPath = SUBMODULES_PATH;

#ifdef Q_OS_OSX
            this->m_pluginFilePattern = "lib*.dylib";
#elif defined(Q_OS_WIN32)
            this->m_pluginFilePattern = "*.dll";
#else
            this->m_pluginFilePattern = "lib*.so";
#endif
        }

        static inline QList<QMetaMethod> methodsByName(const QObject *object,
                                                       const QString &methodName)
        {
            QList<QMetaMethod> methods;
            QStringList methodSignatures;

            for (int i = 0; i < object->metaObject()->methodCount(); i++) {
                QMetaMethod method = object->metaObject()->method(i);
                QString signature(method.methodSignature());

                if (QRegularExpression(QString("\\s*%1\\s*\\(.*").arg(methodName))
                    .match(signature).hasMatch())
                    if (!methodSignatures.contains(signature)) {
                        methods << method;
                        methodSignatures << signature;
                    }
            }

            return methods;
        }

        static inline bool methodCompat(const QMetaMethod &method1,
                                        const QMetaMethod &method2)
        {
            if (method1.parameterTypes() == method2.parameterTypes())
                return true;

            return false;
        }

        static inline QString pluginId(const QString &fileName)
        {
            auto pluginId = QFileInfo(fileName).baseName();

#ifdef Q_OS_WIN32
            return pluginId;
                                              ;
#else
            return pluginId.remove(QRegularExpression("^lib"));
#endif
        }

        inline QString convertToAbsolute(const QString &path) const
        {
            if (!QDir::isRelativePath(path))
                return QDir::cleanPath(path);

            QString absPath = this->m_applicationDir.absoluteFilePath(path);

            return QDir::cleanPath(absPath);
        }

        inline void listPlugins()
        {
            QVector<QStringList *> sPaths {
                &this->m_pluginsSearchPaths,
                &this->m_defaultPluginsSearchPaths
            };

            for (auto sPath: sPaths)
                for (int i = sPath->length() - 1; i >= 0; i--) {
                    QString searchDir(sPath->at(i));

                    searchDir.replace(QRegularExpression("((\\\\/?)|(/\\\\?))+"),
                                      QDir::separator());

                    while (searchDir.endsWith(QDir::separator()))
                        searchDir.resize(searchDir.size() - 1);

                    QStringList searchPaths(searchDir);

                    while (!searchPaths.isEmpty()) {
                        QString path = searchPaths.takeFirst();

                        if (this->m_pluginsBlackList.contains(path))
                            continue;

                        auto pluginId = this->pluginId(path);
                        bool found = false;

                        for (auto &pluginInfo: this->m_pluginsList)
                            if (pluginInfo.m_path == path) {
                                if (pluginInfo.m_metaData.isEmpty()) {
                                    QPluginLoader pluginLoader(path);

                                    if (pluginLoader.load()) {
                                        if (pluginInfo.m_id.isEmpty())
                                            pluginInfo.m_id = pluginId;

                                        pluginInfo.m_metaData =
                                                pluginLoader.metaData().toVariantMap();
                                        pluginInfo.m_used = true;

                                        pluginLoader.unload();
                                    }
                                } else {
                                    if (pluginInfo.m_id.isEmpty())
                                        pluginInfo.m_id = pluginId;

                                    pluginInfo.m_used = true;
                                }

                                found = true;

                                break;
                            }

                        if (found)
                            continue;

                        if (QFileInfo(path).isFile()) {
                            QString fileName = QFileInfo(path).fileName();

                            // if (QRegExp(this->m_pluginFilePattern,
                            //             Qt::CaseSensitive,
                            //             QRegExp::Wildcard).exactMatch(fileName)) {
                            if (QRegularExpression(
                                    QRegularExpression::fromWildcard(this->m_pluginFilePattern, Qt::CaseSensitive)
                                        ).match(fileName).hasMatch()) {
                                QPluginLoader pluginLoader(path);

                                if (pluginLoader.load()) {
                                    auto metaData = pluginLoader.metaData();

                                    if (metaData["MetaData"].toObject().contains("pluginType")
                                        && metaData["MetaData"].toObject()["pluginType"] == AK_PLUGIN_TYPE_ELEMENT) {
                                        this->m_pluginsList <<
                                            AkPluginInfoPrivate {
                                                pluginId,
                                                path,
                                                metaData.toVariantMap(),
                                                true
                                            };
                                    }

                                    pluginLoader.unload();
                                }
                            }
                        } else {
                            QDir dir(path);
                            auto fileList = dir.entryList({this->m_pluginFilePattern},
                                                          QDir::Files
                                                          | QDir::CaseSensitive,
                                                          QDir::Name);

                            for (const QString &file: fileList)
                                searchPaths << dir.absoluteFilePath(file);

                            if (this->m_recursiveSearchPaths) {
                                auto dirList = dir.entryList(QDir::Dirs
                                                             | QDir::NoDotAndDotDot,
                                                             QDir::Name);

                                for (const QString &path: dirList)
                                    searchPaths << dir.absoluteFilePath(path);
                            }
                        }
                    }
                }

            this->m_pluginsScanned = true;
        }
};

Q_GLOBAL_STATIC(AkElementPrivate, akElementGlobalStuff)

AkElement::AkElement(QObject *parent):
    QObject(parent)
{
    this->d = new AkElementPrivate();
    this->d->m_state = ElementStateNull;
}

AkElement::~AkElement()
{
    this->setState(AkElement::ElementStateNull);
    delete this->d;
}

QString AkElement::pluginId() const
{
    return this->d->m_pluginId;
}

QString AkElement::pluginId(const QString &path)
{
    return akElementGlobalStuff->pluginId(path);
}

QString AkElement::pluginPath() const
{
    return this->d->m_pluginPath;
}

AkElement::ElementState AkElement::state() const
{
    return this->d->m_state;
}

QObject *AkElement::controlInterface(QQmlEngine *engine,
                                     const QString &controlId) const
{
    Q_UNUSED(controlId)

    if (!engine)
        return nullptr;

    auto qmlFile = this->controlInterfaceProvide(controlId);

    if (qmlFile.isEmpty())
        return nullptr;

    // Load the UI from the plugin.
    QQmlComponent component(engine, qmlFile);

    if (component.isError()) {
        qDebug() << "Error in plugin "
                 << this->metaObject()->className()
                 << ":"
                 << component.errorString();

        return nullptr;
    }

    // Create a context for the plugin.
    QQmlContext *context = new QQmlContext(engine->rootContext());
    this->controlInterfaceConfigure(context, controlId);

    // Create an item with the plugin context.
    QObject *item = component.create(context);

    if (!item) {
        delete context;

        return nullptr;
    }

    QQmlEngine::setObjectOwnership(item, QQmlEngine::JavaScriptOwnership);
    context->setParent(item);

    return item;
}

bool AkElement::link(const QObject *dstElement,
                     Qt::ConnectionType connectionType) const
{
    return this->link(this, dstElement, connectionType);
}

bool AkElement::link(const AkElementPtr &dstElement, Qt::ConnectionType connectionType) const
{
    return this->link(static_cast<QObject *>(dstElement.data()), connectionType);
}

bool AkElement::unlink(const QObject *dstElement) const
{
    return this->unlink(this, dstElement);
}

bool AkElement::unlink(const AkElementPtr &dstElement) const
{
    return this->unlink(static_cast<QObject *>(dstElement.data()));
}

bool AkElement::link(const AkElementPtr &srcElement,
                     const QObject *dstElement,
                     Qt::ConnectionType connectionType)
{
    return srcElement->link(dstElement, connectionType);
}

bool AkElement::link(const AkElementPtr &srcElement,
                     const AkElementPtr &dstElement,
                     Qt::ConnectionType connectionType)
{
    return srcElement->link(dstElement, connectionType);
}

bool AkElement::link(const QObject *srcElement,
                     const QObject *dstElement,
                     Qt::ConnectionType connectionType)
{
    if (!srcElement || !dstElement)
        return false;

    QList<QMetaMethod> signalList = AkElementPrivate::methodsByName(srcElement, "oStream");
    QList<QMetaMethod> slotList = AkElementPrivate::methodsByName(dstElement, "iStream");

    for (const QMetaMethod &signal: signalList)
        for (const QMetaMethod &slot: slotList)
            if (AkElementPrivate::methodCompat(signal, slot) &&
                signal.methodType() == QMetaMethod::Signal &&
                slot.methodType() == QMetaMethod::Slot)
            {
                qDebug() << "Connecting " << srcElement << dstElement ;
                QObject::connect(srcElement, signal, dstElement, slot, connectionType);
            }

    return true;
}

bool AkElement::unlink(const AkElementPtr &srcElement,
                       const QObject *dstElement)
{
    return srcElement->unlink(dstElement);
}

bool AkElement::unlink(const AkElementPtr &srcElement,
                       const AkElementPtr &dstElement)
{
    return srcElement->unlink(dstElement);
}

bool AkElement::unlink(const QObject *srcElement, const QObject *dstElement)
{
    if (!srcElement || !dstElement)
        return false;

    for (const QMetaMethod &signal: AkElementPrivate::methodsByName(srcElement, "oStream"))
        for (const QMetaMethod &slot: AkElementPrivate::methodsByName(dstElement, "iStream"))
            if (AkElementPrivate::methodCompat(signal, slot) &&
                signal.methodType() == QMetaMethod::Signal &&
                slot.methodType() == QMetaMethod::Slot)
                QObject::disconnect(srcElement, signal, dstElement, slot);

    return true;
}

AkElementPtr AkElement::create(const QString &pluginId,
                               const QString &elementName)
{
    //qDebug() << "****************** Trying to create element : " << pluginId<<","<<elementName;

    auto element = AkElement::createPtr(pluginId, elementName);

    if (!element)
        return AkElementPtr();

    return AkElementPtr(element);
}

AkElement *AkElement::createPtr(const QString &pluginId, const QString &elementName)
{

    //qDebug() << "****************** Trying to create element Ptr : " << pluginId<<","<<elementName;

    QString filePath = AkElement::pluginPath(pluginId);

    if (filePath.isEmpty())
        return nullptr;

    QPluginLoader pluginLoader(filePath);

    if (!pluginLoader.load()) {
        qDebug() << "Error loading plugin " << pluginId << ":" << pluginLoader.errorString();

        return nullptr;
    }

    qDebug() << "Plugin Loaded : " << pluginId;

    auto plugin = qobject_cast<AkPlugin *>(pluginLoader.instance());

    if (!plugin)
        return nullptr;

    auto element = qobject_cast<AkElement *>(plugin->create(AK_PLUGIN_TYPE_ELEMENT, ""));
    delete plugin;

    if (!element)
        return nullptr;

    if (!elementName.isEmpty())
        element->setObjectName(elementName);

    element->d->m_pluginId = pluginId;
    element->d->m_pluginPath = filePath;

    return element;
}

QStringList AkElement::listSubModules(const QString &pluginId,
                                      const QString &type)
{
    QStringList subModules;
    auto subModulesPaths = AkElement::listSubModulesPaths(pluginId);

    qDebug() << "subModulesPaths " << subModulesPaths;

    for (const QString &path: subModulesPaths) {
        QPluginLoader pluginLoader(path);
        QJsonObject metaData = pluginLoader.metaData();
        QString pluginId = AkElementPrivate::pluginId(path);

        if (!type.isEmpty()
            && metaData["MetaData"].toObject().contains("type")
            && metaData["MetaData"].toObject()["type"] == type
            && !subModules.contains(pluginId))
            subModules << pluginId;
        else if (type.isEmpty()
                 && !subModules.contains(pluginId))
            subModules << pluginId;
    }

    subModules.sort();

    qDebug() << "subModules: " << subModules;
    return subModules;
}

QStringList AkElement::listSubModules(const QStringList &types)
{
    QString pluginId;

    if (this->d->m_pluginId.isEmpty()) {
        pluginId = this->metaObject()->className();
        pluginId.replace(QRegularExpression("Element$"), "");
    } else {
        pluginId = this->d->m_pluginId;
    }

    if (types.isEmpty())
        return AkElement::listSubModules(pluginId);

    QStringList subModules;

    for (const QString &type: types)
        subModules << AkElement::listSubModules(pluginId, type);

    return subModules;
}

QStringList AkElement::listSubModulesPaths(const QString &pluginId)
{
    auto filePath = AkElement::pluginPath(pluginId);

    qDebug() << "filePath " << filePath;

    if (filePath.isEmpty())
        return QStringList();

    auto pluginDir = QFileInfo(filePath).absoluteDir();

    if (!pluginDir.cd(akElementGlobalStuff->m_subModulesPath
                      + QDir::separator()
                      + pluginId))
        return QStringList();

    QStringList subModulesPaths;
    auto plugins = pluginDir.entryList({akElementGlobalStuff->m_pluginFilePattern},
                                       QDir::Files
                                       | QDir::AllDirs
                                       | QDir::NoDotAndDotDot,
                                       QDir::Name);

    for (const QString &pluginFile: plugins) {
        auto pluginPath = pluginDir.absoluteFilePath(pluginFile);
        QPluginLoader pluginLoader(pluginPath);

        if (!pluginLoader.load())
        {
            qDebug() << "failed loading : " << pluginPath;
            qDebug() << "Error : " << pluginLoader.errorString();
            continue;
        }

        auto metaData = pluginLoader.metaData();

        if (metaData["MetaData"].toObject().contains("pluginType")
            && metaData["MetaData"].toObject()["pluginType"] == AK_PLUGIN_TYPE_SUBMODULE) {
            subModulesPaths << pluginPath;
        }
    }

    return subModulesPaths;
}

QStringList AkElement::listSubModulesPaths()
{
    QString pluginId;

    if (this->d->m_pluginId.isEmpty()) {
        pluginId = this->metaObject()->className();
        pluginId.replace(QRegularExpression("Element$"), "");
    } else {
        pluginId = this->d->m_pluginId;
    }

    return AkElement::listSubModulesPaths(pluginId);
}

QObject *AkElement::loadSubModule(const QString &pluginId,
                                  const QString &subModule)
{
    auto subModulesPaths = AkElement::listSubModulesPaths(pluginId);

    for (const QString &subModulesPath: subModulesPaths) {
        if (AkElementPrivate::pluginId(subModulesPath) == subModule) {
            QPluginLoader pluginLoader(subModulesPath);

            if (!pluginLoader.load()) {
                qDebug() << QString("Error loading submodule '%1' for '%2' plugin: %3")
                                .arg(subModule)
                                .arg(pluginId)
                                .arg(pluginLoader.errorString());

                return nullptr;
            }

            qDebug() << pluginId <<  " : SubModule Loaded: " << subModule;

            auto plugin = qobject_cast<AkPlugin *>(pluginLoader.instance());

            if (!plugin)
                return nullptr;

            auto obj = plugin->create(AK_PLUGIN_TYPE_SUBMODULE, "");
            delete plugin;

            return obj;
        }
    }

    return nullptr;
}

QObject *AkElement::loadSubModule(const QString &subModule)
{
    QString pluginId;

    if (this->d->m_pluginId.isEmpty()) {
        pluginId = this->metaObject()->className();
        pluginId.replace(QRegularExpression("Element$"), "");
    } else {
        pluginId = this->d->m_pluginId;
    }

    return AkElement::loadSubModule(pluginId, subModule);
}

bool AkElement::recursiveSearch()
{
    return akElementGlobalStuff->m_recursiveSearchPaths;
}

void AkElement::setRecursiveSearch(bool enable)
{
    akElementGlobalStuff->m_recursiveSearchPaths = enable;
}

QStringList AkElement::searchPaths()
{
    return akElementGlobalStuff->m_pluginsSearchPaths;
}

void AkElement::addSearchPath(const QString &path)
{
    auto absPath = akElementGlobalStuff->convertToAbsolute(path);

    if (!path.isEmpty()
        && QDir(absPath).exists()
        && !akElementGlobalStuff->m_pluginsSearchPaths.contains(absPath))
        akElementGlobalStuff->m_pluginsSearchPaths << absPath;
}

void AkElement::setSearchPaths(const QStringList &searchPaths)
{
    akElementGlobalStuff->m_pluginsSearchPaths.clear();

    for (const QString &path: searchPaths)
        AkElement::addSearchPath(path);
}

void AkElement::resetSearchPaths()
{
    akElementGlobalStuff->m_pluginsSearchPaths.clear();
}

QString AkElement::subModulesPath()
{
    return akElementGlobalStuff->m_subModulesPath;
}

void AkElement::setSubModulesPath(const QString &subModulesPath)
{
    akElementGlobalStuff->m_subModulesPath = subModulesPath;
}

void AkElement::resetSubModulesPath()
{
    akElementGlobalStuff->m_subModulesPath = SUBMODULES_PATH;
}

QStringList AkElement::listPlugins(const QString &type)
{
    if (!akElementGlobalStuff->m_pluginsScanned)
        akElementGlobalStuff->listPlugins();

    QStringList plugins;

    for (auto &pluginInfo: akElementGlobalStuff->m_pluginsList) {
        auto metaData = pluginInfo.m_metaData["MetaData"].toMap();

        if (!type.isEmpty()
            && metaData.contains("type")
            && metaData["type"] == type
            && !plugins.contains(pluginInfo.m_id))
            plugins << pluginInfo.m_id;
        else if (type.isEmpty()
                 && !plugins.contains(pluginInfo.m_id))
            plugins << pluginInfo.m_id;
    }

    plugins.sort();

    return plugins;
}

QStringList AkElement::listPluginPaths(const QString &searchPath)
{
    if (!akElementGlobalStuff->m_pluginsScanned)
        akElementGlobalStuff->listPlugins();

    QString searchDir(searchPath);

    searchDir.replace(QRegularExpression("((\\\\/?)|(/\\\\?))+"),
                                  QDir::separator());

    QStringList files;

    for (auto &pluginInfo: akElementGlobalStuff->m_pluginsList)
        if (pluginInfo.m_path.startsWith(searchDir))
            files << pluginInfo.m_path;

    return files;
}

QStringList AkElement::listPluginPaths(bool all)
{
    if (!akElementGlobalStuff->m_pluginsScanned)
        akElementGlobalStuff->listPlugins();

    QStringList files;

    for (auto &pluginInfo: akElementGlobalStuff->m_pluginsList)
        if (!pluginInfo.m_path.isEmpty()
            && !pluginInfo.m_id.isEmpty()
            && (all || pluginInfo.m_used))
            files << pluginInfo.m_path;

    qDebug() << "Plugin paths: " << files;
    return files;
}

void AkElement::setPluginPaths(const QStringList &paths)
{
    for (auto &path: paths) {
        bool contains = false;

        for (auto &pluginInfo: akElementGlobalStuff->m_pluginsList)
            if (pluginInfo.m_path == path) {
                contains = true;

                break;
            }

        if (!contains)
            akElementGlobalStuff->m_pluginsList
                    << AkPluginInfoPrivate {
                           akElementGlobalStuff->pluginId(path),
                           path,
                           {},
                           false
                       };
    }
}

QStringList AkElement::pluginsBlackList()
{
    return akElementGlobalStuff->m_pluginsBlackList;
}

void AkElement::setPluginsBlackList(const QStringList &blackList)
{
    akElementGlobalStuff->m_pluginsBlackList = blackList;
}

QString AkElement::pluginPath(const QString &pluginId)
{
    QStringList pluginPaths = AkElement::listPluginPaths();

    for (const QString &path: pluginPaths) {
        QString baseName = QFileInfo(path).baseName();

#ifdef Q_OS_WIN32
        if (baseName == pluginId)
            return path;
#else
        if (baseName == QString("lib%1").arg(pluginId))
            return path;
#endif
    }

    return QString();
}

QVariantMap AkElement::pluginInfo(const QString &pluginId)
{
    if (!akElementGlobalStuff->m_pluginsScanned)
        akElementGlobalStuff->listPlugins();

    for (auto &pluginInfo: akElementGlobalStuff->m_pluginsList)
        if (pluginInfo.m_id == pluginId)
            return pluginInfo.m_metaData;

    return QVariantMap();
}

void AkElement::setPluginInfo(const QString &path,
                              const QVariantMap &metaData)
{
    for (auto &pluginInfo: akElementGlobalStuff->m_pluginsList)
        if (pluginInfo.m_path == path) {
            pluginInfo.m_metaData = metaData;

            return;
        }

    akElementGlobalStuff->m_pluginsList
            << AkPluginInfoPrivate {
                   akElementGlobalStuff->pluginId(path),
                   path,
                   metaData,
                   false
               };
}

void AkElement::clearCache()
{
    akElementGlobalStuff->m_pluginsList.clear();
    akElementGlobalStuff->m_pluginsScanned = false;
}

AkPacket AkElement::operator ()(const AkPacket &packet)
{
    return this->iStream(packet);
}

AkPacket AkElement::operator ()(const AkAudioPacket &packet)
{
    return this->iStream(packet);
}

AkPacket AkElement::operator ()(const AkVideoPacket &packet)
{
    return this->iStream(packet);
}

QString AkElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString();
}

void AkElement::controlInterfaceConfigure(QQmlContext *context,
                                          const QString &controlId) const
{
    Q_UNUSED(context)
    Q_UNUSED(controlId)
}

void AkElement::stateChange(AkElement::ElementState from, AkElement::ElementState to)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
}

AkPacket AkElement::iStream(const AkPacket &packet)
{
    if (packet.caps().mimeType() == "audio/x-raw")
        return this->iStream(AkAudioPacket(packet));
    else if (packet.caps().mimeType() == "video/x-raw")
        return this->iStream(AkVideoPacket(packet));

    return AkPacket();
}

AkPacket AkElement::iStream(const AkAudioPacket &packet)
{
    Q_UNUSED(packet)

    return AkPacket();
}

AkPacket AkElement::iStream(const AkVideoPacket &packet)
{
    Q_UNUSED(packet)

    return AkPacket();
}

bool AkElement::setState(AkElement::ElementState state)
{
    if (this->d->m_state == state)
        return false;

    ElementState preState = this->d->m_state;
    this->d->m_state = state;

    switch (preState) {
    case ElementStateNull: {
        switch (state) {
        case ElementStatePaused:
            emit this->stateChanged(state);
            this->stateChange(preState, state);

            break;
        case ElementStatePlaying:
            emit this->stateChanged(ElementStatePaused);
            this->stateChange(preState, ElementStatePaused);

            emit this->stateChanged(state);
            this->stateChange(ElementStatePaused, state);

            break;
        case ElementStateNull:
            break;
        default:
            emit this->stateChanged(state);
        }

        break;
    }
    case ElementStatePaused: {
        switch (state) {
        case ElementStateNull:
        case ElementStatePlaying:
            emit this->stateChanged(state);
            this->stateChange(preState, state);

            break;
        case ElementStatePaused:
            break;
        default:
            emit this->stateChanged(state);
        }

        break;
    }
    case ElementStatePlaying: {
        switch (state) {
        case ElementStateNull:
            emit this->stateChanged(ElementStatePaused);
            this->stateChange(preState, ElementStatePaused);

            emit this->stateChanged(state);
            this->stateChange(ElementStatePaused, state);

            break;
        case ElementStatePaused:
            emit this->stateChanged(state);
            this->stateChange(preState, state);

            break;
        case ElementStatePlaying:
            break;
        default:
            emit this->stateChanged(state);
        }


        break;
    }
    default:
        emit this->stateChanged(state);
    }

    return true;
}

void AkElement::resetState()
{
    this->setState(ElementStateNull);
}

QDataStream &operator >>(QDataStream &istream, AkElement::ElementState &state)
{
    int stateInt;
    istream >> stateInt;
    state = static_cast<AkElement::ElementState>(stateInt);

    return istream;
}

QDataStream &operator <<(QDataStream &ostream, AkElement::ElementState state)
{
    ostream << static_cast<int>(state);

    return ostream;
}
