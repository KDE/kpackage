/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMOIDSTRUCTURE_P_H
#define PLASMOIDSTRUCTURE_P_H

#include "kpackage/packagestructure.h"

namespace KPackage
{
class PlasmoidPackage : public KPackage::PackageStructure
{
    Q_OBJECT

public:
    // Needed for K_PLUGIN_CLASS_WITH_JSON macro
    explicit PlasmoidPackage(QObject * /*parent*/, const QVariantList & /*args*/ = {})
        : KPackage::PackageStructure()
    {
    }
    void initPackage(Package *package) override;
};

} // namespace KPackage

#endif // PLASMOIDSTRUCTURE_P_H
