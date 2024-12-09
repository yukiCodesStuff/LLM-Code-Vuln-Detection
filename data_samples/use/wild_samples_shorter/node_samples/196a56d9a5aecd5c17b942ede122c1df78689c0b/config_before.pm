use warnings;
use Getopt::Std;
use File::Basename;
use IPC::Cmd;
use POSIX;
use Carp;

# These control our behavior.
my $DRYRUN;
my $VERSION;
my $CCVENDOR;
my $CCVER;
my $GCC_BITS;
my $GCC_ARCH;

# Some environment variables; they will affect Configure
my @cc_version =
    (
     clang => sub {
         my $v = `$CROSS_COMPILE$CC -v 2>&1`;
         $v =~ m/(?:(?:clang|LLVM) version|.*based on LLVM)\s+([0-9]+\.[0-9]+)/;
         return $1;
     },
     gnu => sub {
         my $v = `$CROSS_COMPILE$CC -dumpversion 2>/dev/null`;
         # Strip off whatever prefix egcs prepends the number with.
         # Hopefully, this will work for any future prefixes as well.
         $v =~ s/^[a-zA-Z]*\-//;
         return $v;
    [ 'CYGWIN.*',                   '${MACHINE}-pc-cygwin' ],
    [ 'vxworks.*',                  '${MACHINE}-whatever-vxworks' ],

    # Note: there's also NEO and NSR, but they are old and unsupported
    [ 'NONSTOP_KERNEL:.*:NSE-.*?',  'nse-tandem-nsk${RELEASE}' ],
    [ 'NONSTOP_KERNEL:.*:NSV-.*?',  'nsv-tandem-nsk${RELEASE}' ],
    [ 'NONSTOP_KERNEL:.*:NSX-.*?',  'nsx-tandem-nsk${RELEASE}' ],
                $CCVER = $v;
            }
        }
    }

    # If no C compiler has been determined at this point, we die.  Hard.
    die <<_____
            } else {
                $config{disable} = [ 'asm' ];
            }
            return %config;
        }
      ],

      # Windows values found by looking at Perl 5's win32/win32.c
      [ 'amd64-.*?-Windows NT',   { target => 'VC-WIN64A' } ],
      [ 'ia64-.*?-Windows NT',    { target => 'VC-WIN64I' } ],
      [ 'x86-.*?-Windows NT',     { target => 'VC-WIN32'  } ],

      # VMS values found by observation on existing machinery.
      # Unfortunately, the machine part is a bit...  overdone.  It seems,
      # though, that 'Alpha' exists in that part for Alphas, making it
      # distinguishable from Itanium.  It will be interesting to see what
      # we'll get in the upcoming x86_64 port...
      [ '.*Alpha.*?-.*?-OpenVMS', { target => 'vms-alpha' } ],
      [ '.*?-.*?-OpenVMS',        { target => 'vms-ia64'  } ],

      # TODO: There are a few more choices among OpenSSL config targets, but
      # reaching them involves a bit more than just a host tripet.  Select
      # environment variables could do the job to cover for more granular