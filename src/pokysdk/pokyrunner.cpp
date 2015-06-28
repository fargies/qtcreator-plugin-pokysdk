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
 ** pokyrunner.cpp
 **
 **        Created on: 27/06/2015
 **   Original Author: fargie_s
 **
 **/

#include <QDebug>

#include "pokyrunner.hpp"

using namespace PokySDK;

class PokyRunner::Private
{
public:
    explicit Private(PokyRunner *runner) :
        q(runner)
    {
        process.setProcessChannelMode(QProcess::SeparateChannels);
    }

    QProcessEnvironment updateEnvironment(const QProcessEnvironment &environment);

    PokyRunner *q;
    QProcess process;
    QString envFile;
};

PokyRunner::PokyRunner(const QString &envFile, QObject *parent) :
    QProcess(parent),
    d(new PokyRunner::Private(this))
{
    setEnvFile(envFile);
}

PokyRunner::~PokyRunner()
{
    delete d;
}

QString PokyRunner::envFile() const
{
    return d->envFile;
}

void PokyRunner::setEnvFile(const QString &envFile)
{
    d->envFile = envFile;
    QProcess::setProcessEnvironment(d->updateEnvironment(QProcessEnvironment::systemEnvironment()));
}

void PokyRunner::setProcessEnvironment(const QProcessEnvironment &environment)
{
    QProcess::setProcessEnvironment(d->updateEnvironment(environment));
}

QProcessEnvironment PokyRunner::Private::updateEnvironment(const QProcessEnvironment &environment)
{
    QProcessEnvironment env(environment);
    QProcess proc;
    QStringList args;
    args << QLatin1String("-c")
         << QString(QLatin1String(". \"%1\"; env")).arg(envFile);

    proc.start(QLatin1String("sh"), args);
    if (!proc.waitForFinished(5000))
    {
        qWarning() << "[PokyRunner]: Failed to retrieve poky environment from" << envFile;
        proc.kill();
    }
    QByteArray buff;
    buff.reserve(4096);
    qint64 len;

    while ((len = proc.readLine(buff.data(), 4096)) > 0)
    {
        buff.resize(len - 1);
        QString line = QString::fromLatin1(buff);
        int idx = line.indexOf(QLatin1Char('='));
        if (idx < 0)
            qWarning() << "[PokyRunner]: Corrupted output" << line;
        else
            env.insert(line.left(idx), line.mid(idx + 1));
    }
    return env;
}

