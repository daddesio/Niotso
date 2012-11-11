LIBS = freetype libjpeg-turbo libmpg123 libpng libpq zlib
all: .PHONY $(addprefix ../_deps/, $(LIBS))
.PHONY: $(LIBS)
$(LIBS):
	wget -N http://niotso.org/pub/environment/windows/lib/$@.tar.xz
../_deps/%: ../_deps/%.tar.xz
ifdef CMD
	del /F /S /Q "$@"
else
	rm -rf "$@/*"
endif
	xzdec $< | tar -x
	echo . > "$@/temp"