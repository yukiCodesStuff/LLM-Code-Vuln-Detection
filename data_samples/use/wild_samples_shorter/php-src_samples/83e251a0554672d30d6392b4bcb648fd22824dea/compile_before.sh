#!/bin/bash
./buildconf
./configure --quiet \
--with-pdo-mysql \
--with-mysql \
--with-mysqli \
--with-pgsql \
--with-pdo-pgsql \
--with-pdo-sqlite \
--enable-intl \