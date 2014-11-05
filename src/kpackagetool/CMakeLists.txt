add_executable(kpackagetool
    main.cpp
    kpackagetool.cpp
)

target_link_libraries(kpackagetool KF5::Package KF5::I18n KF5::Service KF5::CoreAddons Qt5::DBus)

install(TARGETS kpackagetool ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

