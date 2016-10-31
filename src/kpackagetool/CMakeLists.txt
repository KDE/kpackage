add_executable(kpackagetool5
    main.cpp
    kpackagetool.cpp
    ../kpackage/private/packagejobthread.cpp
    ../kpackage/private/versionparser.cpp
)

add_executable(KF5::kpackagetool5 ALIAS kpackagetool5)
target_link_libraries(kpackagetool5 KF5::Archive KF5::Package KF5::I18n KF5::CoreAddons)
ecm_mark_nongui_executable(kpackagetool5)

install(TARGETS kpackagetool5 EXPORT KF5PackageTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
