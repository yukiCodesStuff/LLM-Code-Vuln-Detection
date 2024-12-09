#!/bin/sh
#
# Copyright 2017-2022 The OpenSSL Project Authors. All Rights Reserved.
# Copyright (c) 2017, Oracle and/or its affiliates.  All rights reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
rm -rf venv-cryptography
python -m venv venv-cryptography
. ./venv-cryptography/bin/activate
# Upgrade pip to always have latest
pip install -U pip

cd pyca-cryptography

echo "------------------------------------------------------------------"
echo "Building cryptography and installing test requirements"
echo "------------------------------------------------------------------"
LDFLAGS="-L$O_LIB" CFLAGS="-I$O_BINC -I$O_SINC " pip install .[test]
pip install -e vectors

echo "------------------------------------------------------------------"
echo "Print linked libraries"
echo "------------------------------------------------------------------"
ldd $(find ../venv-cryptography/lib/ -iname '*.so')


echo "------------------------------------------------------------------"
echo "Running tests"
echo "------------------------------------------------------------------"
pytest -n auto tests --wycheproof-root=../wycheproof

cd ../
deactivate
rm -rf venv-cryptography