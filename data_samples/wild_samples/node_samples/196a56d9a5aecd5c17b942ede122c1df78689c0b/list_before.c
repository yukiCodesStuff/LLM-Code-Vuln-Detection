const OPTIONS list_options[] = {

    OPT_SECTION("General"),
    {"help", OPT_HELP, '-', "Display this summary"},

    OPT_SECTION("Output"),
    {"1", OPT_ONE, '-', "List in one column"},
    {"verbose", OPT_VERBOSE, '-', "Verbose listing"},
    {"select", OPT_SELECT_NAME, 's', "Select a single algorithm"},
    {"commands", OPT_COMMANDS, '-', "List of standard commands"},
    {"standard-commands", OPT_COMMANDS, '-', "List of standard commands"},
#ifndef OPENSSL_NO_DEPRECATED_3_0
    {"digest-commands", OPT_DIGEST_COMMANDS, '-',
     "List of message digest commands (deprecated)"},
#endif
    {"digest-algorithms", OPT_DIGEST_ALGORITHMS, '-',
     "List of message digest algorithms"},
    {"kdf-algorithms", OPT_KDF_ALGORITHMS, '-',
     "List of key derivation and pseudo random function algorithms"},
    {"random-instances", OPT_RANDOM_INSTANCES, '-',
     "List the primary, public and private random number generator details"},
    {"random-generators", OPT_RANDOM_GENERATORS, '-',
     "List of random number generators"},
    {"mac-algorithms", OPT_MAC_ALGORITHMS, '-',
     "List of message authentication code algorithms"},
#ifndef OPENSSL_NO_DEPRECATED_3_0
    {"cipher-commands", OPT_CIPHER_COMMANDS, '-', 
    "List of cipher commands (deprecated)"},
#endif
    {"cipher-algorithms", OPT_CIPHER_ALGORITHMS, '-',
     "List of cipher algorithms"},
    {"encoders", OPT_ENCODERS, '-', "List of encoding methods" },
    {"decoders", OPT_DECODERS, '-', "List of decoding methods" },
    {"key-managers", OPT_KEYMANAGERS, '-', "List of key managers" },
    {"key-exchange-algorithms", OPT_KEYEXCHANGE_ALGORITHMS, '-',
     "List of key exchange algorithms" },
    {"kem-algorithms", OPT_KEM_ALGORITHMS, '-',
     "List of key encapsulation mechanism algorithms" },
    {"signature-algorithms", OPT_SIGNATURE_ALGORITHMS, '-',
     "List of signature algorithms" },
    {"asymcipher-algorithms", OPT_ASYM_CIPHER_ALGORITHMS, '-',
      "List of asymmetric cipher algorithms" },
    {"public-key-algorithms", OPT_PK_ALGORITHMS, '-',
     "List of public key algorithms"},
    {"public-key-methods", OPT_PK_METHOD, '-',
     "List of public key methods"},
    {"store-loaders", OPT_STORE_LOADERS, '-',
     "List of store loaders"},
    {"providers", OPT_PROVIDER_INFO, '-',
     "List of provider information"},
#ifndef OPENSSL_NO_DEPRECATED_3_0
    {"engines", OPT_ENGINES, '-',
     "List of loaded engines"},
#endif
    {"disabled", OPT_DISABLED, '-', "List of disabled features"},
    {"options", OPT_OPTIONS, 's',
     "List options for specified command"},
    {"objects", OPT_OBJECTS, '-',
     "List built in objects (OID<->name mappings)"},

    OPT_PROV_OPTIONS,
    {NULL}
};

