
class AutocannonBenchmarker {
  constructor() {
    const shell = (process.platform === 'win32');
    this.name = 'autocannon';
    this.opts = { shell };
    this.executable = shell ? 'autocannon.cmd' : 'autocannon';
    const result = child_process.spawnSync(this.executable, ['-h'], this.opts);
    if (shell) {
      this.present = (result.status === 0);
    } else {
      this.present = !(result.error && result.error.code === 'ENOENT');
    }
  }

  create(options) {
    const args = [
      '-n',
    ];
    for (const field in options.headers) {
      if (this.opts.shell) {
        args.push('-H', `'${field}=${options.headers[field]}'`);
      } else {
        args.push('-H', `${field}=${options.headers[field]}`);
      }
    }
    const scheme = options.scheme || 'http';
    args.push(`${scheme}://127.0.0.1:${options.port}${options.path}`);
    const child = child_process.spawn(this.executable, args, this.opts);
    return child;
  }

  processResults(output) {