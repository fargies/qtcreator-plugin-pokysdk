/**
 ** Copyright (C) 2015 fargie_s
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
 ** pokysdkkitinformation.cpp
 **
 **        Created on: 28/06/2015
 **   Original Author: fargie_s
 **
 **/

#include <QFileInfo>
#include <QFile>

#include <projectexplorer/toolchain.h>
#include <utils/environment.h>

#include "pokysdkkitinformation.h"
#include "pokysdkplugin.h"
#include "pokyrunner.hpp"

using namespace PokySDK;
using namespace ProjectExplorer;
using namespace Utils;

PokySDKKitInformation::PokySDKKitInformation() :
    KitInformation()
{
    setId(PokySDKKitInformation::id());
}

QVariant PokySDKKitInformation::defaultValue(Kit *kit) const
{
    return environmentFile(kit).toString();
}

QList<Task> PokySDKKitInformation::validate(const Kit */*kit*/) const
{
    return QList<Task>();
}

KitInformation::ItemList PokySDKKitInformation::toUserOutput(const Kit *kit) const
{
    return KitInformation::ItemList()
            << qMakePair(tr("Poky Environment"), environmentFile(kit).toUserOutput());
}

KitConfigWidget *PokySDKKitInformation::createConfigWidget(Kit */*kit*/) const
{
    return 0;
}

Core::Id PokySDKKitInformation::id()
{
    return "PokySDK.Information";
}

bool PokySDKKitInformation::isPokySDKKit(const Kit *kit)
{
    if (!kit->isAutoDetected())
        return false;
    ToolChain *tc = ToolChainKitInformation::toolChain(kit);
    if (tc && !findEnvFromCompiler(tc->compilerCommand()).isEmpty())
        return true;
    else if (!findEnvFromSysroot(SysRootKitInformation::sysRoot(kit)).isEmpty())
        return true;
    else
        return false;
}

FileName PokySDKKitInformation::environmentFile(const Kit *kit)
{
    FileName envFile;
    ToolChain *tc = ToolChainKitInformation::toolChain(kit);
    if (tc)
    {
        envFile = findEnvFromCompiler(tc->compilerCommand());
        if (QFile::exists(envFile.toString()))
            return envFile;
    }

    envFile = findEnvFromSysroot(SysRootKitInformation::sysRoot(kit));
    if (QFile::exists(envFile.toString()))
        return envFile;
    else
        return FileName();
}

void PokySDKKitInformation::addToEnvironment(const Kit *kit, Environment &env) const
{
    FileName pokyEnvFile = environmentFile(kit);
    if (pokyEnvFile.isEmpty())
        return;
    PokyRunner runner(pokyEnvFile.toString());
    QProcessEnvironment pokyEnv = runner.processEnvironment();
    foreach (QString key, pokyEnv.keys())
        env.set(key, pokyEnv.value(key));
}

FileName PokySDKKitInformation::findEnvFromSysroot(const FileName &sysRoot)
{
    const QString sysRootStr = sysRoot.toString();
    int idx = sysRootStr.indexOf(QLatin1String("/sysroots/"));
    if (idx < 0)
        return FileName();
    QString envFile = QString(QLatin1String("%1/environment-setup-%2"))
            .arg(sysRootStr.left(idx), sysRoot.toFileInfo().fileName());
    return FileName::fromString(envFile);
}

FileName PokySDKKitInformation::findEnvFromCompiler(const FileName &compilerCmd)
{
    const QString compilerCmdStr = compilerCmd.toString();
    int idx = compilerCmdStr.indexOf(QLatin1String("/sysroots/"));
    if (idx < 0)
        return FileName();
    QString target = compilerCmd.toFileInfo().fileName().remove(QLatin1String("-g++"));
    QString envFile = QString(QLatin1String("%1/environment-setup-%2"))
            .arg(compilerCmdStr.left(idx), target);
    return FileName::fromString(envFile);
}
