  constructor(ack = false) {
    const buffers = [Buffer.alloc(8)];
    super(8, 6, ack ? 1 : 0, 0);
    buffers.unshift(this[kFrameData]);
    this[kFrameData] = Buffer.concat(buffers);
  }
}

class AltSvcFrame extends Frame {
  constructor(size) {
    const buffers = [Buffer.alloc(size)];
    super(size, 10, 0, 0);
    buffers.unshift(this[kFrameData]);
    this[kFrameData] = Buffer.concat(buffers);
  }