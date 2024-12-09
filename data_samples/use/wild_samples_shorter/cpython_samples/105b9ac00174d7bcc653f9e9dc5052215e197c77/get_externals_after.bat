echo.Fetching external libraries...

set libraries=
set libraries=%libraries%                                       bzip2-1.0.8
if NOT "%IncludeLibffiSrc%"=="false" set libraries=%libraries%  libffi-3.4.2
if NOT "%IncludeSSLSrc%"=="false" set libraries=%libraries%     openssl-1.1.1m
set libraries=%libraries%                                       sqlite-3.37.2.0
if NOT "%IncludeTkinterSrc%"=="false" set libraries=%libraries% tcl-core-8.6.12.1