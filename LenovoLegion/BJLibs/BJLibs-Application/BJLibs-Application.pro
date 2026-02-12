#-------------------------------------------------
#
# Project created by QtCreator 2015-12-17T15:45:58
#
#-------------------------------------------------
TEMPLATE        = lib
CONFIG          += staticlib
TARGET          = $${PROJECT_LIBS_NAME}

# No generate unique section => unloadable from memory
# E.g. a common Linux issue is if you created a library that uses STB_GNU_UNIQUE symbols,
# that library is marked as "not unloadable" and thus will simply never be unloaded.
# Try running readelf -Ws on your library and look for objects tagged as UNIQUE.
QMAKE_CXXFLAGS += --no-gnu-unique

QT += core5compat core widgets

DESTDIR = $${DESTINATION_LIB_PATH}

SOURCES += \
            ConnectX/ConnectXBoardBase.cpp \
            ConnectX/ConnectXBoardTracker.cpp \
            ConnectX/ConnectXBoardTrackerBase.cpp \
            ConnectX/ConnectXGameProbe.cpp \
            ConnectX/ConnectXGameSimulation.cpp \
            ConnectX/ConnectXGameStats.cpp \
            ConnectX/ConnectXRender.cpp \
            ConnectX/ConnectXScore.cpp \
            Core/ApplicationModulesHandler.cpp \
            Core/Application.cpp \
            Core/ApplicationModulesLoader.cpp \
            Core/ExceptionBuilder.cpp \
            Core/Exception.cpp \
            Core/Logger.cpp \
            Core/RunGuard.cpp \
            Core/StackTrace.cpp\
            Dbf/QDbfTable.cpp \
            Dbf/QDbfField.cpp \
            Dbf/QdbfRecord.cpp \
            ConnectX/ConnectXProtocol.cpp\
            BigInt/BigInteger.cpp\
            BigInt/BigIntegerAlgorithms.cpp\
            BigInt/BigIntegerUtils.cpp\
            BigInt/BigUnsigned.cpp\
            BigInt/BigUnsignedInABase.cpp

HEADERS += \
            ConnectX/ConnectXBoardBase.h \
            ConnectX/ConnectXBoardTracker.h \
            ConnectX/ConnectXBoardTrackerBase.h \
            ConnectX/ConnectXBoardTypes.h \
            ConnectX/ConnectXGamePlugin.h \
            ConnectX/ConnectXGameProbe.h \
            ConnectX/ConnectXGameSimulation.h \
            ConnectX/ConnectXGameStats.h \
            ConnectX/ConnectXRender.h \
            ConnectX/ConnectXScore.h \
            ConnectX/ConnectXSignedNumber.h \
            Core/ApplicationEnviroment.h \
            Core/Application.h \
            Core/ApplicationInterface.h \
            Core/ApplicationModulesLoader.h \
            Core/ApplicationPlugin.h \
            Core/ApplicationModulesHandler.h \
            Core/ExceptionBuilder.h \
            Core/Exception.h \
            Core/Logger.h \
            Core/LoggerHolder.h \
            Core/RunGuard.h \
            Core/StackTrace.h \
            Dbf/QDbfTable.h \
            Dbf/QDbfField.h \
            Dbf/QdbfRecord.h \
            Singleton/SingletonHolder.h \
            Singleton/CPP11ThreadModel.h \
            Singleton/StaticCreationPolicy.h \
            ConnectX/ConnectXProtocol.h \
            ConnectX/ConnectXBoard.h \
            BigInt/BigInteger.h\
            BigInt/BigIntegerAlgorithms.h\
            BigInt/BigIntegerLibrary.h\
            BigInt/BigIntegerUtils.h\
            BigInt/BigUnsigned.h\
            BigInt/BigUnsignedInABase.h\
            BigInt/NumberlikeArray.h

unix {

    SOURCES += \
            OS/Linux/Signal.cpp

    HEADERS += \
            OS/Linux/Signal.h
}

DEFINES        += BL_DATA_DIR_DEFINE=$${DESTINATION_DATA_DIR}
DEFINES        += BL_LOG_DIR_DEFINE=$${DESTINATION_LOG_DIR}
DEFINES        += BL_APP_NAME_DEFINE=$${APPLICATION_NAME}
DEFINES        += BL_APP_NAME_DEFINE2=$${APPLICATION_NAME2}
DEFINES        += BL_APP_NAME_DEFINE3=$${APPLICATION_NAME3}
DEFINES        += BL_APP_NAME_DEFINE4=$${APPLICATION_NAME4}
DEFINES        += BL_APP_NAME_DEFINE5=$${APPLICATION_NAME5}
DEFINES        += BL_APP_NAME_DEFINE6=$${APPLICATION_NAME6}
DEFINES        += BL_APP_NAME_DEFINE7=$${APPLICATION_NAME7}
DEFINES        += BL_APP_NAME_DEFINE8=$${APPLICATION_NAME8}
DEFINES        += BL_APP_NAME_DEFINE9=$${APPLICATION_NAME9}
DEFINES        += BL_APP_NAME_DEFINE10=$${APPLICATION_NAME10}
DEFINES        += BL_MODULES_DIR_DEFINE=$${DESTINATION_MODULES_DIR}
DEFINES        += BL_APP_VERSION_DEFINE=$${APPLICATION_VERSION}

windows {
LIBS += -lDbgHelp
}
