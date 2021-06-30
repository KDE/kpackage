/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMOIDSTRUCTURE_P_H
#define PLASMOIDSTRUCTURE_P_H

#include "../src/kpackage/private/packages_p.h"
#include "kpackage/packagestructure.h"

namespace KPackage
{
class PlasmoidPackage : public GenericPackage
{
    Q_OBJECT

public:
    // Needed for K_PLUGIN_CLASS_WITH_JSON macro
    explicit PlasmoidPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : GenericPackage()
    {
        Q_UNUSED(parent)
        Q_UNUSED(args)
    }
    void initPackage(Package *package) override;
};

} // namespace KPackage

#endif // PLASMOIDSTRUCTURE_P_H
