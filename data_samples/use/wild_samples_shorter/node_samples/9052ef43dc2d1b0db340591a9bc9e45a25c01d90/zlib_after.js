  self.bytesWritten += inDelta;

  const have = handle.availOutBefore - availOutAfter;
  let streamBufferIsFull = false;
  if (have > 0) {
    const out = self._outBuffer.slice(self._outOffset, self._outOffset + have);
    self._outOffset += have;
    streamBufferIsFull = !self.push(out);
  } else {
    assert(have === 0, 'have should not go down');
  }

    handle.inOff += inDelta;
    handle.availInBefore = availInAfter;


    if (!streamBufferIsFull) {
      this.write(handle.flushFlag,
                 this.buffer, // in
                 handle.inOff, // in_off
                 handle.availInBefore, // in_len
                 self._outBuffer, // out
                 self._outOffset, // out_off
                 self._chunkSize); // out_len
    } else {
      const oldRead = self._read;
      self._read = (n) => {
        self._read = oldRead;
        this.write(handle.flushFlag,
                   this.buffer, // in
                   handle.inOff, // in_off
                   handle.availInBefore, // in_len
                   self._outBuffer, // out
                   self._outOffset, // out_off
                   self._chunkSize); // out_len
        self._read(n);
      };
    }
    return;
  }

  if (availInAfter > 0) {