int list_main(int argc, char **argv)
{
    char *prog;
    HELPLIST_CHOICE o;
    int one = 0, done = 0;
    struct {
        unsigned int commands:1;
        unsigned int random_instances:1;
        unsigned int random_generators:1;
        unsigned int digest_commands:1;
        unsigned int digest_algorithms:1;
        unsigned int kdf_algorithms:1;
        unsigned int mac_algorithms:1;
        unsigned int cipher_commands:1;
        unsigned int cipher_algorithms:1;
        unsigned int encoder_algorithms:1;
        unsigned int decoder_algorithms:1;
        unsigned int keymanager_algorithms:1;
        unsigned int signature_algorithms:1;
        unsigned int keyexchange_algorithms:1;
        unsigned int kem_algorithms:1;
        unsigned int asym_cipher_algorithms:1;
        unsigned int pk_algorithms:1;
        unsigned int pk_method:1;
        unsigned int store_loaders:1;
        unsigned int provider_info:1;
#ifndef OPENSSL_NO_DEPRECATED_3_0
        unsigned int engines:1;
#endif
        unsigned int disabled:1;
        unsigned int objects:1;
        unsigned int options:1;
    } todo = { 0, };

    verbose = 0;                 /* Clear a possible previous call */

    prog = opt_init(argc, argv, list_options);
    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_EOF:  /* Never hit, but suppresses warning */
        case OPT_ERR:
opthelp:
            BIO_printf(bio_err, "%s: Use -help for summary.\n", prog);
            return 1;
        case OPT_HELP:
            opt_help(list_options);
            return 0;
        case OPT_ONE:
            one = 1;
            break;
        case OPT_COMMANDS:
            todo.commands = 1;
            break;
        case OPT_DIGEST_COMMANDS:
            todo.digest_commands = 1;
            break;
        case OPT_DIGEST_ALGORITHMS:
            todo.digest_algorithms = 1;
            break;
        case OPT_KDF_ALGORITHMS:
            todo.kdf_algorithms = 1;
            break;
        case OPT_RANDOM_INSTANCES:
            todo.random_instances = 1;
            break;
        case OPT_RANDOM_GENERATORS:
            todo.random_generators = 1;
            break;
        case OPT_MAC_ALGORITHMS:
            todo.mac_algorithms = 1;
            break;
        case OPT_CIPHER_COMMANDS:
            todo.cipher_commands = 1;
            break;
        case OPT_CIPHER_ALGORITHMS:
            todo.cipher_algorithms = 1;
            break;
        case OPT_ENCODERS:
            todo.encoder_algorithms = 1;
            break;
        case OPT_DECODERS:
            todo.decoder_algorithms = 1;
            break;
        case OPT_KEYMANAGERS:
            todo.keymanager_algorithms = 1;
            break;
        case OPT_SIGNATURE_ALGORITHMS:
            todo.signature_algorithms = 1;
            break;
        case OPT_KEYEXCHANGE_ALGORITHMS:
            todo.keyexchange_algorithms = 1;
            break;
        case OPT_KEM_ALGORITHMS:
            todo.kem_algorithms = 1;
            break;
        case OPT_ASYM_CIPHER_ALGORITHMS:
            todo.asym_cipher_algorithms = 1;
            break;
        case OPT_PK_ALGORITHMS:
            todo.pk_algorithms = 1;
            break;
        case OPT_PK_METHOD:
            todo.pk_method = 1;
            break;
        case OPT_STORE_LOADERS:
            todo.store_loaders = 1;
            break;
        case OPT_PROVIDER_INFO:
            todo.provider_info = 1;
            break;
#ifndef OPENSSL_NO_DEPRECATED_3_0
        case OPT_ENGINES:
            todo.engines = 1;
            break;
#endif
        case OPT_DISABLED:
            todo.disabled = 1;
            break;
        case OPT_OBJECTS:
            todo.objects = 1;
            break;
        case OPT_OPTIONS:
            list_options_for_command(opt_arg());
            break;
        case OPT_VERBOSE:
            verbose = 1;
            break;
        case OPT_SELECT_NAME:
            select_name = opt_arg();
            break;
        case OPT_PROV_CASES:
            if (!opt_provider(o))
                return 1;
            break;
        }
        done = 1;
    }

    /* No extra arguments. */
    if (opt_num_rest() != 0)
        goto opthelp;

    if (todo.commands)
        list_type(FT_general, one);
    if (todo.random_instances)
        list_random_instances();
    if (todo.random_generators)
        list_random_generators();
    if (todo.digest_commands)
        list_type(FT_md, one);
    if (todo.digest_algorithms)
        list_digests();
    if (todo.kdf_algorithms)
        list_kdfs();
    if (todo.mac_algorithms)
        list_macs();
    if (todo.cipher_commands)
        list_type(FT_cipher, one);
    if (todo.cipher_algorithms)
        list_ciphers();
    if (todo.encoder_algorithms)
        list_encoders();
    if (todo.decoder_algorithms)
        list_decoders();
    if (todo.keymanager_algorithms)
        list_keymanagers();
    if (todo.signature_algorithms)
        list_signatures();
    if (todo.asym_cipher_algorithms)
        list_asymciphers();
    if (todo.keyexchange_algorithms)
        list_keyexchanges();
    if (todo.kem_algorithms)
        list_kems();
    if (todo.pk_algorithms)
        list_pkey();
    if (todo.pk_method)
        list_pkey_meth();
    if (todo.store_loaders)
        list_store_loaders();
    if (todo.provider_info)
        list_provider_info();
#ifndef OPENSSL_NO_DEPRECATED_3_0
    if (todo.engines)
        list_engines();
#endif
    if (todo.disabled)
        list_disabled();
    if (todo.objects)
        list_objects();

    if (!done)
        goto opthelp;

    return 0;
}