/*
** Copyright (C) 2015 Fargier Sylvain <fargier.sylvain@free.fr>
**
** This software is provided 'as-is', without any express or implied
** warranty.  In no event will the authors be held liable for any damages
** arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose,
** including commercial applications, and to alter it and redistribute it
** freely, subject to the following restrictions:
**
** 1. The origin of this software must not be misrepresented; you must not
**    claim that you wrote the original software. If you use this software
**    in a product, an acknowledgment in the product documentation would be
**    appreciated but is not required.
** 2. Altered source versions must be plainly marked as such, and must not be
**    misrepresented as being the original software.
** 3. This notice may not be removed or altered from any source distribution.
**
** pokysdkplugin.cpp
**
**        Created on: juin 26, 2015
**   Original Author: Fargier Sylvain <fargier.sylvain@free.fr>
**
*/

#include "pokysdk.h"
#include "pokysdkplugin.h"

#include <projectexplorer/kitmanager.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/gcctoolchain.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/abi.h>
#include <projectexplorer/toolchainmanager.h>

#include <debugger/debuggeritemmanager.h>
#include <debugger/debuggeritem.h>
#include <debugger/debuggerkitinformation.h>

#include <qtsupport/qtversionfactory.h>
#include <qtsupport/qtversionmanager.h>
#include <qtsupport/qtkitinformation.h>

#include <remotelinux/remotelinux_constants.h>

#include <utils/environment.h>
#include <utils/fileutils.h>

#include <QtPlugin>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QProcess>

#include "pokyrunner.hpp"
#include "pokysdkkitinformation.h"

using namespace PokySDK;
using namespace PokySDK::Internal;
using namespace ProjectExplorer;
using namespace Debugger;
using namespace QtSupport;
using namespace Utils;
using namespace RemoteLinux;

PokySDKPlugin::PokySDKPlugin()
{
}

PokySDKPlugin::~PokySDKPlugin()
{
}

bool PokySDKPlugin::initialize(const QStringList & /*arguments*/, QString */*errorMessage*/)
{
    KitManager::registerKitInformation(new PokySDKKitInformation);
    connect(ProjectExplorer::KitManager::instance(), SIGNAL(kitsLoaded()),
            this, SLOT(kitsRestored()));
    return true;
}

void PokySDKPlugin::extensionsInitialized()
{
}

