TEMPLATE = subdirs

CONFIG += c++20

SUBDIRS +=  \
            BJLibs-Application\
            BJLibs-UnitTests

BJLibs-UnitTests.depends =  BJLibs-Application
