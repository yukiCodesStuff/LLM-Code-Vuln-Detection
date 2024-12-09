
sub ::endbranch
{
    &::generic("%ifdef __CET__\n");
    &::data_byte(0xf3,0x0f,0x1e,0xfb);
    &::generic("%endif\n");
}

# label management
$lbdecor="L";		# local label decoration, set by package