#define FOLL_MIGRATION	0x400	/* wait for page to replace migration entry */
#define FOLL_TRIED	0x800	/* a retry, previous pass started an IO */
#define FOLL_REMOTE	0x2000	/* we are working on non-current tsk/mm */
#define FOLL_ANON	0x8000	/* don't do file mappings */
#define FOLL_LONGTERM	0x10000	/* mapping lifetime is indefinite: see below */
#define FOLL_SPLIT_PMD	0x20000	/* split huge pmd before returning */
#define FOLL_PIN	0x40000	/* pages must be released via unpin_user_page */