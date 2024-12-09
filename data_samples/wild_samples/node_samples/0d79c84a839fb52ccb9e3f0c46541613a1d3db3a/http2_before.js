  constructor(ack = false) {
    const buffers = [Buffer.alloc(8)];
    super(8, 6, ack ? 1 : 0, 0);
    buffers.unshift(this[kFrameData]);
    this[kFrameData] = Buffer.concat(buffers);
  }
}

module.exports = {
  Frame,
  DataFrame,
  HeadersFrame,
  SettingsFrame,
  PingFrame,
  kFakeRequestHeaders,
  kFakeResponseHeaders,
  kClientMagic
};