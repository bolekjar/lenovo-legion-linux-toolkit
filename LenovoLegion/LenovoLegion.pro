TEMPLATE = subdirs

SUBDIRS +=                          \
    LenovoLegion-PrepareBuild       \
    LenovoLegion-Messages           \
    BJLibs                          \
    LenovoLegion-DaemonSDK          \
    LenovoLegion-DaemonModules      \
    LenovoLegion-Daemon             \
    LenovoLegion-Application

LenovoLegion-Messages.depends = LenovoLegion-PrepareBuild
LenovoLegion-Application.depends = LenovoLegion-PrepareBuild LenovoLegion-Messages BJLibs
LenovoLegion-DaemonModules.depends = LenovoLegion-PrepareBuild LenovoLegion-Messages
LenovoLegion-Daemon.depends = LenovoLegion-PrepareBuild LenovoLegion-Messages BJLibs

DISTFILES +=     \
    .qmake.conf  \
    ToDoList.txt \
    README.md    \
    .gitignore
