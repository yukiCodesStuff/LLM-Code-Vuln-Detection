#! /usr/bin/env perl
# Copyright 2015-2022 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
    ZERO_LEN_KEX_DATA => 9,
    TRAILING_DATA => 10,
    SELECT_X25519 => 11,
    NO_KEY_SHARES_IN_HRR => 12,
    NON_TLS1_3_KEY_SHARE => 13
};

use constant {
    CLIENT_TO_SERVER => 1,
    $proxy->serverflags("-groups P-256");
}
$proxy->start() or plan skip_all => "Unable to start up Proxy for tests";
plan tests => 23;
ok(TLSProxy::Message->success(), "Success after HRR");

#Test 2: The server sending an HRR requesting a group the client already sent
#        should fail
$proxy->start();
ok(TLSProxy::Message->fail(), "Server sends HRR with no key_shares");

SKIP: {
    skip "No EC support in this OpenSSL build", 1 if disabled("ec");
    #Test 23: Trailing data on key_share in ServerHello should fail
    $proxy->clear();
    $direction = CLIENT_TO_SERVER;
    $proxy->clientflags("-groups secp192r1:P-256:X25519");
    $proxy->ciphers("AES128-SHA:\@SECLEVEL=0");
    $testtype = NON_TLS1_3_KEY_SHARE;
    $proxy->start();
    my $ishrr = defined ${$proxy->message_list}[2]
                &&(${$proxy->message_list}[0]->mt == TLSProxy::Message::MT_CLIENT_HELLO)
                && (${$proxy->message_list}[2]->mt == TLSProxy::Message::MT_CLIENT_HELLO);
    ok(TLSProxy::Message->success() && $ishrr,
       "Client sends a key_share for a Non TLSv1.3 group");
}

sub modify_key_shares_filter
{
    my $proxy = shift;

    # We're only interested in the initial ClientHello/SererHello/HRR
    if (($direction == CLIENT_TO_SERVER && $proxy->flight != 0
                && ($proxy->flight != 1 || $testtype != NO_KEY_SHARES_IN_HRR))
            || ($direction == SERVER_TO_CLIENT && $proxy->flight != 1)) {
        return;
            my $ext;
            my $suppgroups;

            if ($testtype != NON_TLS1_3_KEY_SHARE) {
                #Setup supported groups to include some unrecognised groups
                $suppgroups = pack "C8",
                    0x00, 0x06, #List Length
                    0xff, 0xfe, #Non existing group 1
                    0xff, 0xff, #Non existing group 2
                    0x00, 0x1d; #x25519
            } else {
                $suppgroups = pack "C6",
                    0x00, 0x04, #List Length
                    0x00, 0x13,
                    0x00, 0x1d; #x25519
            }

            if ($testtype == EMPTY_EXTENSION) {
                $ext = pack "C2",
                    0x00, 0x00;
                    0x00, 0x17, #P-256
                    0x00, 0x01, #key_exchange data length
                    0xff;       #Dummy key_share data
            } elsif ($testtype == NON_TLS1_3_KEY_SHARE) {
                $ext = pack "C6H98",
                    0x00, 0x35, #List Length
                    0x00, 0x13, #P-192
                    0x00, 0x31, #key_exchange data length
                    "04EE3B38D1CB800A1A2B702FC8423599F2AC7161E175C865F8".
                    "3DAF78BCBAE561464E8144359BE70CB7989D28A2F43F8F2C";  #key_exchange data
            }

            if ($testtype != EMPTY_EXTENSION
                    && $testtype != NO_KEY_SHARES_IN_HRR) {
                $message->set_extension(
                    TLSProxy::Message::EXT_SUPPORTED_GROUPS, $suppgroups);
            }
            if ($testtype == MISSING_EXTENSION) {
                $message->delete_extension(
                    TLSProxy::Message::EXT_KEY_SHARE);
            } elsif ($testtype != NOT_IN_SUPPORTED_GROUPS) {