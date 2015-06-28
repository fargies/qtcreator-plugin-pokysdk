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
** pokysdkplugin.h
**
**        Created on: juin 26, 2015
**   Original Author: Fargier Sylvain <fargier.sylvain@free.fr>
**
*/

#ifndef POKYSDK_H
#define POKYSDK_H

#include <extensionsystem/iplugin.h>

#include <QObject>
#include <QList>

namespace ProjectExplorer {
class ToolChain;
class Kit;
}

namespace Debugger {
class DebuggerItem;
}

namespace PokySDK {

namespace Constants {
const char POKYSDK_TOOLCHAIN_ID[] = "PokySDK.ToolChain";
const char POKYSDKQT[] = "PokySDK.QtVersion";
const char POKY_ICON[] = ":/pokysdk/images/poky.png";
}

namespace Internal {

class PokySDKPlugin
  : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "PokySDK.json")

public:
    PokySDKPlugin();
    ~PokySDKPlugin();

    bool initialize(const QStringList &arguments, QString *errorMessage);

    void extensionsInitialized();

protected:
    ProjectExplorer::Kit *detectKit(const QString &name, const QString &envFile);
    ProjectExplorer::ToolChain *findToolChainCompiler(const QString &compiler);
    ProjectExplorer::Kit *findKitSysroot(const QString &sysroot);

    void clearOldKits(const QList<ProjectExplorer::Kit *> &validKits);

protected slots:
    void kitsRestored();
};

} // namespace Internal
} // namespace PokySDK

#endif // POKYSDK_H

