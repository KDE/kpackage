/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIBS_KPACKAGE_PACKAGES_P_H
#define LIBS_KPACKAGE_PACKAGES_P_H

#include "kpackage/packagestructure.h"

class ChangeableMainScriptPackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(KPackage::Package *package) override;

protected:
    virtual QString mainScriptConfigKey() const;
    void pathChanged(KPackage::Package *package) override;
};

class GenericPackage : public ChangeableMainScriptPackage
{
    Q_OBJECT
public:
    void initPackage(KPackage::Package *package) override;
};

class GenericQMLPackage : public GenericPackage
{
    Q_OBJECT
public:
    void initPackage(KPackage::Package *package) override;
};

#endif // LIBS_KPACKAGE_PACKAGES_P_H
