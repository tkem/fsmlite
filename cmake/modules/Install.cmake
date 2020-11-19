# Install
if(NOT CMAKE_INSTALL_BINDIR)
  include(GNUInstallDirs)
endif()

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Install public headers
install(
    FILES ${PUBLIC_HEADERS}
    DESTINATION "include/${PROJECT_NAME}"
)

# Specify binary destinations for the targets
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # This is for Windows
)

# Export the targets to a script
install(
  EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake INSTALL_DESTINATION
  ${INSTALL_CONFIGDIR}
)

# Install the config, configversion and custom find modules
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(
  EXPORT ${PROJECT_NAME}-targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
)