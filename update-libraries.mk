LIBS = freetype libjpeg-turbo libmpg123 libpng libpq zlib
all: .PHONY ../_deps/$(LIBS)
.PHONY: $(LIBS)
	wget -N http://niotso.org/pub/environment/windows/lib/$<.tar.xz
../_deps/%: %.tar.xz
ifdef CMD
	del /F /S /Q "$@"
else
	rm -rf "$@/*"
endif
	xzdec $< | tar -x
	echo . > "$@/temp"