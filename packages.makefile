####
## [Packages]

all: client filehandler server fardive

include Client/Makefile
include Libraries/FileHandler/Makefile
include Server/Makefile
include Tools/FARDive/Makefile