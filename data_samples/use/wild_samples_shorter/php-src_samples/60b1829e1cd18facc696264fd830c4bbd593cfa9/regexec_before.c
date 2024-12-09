      break;

    case OP_EXACT1:  MOP_IN(OP_EXACT1);
#if 0
      DATA_ENSURE(1);
      if (*p != *s) goto fail;
      p++; s++;
#endif
      if (*p != *s++) goto fail;
      DATA_ENSURE(0);
      p++;
      MOP_OUT;
      break;

    case OP_EXACT1_IC:  MOP_IN(OP_EXACT1_IC);