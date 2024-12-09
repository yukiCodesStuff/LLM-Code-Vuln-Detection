VERSION=5.45
if [[ ! -d libmagic.orig ]]; then
  mkdir libmagic.orig
  wget -O - ftp://ftp.astron.com/pub/file/file-$VERSION.tar.gz \
    | tar -xz --strip-components=2 -C libmagic.orig file-$VERSION/src