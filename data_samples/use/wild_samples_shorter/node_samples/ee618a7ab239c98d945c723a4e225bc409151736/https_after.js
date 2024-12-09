  this.timeout = 2 * 60 * 1000;
  this.keepAliveTimeout = 5000;
  this.maxHeadersCount = null;
  this.headersTimeout = 40 * 1000; // 40 seconds
}
inherits(Server, tls.Server);

Server.prototype.setTimeout = HttpServer.prototype.setTimeout;