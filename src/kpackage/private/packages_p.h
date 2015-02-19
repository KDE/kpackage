/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef LIBS_KPACKAGE_PACKAGES_P_H
#define LIBS_KPACKAGE_PACKAGES_P_H

#include "kpackage/packagestructure.h"

class ChangeableMainScriptPackage : public KPackage::PackageStructure
{
public:
    void initPackage(KPackage::Package *package) Q_DECL_OVERRIDE;

protected:
    virtual QString mainScriptConfigKey() const;
    void pathChanged(KPackage::Package *package) Q_DECL_OVERRIDE;
};

class GenericPackage : public ChangeableMainScriptPackage
{
public:
    void initPackage(KPackage::Package *package) Q_DECL_OVERRIDE;
};

class GenericQMLPackage : public GenericPackage
{
public:
    void initPackage(KPackage::Package *package) Q_DECL_OVERRIDE;
};

#endif // LIBS_KPACKAGE_PACKAGES_P_H
