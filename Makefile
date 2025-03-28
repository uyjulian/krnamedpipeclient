
SOURCES += main.cpp

PROJECT_BASENAME = krnamedpipeclient

RC_DESC ?= Named Pipe client for TVP(KIRIKIRI) (2/Z)
RC_PRODUCTNAME ?= Named Pipe client for TVP(KIRIKIRI) (2/Z)
RC_LEGALCOPYRIGHT ?= Copyright (C) 2020-2021 Julian Uy; This product is licensed under the MIT license.

include external/ncbind/Rules.lib.make
