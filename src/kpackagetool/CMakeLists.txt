add_executable(kpackagetool6)
add_executable(KF6::kpackagetool ALIAS kpackagetool6)

ecm_mark_nongui_executable(kpackagetool6)

target_sources(kpackagetool6 PRIVATE
    main.cpp
    kpackagetool.cpp
    kpackagetool.qrc
)
ecm_qt_declare_logging_category(kpackagetool6
    HEADER kpackage_debug.h
    IDENTIFIER KPACKAGE_LOG
    CATEGORY_NAME kf.package
)
target_link_libraries(kpackagetool6 kpackage_common_STATIC KF6::Archive KF6::Package KF6::I18n KF6::CoreAddons)

install(TARGETS kpackagetool6 EXPORT KF6PackageToolsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
