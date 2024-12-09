#!/bin/sh
#
# Copyright 2017-2021 The OpenSSL Project Authors. All Rights Reserved.
# Copyright (c) 2017, Oracle and/or its affiliates.  All rights reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
rm -rf venv-cryptography
python -m venv venv-cryptography
. ./venv-cryptography/bin/activate

cd pyca-cryptography

pip install .[test]
pip install -e vectors

echo "------------------------------------------------------------------"
echo "Building cryptography"
echo "------------------------------------------------------------------"
CFLAGS="-I$O_BINC -I$O_SINC -L$O_LIB" pip install .

echo "------------------------------------------------------------------"
echo "Running tests"
echo "------------------------------------------------------------------"

CFLAGS="-I$O_BINC -I$O_SINC -L$O_LIB" pytest -n auto tests --wycheproof-root=../wycheproof

cd ../
deactivate
rm -rf venv-cryptography