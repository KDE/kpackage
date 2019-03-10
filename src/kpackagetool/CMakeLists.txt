set(CMAKE_AUTORCC TRUE)
set(kpackagetool5_debug_SRCS)
ecm_qt_declare_logging_category(kpackagetool5_debug_SRCS HEADER kpackage_debug.h IDENTIFIER KPACKAGE_LOG CATEGORY_NAME kf5.kpackage)
add_executable(kpackagetool5
    main.cpp
    kpackagetool.cpp
    ../kpackage/private/packagejobthread.cpp
    ../kpackage/private/versionparser.cpp
    kpackagetool.qrc
    ${kpackagetool5_debug_SRCS}
)
add_executable(KF5::kpackagetool5 ALIAS kpackagetool5)
target_link_libraries(kpackagetool5 KF5::Archive KF5::Package KF5::I18n KF5::CoreAddons)
ecm_mark_nongui_executable(kpackagetool5)

install(TARGETS kpackagetool5 EXPORT KF5PackageToolsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
