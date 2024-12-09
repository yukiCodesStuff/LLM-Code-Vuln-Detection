set libraries=
set libraries=%libraries%                                       bzip2-1.0.8
if NOT "%IncludeLibffiSrc%"=="false" set libraries=%libraries%  libffi-3.4.3
if NOT "%IncludeSSLSrc%"=="false" set libraries=%libraries%     openssl-1.1.1t
set libraries=%libraries%                                       sqlite-3.39.4.0
if NOT "%IncludeTkinterSrc%"=="false" set libraries=%libraries% tcl-core-8.6.13.0
if NOT "%IncludeTkinterSrc%"=="false" set libraries=%libraries% tk-8.6.13.0
if NOT "%IncludeTkinterSrc%"=="false" set libraries=%libraries% tix-8.4.3.6

set binaries=
if NOT "%IncludeLibffi%"=="false"  set binaries=%binaries% libffi-3.4.3
if NOT "%IncludeSSL%"=="false"     set binaries=%binaries% openssl-bin-1.1.1t
if NOT "%IncludeTkinter%"=="false" set binaries=%binaries% tcltk-8.6.13.0
if NOT "%IncludeSSLSrc%"=="false"  set binaries=%binaries% nasm-2.11.06

for %%b in (%binaries%) do (