      break;

    case OP_EXACT1:  MOP_IN(OP_EXACT1);
      DATA_ENSURE(1);
      if (*p != *s) goto fail;
      p++; s++;
      MOP_OUT;
      break;

    case OP_EXACT1_IC:  MOP_IN(OP_EXACT1_IC);