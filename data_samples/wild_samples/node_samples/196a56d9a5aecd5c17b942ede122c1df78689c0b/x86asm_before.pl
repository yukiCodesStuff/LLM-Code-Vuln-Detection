	push @opcode,$c=~/^0/?oct($c):$c;
	&::data_byte(@opcode);
    }
    else
    {	&::generic("vprotd",@_);	}
}

sub ::endbranch
{
    &::generic("%ifdef __CET__\n");
    &::data_byte(0xf3,0x0f,0x1e,0xfb);
    &::generic("%endif\n");
}