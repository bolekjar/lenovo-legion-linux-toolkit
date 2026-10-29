#-------------------------------------------------
#
# Use of the shared protocol messages library (LenovoLegion-Messages)
#
#   include(../LenovoLegion-Messages/LenovoLegion-Messages.pri)
#
# The project must be in DESTINATION_BIN_PATH or set its own rpath to it.
#
#-------------------------------------------------
LIBS            += -L$${DESTINATION_BIN_PATH} -lLenovoLegionMessages
PRE_TARGETDEPS  += $${DESTINATION_BIN_PATH}/libLenovoLegionMessages.so

# Find libLenovoLegionMessages.so next to the binary (development build and /opt/LenovoLegion)
QMAKE_LFLAGS    += '-Wl,-rpath,\'\$$ORIGIN\''
