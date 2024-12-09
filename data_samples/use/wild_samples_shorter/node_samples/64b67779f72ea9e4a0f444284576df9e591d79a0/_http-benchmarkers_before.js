
class AutocannonBenchmarker {
  constructor() {
    this.name = 'autocannon';
    this.executable =
      process.platform === 'win32' ? 'autocannon.cmd' : 'autocannon';
    const result = child_process.spawnSync(this.executable, ['-h']);
    this.present = !(result.error && result.error.code === 'ENOENT');
  }

  create(options) {
    const args = [
      '-n',
    ];
    for (const field in options.headers) {
      args.push('-H', `${field}=${options.headers[field]}`);
    }
    const scheme = options.scheme || 'http';
    args.push(`${scheme}://127.0.0.1:${options.port}${options.path}`);
    const child = child_process.spawn(this.executable, args);
    return child;
  }

  processResults(output) {