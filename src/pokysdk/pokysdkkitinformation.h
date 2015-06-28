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
 ** pokysdkkitinformation.h
 **
 **        Created on: 28/06/2015
 **   Original Author: fargie_s
 **
 **/

#ifndef POKYSDKKITINFORMATION_H
#define POKYSDKKITINFORMATION_H

#include <projectexplorer/kitinformation.h>
#include <utils/fileutils.h>

namespace PokySDK {

class PokySDKKitInformation : public ProjectExplorer::KitInformation
{
    Q_OBJECT
public:
    explicit PokySDKKitInformation();

    QVariant defaultValue(ProjectExplorer::Kit *kit) const;
    QList<ProjectExplorer::Task> validate(const ProjectExplorer::Kit *kit) const;

    ItemList toUserOutput(const ProjectExplorer::Kit *kit) const;

    ProjectExplorer::KitConfigWidget *createConfigWidget(ProjectExplorer::Kit *kit) const;

    static Core::Id id();
    static bool isPokySDKKit(const ProjectExplorer::Kit *kit);
    static Utils::FileName environmentFile(const ProjectExplorer::Kit *kit);

protected:
    static Utils::FileName findEnvFromSysroot(const Utils::FileName &sysRoot);
    static Utils::FileName findEnvFromCompiler(const Utils::FileName &compilerCommand);

    QString m_envFile;

};

}

#endif // POKYSDKKITINFORMATION_H
