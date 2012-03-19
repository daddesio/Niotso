#define VERSION_A 0
#define VERSION_B 0
#define VERSION_C 0
#define VERSION_D 0

//You don't have to touch the following
#define xstr(x) str(x)
#define str(x) #x //Yes, double levels is required. See <http://gcc.gnu.org/onlinedocs/cpp/Stringification.html>
#define VERSIONSTR "" \
	xstr(VERSION_A) "." xstr(VERSION_B) "." xstr(VERSION_C) "." xstr(VERSION_D)
