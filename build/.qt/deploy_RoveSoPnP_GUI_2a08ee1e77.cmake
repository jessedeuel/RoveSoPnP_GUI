include("/home/jessedeuel/Documents/Misc/RoveSoPnP_GUI/build/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/RoveSoPnP_GUI-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "/home/jessedeuel/Documents/Misc/RoveSoPnP_GUI/build/RoveSoPnP_GUI"
    GENERATE_QT_CONF
)