ProjectExplorer::Kit *PokySDKPlugin::detectKit(const QString &name, const QString &envFile)
{
    PokyRunner runner(envFile);

    QProcessEnvironment env = runner.processEnvironment();
    QString targetTriplet = env.value(QLatin1String("TARGET_PREFIX"));
    targetTriplet.chop(1);
    if (targetTriplet.isEmpty())
    {
        qWarning() << "[PokySDK]: invalid poky SDK" << envFile;
        return 0;
    }

    QString compiler = QString(QLatin1String("%1/usr/bin/%2/%2-g++"))
            .arg(env.value(QLatin1String("OECORE_NATIVE_SYSROOT")), targetTriplet);
    ToolChain *tcObject = findToolChainCompiler(compiler);
    if (!QFile::exists(compiler))
    {
        qWarning() << "[PokySDK]: failed to find compiler" << compiler;
        if (tcObject)
            ToolChainManager::deregisterToolChain(tcObject);
        tcObject = 0;
        compiler.clear();
    }
    else if (!tcObject)
    {
        GccToolChain *toolchain = new GccToolChain(
                    QLatin1String(Constants::POKYSDK_TOOLCHAIN_ID),
                    ToolChain::AutoDetection);
        toolchain->setDisplayName(name + QLatin1String(" toolchain"));

        toolchain->setCompilerCommand(FileName::fromString(compiler));
//        toolchain->setTargetAbi(Abi::abiFromTargetTriplet(targetTriplet));


        if (!ToolChainManager::registerToolChain(toolchain))
            qWarning() << "[PokySDK]: failed to register toolchain";
        else
            tcObject = toolchain;
    }

    QString debugger = QString(QLatin1String("%1/usr/bin/%2/%2-gdb"))
            .arg(env.value(QLatin1String("OECORE_NATIVE_SYSROOT")), targetTriplet);
    const DebuggerItem *dbgObject = DebuggerItemManager::findByCommand(FileName::fromString(debugger));
    if (!QFile::exists(debugger))
    {
        qWarning() << "[PokySDK]: failed to find debugger" << debugger;
        if (dbgObject)
            DebuggerItemManager::deregisterDebugger(dbgObject->id());
        dbgObject = 0;
        debugger.clear();
    }
    else if (tcObject && !dbgObject)
    {
        DebuggerItem dbgItem;
        dbgItem.setCommand(FileName::fromString(debugger));
        dbgItem.setEngineType(Debugger::GdbEngineType);
        dbgItem.setDisplayName(name + QLatin1String(" debugger"));
        dbgItem.setAutoDetected(true);
        dbgItem.setAbi(tcObject->targetAbi());
        QVariant id = DebuggerItemManager::registerDebugger(dbgItem);
        dbgObject = DebuggerItemManager::findById(id);
    }

    QString qt5 = QString(QLatin1String("%1/usr/bin/qt5/qmake"))
            .arg(env.value(QLatin1String("OECORE_NATIVE_SYSROOT")));
    BaseQtVersion *qt5Object = QtVersionManager::qtVersionForQMakeBinary(FileName::fromString(qt5));
    if (!QFile::exists(qt5))
    {
        qWarning() << "[PokySDK]: failed to find Qt5" << qt5;
        if (qt5Object)
            QtVersionManager::removeVersion(qt5Object);
        debugger.clear();
    }
    else if (!qt5Object)
    {
        QString error;
        qt5Object = QtVersionFactory::createQtVersionFromQMakePath(
                    FileName::fromString(qt5), true,
                    QLatin1String(Constants::POKYSDKQT), &error);
        if (!qt5Object)
            qWarning() << "[PokySDK]: failed to register Qt version" << error;
        else
        {
            qt5Object->setDisplayName(name + QLatin1String(" Qt ") + qt5Object->qtVersionString());
            QtVersionManager::addVersion(qt5Object);
        }
    }

    QString sysRoot = env.value(QLatin1String("SDKTARGETSYSROOT"));
    Kit *kit = 0;
    if (sysRoot.isEmpty())
        qWarning() << "[PokySDK]: failed to find target sysroot";
    else if (!tcObject)
        qWarning() << "[PokySDK]: no compiler found, not creating a Kit";
    else if (!(kit = findKitSysroot(sysRoot)))
    {
        kit = new Kit;
        kit->setAutoDetected(true);
        kit->setDisplayName(name);
        kit->setIconPath(FileName::fromString(QLatin1String(Constants::POKY_ICON)));
        DeviceTypeKitInformation::setDeviceTypeId(kit, RemoteLinux::Constants::GenericLinuxOsType);
        SysRootKitInformation::setSysRoot(kit, FileName::fromString(sysRoot));

        KitManager::registerKit(kit);
    }
    if (kit)
    {
        if (tcObject)
            ToolChainKitInformation::setToolChain(kit, tcObject);
        if (qt5Object)
            QtKitInformation::setQtVersion(kit, qt5Object);
        if (dbgObject)
            DebuggerKitInformation::setDebugger(kit, dbgObject->id());

        kit->setMutable(DeviceKitInformation::id(), true);
        kit->setSticky(QtKitInformation::id(), true);
        kit->setSticky(ToolChainKitInformation::id(), true);
        kit->setSticky(DeviceTypeKitInformation::id(), false);
        kit->setSticky(SysRootKitInformation::id(), true);
        kit->setSticky(DebuggerKitInformation::id(), true);
    }
    return kit;
}

ProjectExplorer::ToolChain *PokySDKPlugin::findToolChainCompiler(const QString &compiler)
{
    FileName fileName = FileName::fromString(compiler);
    foreach (ToolChain *tc, ToolChainManager::toolChains())
    {
        if ((tc->id() == QLatin1String(Constants::POKYSDK_TOOLCHAIN_ID)) &&
                (tc->compilerCommand() == fileName))
            return tc;
    }
    return 0;
}

Kit *PokySDKPlugin::findKitSysroot(const QString &sysroot)
{
    FileName sysRootFileName = FileName::fromString(sysroot);
    foreach (Kit *kit, KitManager::kits())
    {
        if (SysRootKitInformation::sysRoot(kit) == sysRootFileName)
            return kit;
    }
    return 0;
}

void PokySDKPlugin::clearOldKits(const QList<Kit *> &validKits)
{
    foreach (Kit *kit, KitManager::kits())
    {
        if (validKits.contains(kit))
            continue;
        else if (PokySDKKitInformation::isPokySDKKit(kit))
        {
            qDebug() << "[PokySDK]: removing old kit" << kit->displayName();
            KitManager::deregisterKit(kit);
        }
    }
}

void PokySDKPlugin::kitsRestored()
{
    QList<Kit *> kitList;
    QDir dirs(QLatin1String("/opt"));
    foreach (QString optFile, dirs.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir optDir(dirs.filePath(optFile));

        foreach (QString tcFile, optDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QDir tcDir(optDir.filePath(tcFile));

            foreach (QString envFile, tcDir.entryList(
                         QStringList() << QLatin1String("environment-setup*"), QDir::Files)) {
                QString name = QString(QLatin1String("Poky %1 (%2)")).arg(optFile, tcFile);
                Kit *kit = detectKit(name, tcDir.filePath(envFile));
                if (kit)
                    kitList << kit;
            }
        }
    }

    clearOldKits(kitList);
}

Q_EXPORT_PLUGIN(PokySDKPlugin)

