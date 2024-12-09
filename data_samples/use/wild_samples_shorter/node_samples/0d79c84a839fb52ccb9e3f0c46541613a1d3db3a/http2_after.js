  }
}

class AltSvcFrame extends Frame {
  constructor(size) {
    const buffers = [Buffer.alloc(size)];
    super(size, 10, 0, 0);
    buffers.unshift(this[kFrameData]);
    this[kFrameData] = Buffer.concat(buffers);
  }
}

module.exports = {
  Frame,
  AltSvcFrame,
  DataFrame,
  HeadersFrame,
  SettingsFrame,
  PingFrame,