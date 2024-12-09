if NOT "%IncludeTkinterSrc%"=="false" set libraries=%libraries% tk-8.6.12.1
if NOT "%IncludeTkinterSrc%"=="false" set libraries=%libraries% tix-8.4.3.6
set libraries=%libraries%                                       xz-5.2.2
set libraries=%libraries%                                       zlib-1.2.11

for %%e in (%libraries%) do (
    if exist "%EXTERNALS_DIR%\%%e" (
        echo.%%e already exists, skipping.