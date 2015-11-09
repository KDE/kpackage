add_executable(kpackagetool5
    main.cpp
    kpackagetool.cpp
    ../kpackage/private/packagejobthread.cpp
    ../kpackage/private/versionparser.cpp
)

target_link_libraries(kpackagetool5 KF5::Archive KF5::Package KF5::I18n KF5::ConfigCore KF5::CoreAddons)

install(TARGETS kpackagetool5 ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

