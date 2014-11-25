add_executable(kpackagetool
    main.cpp
    kpackagetool.cpp
)

target_link_libraries(kpackagetool KF5::Package KF5::I18n KF5::ConfigCore KF5::CoreAddons)

install(TARGETS kpackagetool ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

