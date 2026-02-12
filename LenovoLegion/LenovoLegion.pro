TEMPLATE = subdirs

SUBDIRS +=                          \
    LenovoLegion-PrepareBuild       \
    BJLibs                          \
    LenovoLegion-Daemon             \
    LenovoLegion-Application

LenovoLegion-Application.depends = LenovoLegion-PrepareBuild BJLibs
LenovoLegion-Daemon.depends = LenovoLegion-PrepareBuild BJLibs

DISTFILES +=     \
    .qmake.conf  \
    ToDoList.txt \
    README.md    \
    .gitignore
