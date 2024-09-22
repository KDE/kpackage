#compdef kpackagetool6

# SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

local -aU package_names
local -aU package_types

_arguments \
  "(- *)"{-v,--version}"[Displays version information]" \
  "(- *)"{-h,--help}"[Displays help on commandline options]" \
  "(- *)"{-h,--help}"[Displays help, including generic Qt options]" \
  "(--hash)"{--hash}"[Generate a SHA1 hash for the package at <path>]" \
  "(-g --global)""{-g,--global}""For install or remove, operates on packages installed for all users" \
  "(-t --type)""{-t,--type}""The type of package, corresponding to the service type of the package plugin":::_package_types \
  "(-i --install)""{-i,--install}""[Install the package at <path>]"":::_files" \
  "(-s --show)""{-s,--show}""[Show information of package <name>]":::_package_names \
  "(-u --upgrade)""{-u,--upgrade}""[Upgrade the package at <path>]"":::_files" \
  "(-l --list)""{-l,--list}""[List installed packages]" \
  "(--list-types)""{--list-types}""[List all known package types that can be installed]" \
  "(-r --remove)""{-r,--remove}""{Remove the package named <name>}":::_package_names \
  "(-p --packageroot)""{-p,--packageroot}""[Absolute path to the package root]"":::_files" \
  "(--appstream-metainfo)""{--appstream-metainfo}""[Outputs the metadata for the package <path>]":::_files \
  "(--appstream-metainfo-output)""{--appstream-metainfo-output}""[Outputs the metadata for the package into <path>]":::_files \
