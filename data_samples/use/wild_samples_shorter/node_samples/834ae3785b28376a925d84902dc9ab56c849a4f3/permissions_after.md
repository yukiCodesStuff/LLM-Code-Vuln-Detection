* `--allow-fs-read=/home/test*` will allow read access to everything
  that matches the wildcard. e.g: `/home/test/file1` or `/home/test2`

After passing a wildcard character (`*`) all subsequent characters will
be ignored. For example: `/home/*.js` will work similar to `/home/*`.

#### Permission Model constraints

There are constraints you need to know before using this system:
