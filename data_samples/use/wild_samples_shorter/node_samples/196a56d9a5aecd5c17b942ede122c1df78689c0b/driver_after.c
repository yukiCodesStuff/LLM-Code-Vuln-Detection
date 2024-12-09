/*
 * Copyright 2016-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
            test_flush_tapout();
        } else if (all_tests[i].num == -1) {
            set_test_title(all_tests[i].test_case_name);
            ERR_clear_error();
            verdict = all_tests[i].test_fn();
            finalize(verdict != 0);
            test_verdict(verdict, "%d - %s", test_case_count + 1, test_title);
            if (verdict == 0)
                num_failed++;
            test_case_count++;
        } else {
            verdict = TEST_SKIP_CODE;
            set_test_title(all_tests[i].test_case_name);
            if (all_tests[i].subtest) {
                level += 4;
                j = (j + jstep) % all_tests[i].num;
                if (single_iter != -1 && ((jj + 1) != single_iter))
                    continue;
                ERR_clear_error();
                v = all_tests[i].param_test_fn(j);

                if (v == 0) {
                    verdict = 0;
                } else if (v != TEST_SKIP_CODE && verdict != 0) {
                    verdict = 1;
                }