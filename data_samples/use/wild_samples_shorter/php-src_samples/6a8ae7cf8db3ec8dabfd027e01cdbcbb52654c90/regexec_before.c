    }
    else {
      UChar *q = p + reg->dmin;
      while (p < q) p += enclen(reg->enc, p);
    }
  }
