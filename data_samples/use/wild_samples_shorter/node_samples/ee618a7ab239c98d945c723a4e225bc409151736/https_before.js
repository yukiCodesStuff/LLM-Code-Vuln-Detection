  this.timeout = 2 * 60 * 1000;
  this.keepAliveTimeout = 5000;
  this.maxHeadersCount = null;
}
inherits(Server, tls.Server);

Server.prototype.setTimeout = HttpServer.prototype.setTimeout;