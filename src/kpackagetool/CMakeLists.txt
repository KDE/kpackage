add_executable(kpackagetool5)
add_executable(KF5::kpackagetool5 ALIAS kpackagetool5)

ecm_mark_nongui_executable(kpackagetool5)

target_sources(kpackagetool5 PRIVATE
    main.cpp
    kpackagetool.cpp
    ../kpackage/private/packagejobthread.cpp
    ../kpackage/private/versionparser.cpp
    kpackagetool.qrc
)
ecm_qt_declare_logging_category(kpackagetool5
    HEADER kpackage_debug.h
    IDENTIFIER KPACKAGE_LOG
    CATEGORY_NAME kf.package
)
target_link_libraries(kpackagetool5 KF5::Archive KF5::Package KF5::I18n KF5::CoreAddons)

install(TARGETS kpackagetool5 EXPORT KF5PackageToolsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
