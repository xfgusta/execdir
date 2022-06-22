# execdir

Execute a command in a specific directory

## Examples

+ Run a command in a specific directory:

```text
$ execdir ~/Fedora/SCM/nq git status
On branch rawhide
Your branch is up to date with 'origin/rawhide'.

nothing to commit, working tree clean
```

+ Run a shell command in a specific directory:

```text
$ execdir -s ~/Desktop echo \$PWD
/home/xfgusta/Desktop
```

There's also a symlink called `xdir`, so you can type less using it.

## Installation

### From source

The install directory defaults to `/usr/local`:

```text
$ make install
gcc -Wall -Wextra -Werror -pedantic -std=c11 -O2 execdir.c  -o execdir
mkdir -p /usr/local/bin
install -p -m 0755 execdir /usr/local/bin
ln -s /usr/local/bin/execdir /usr/local/bin/xdir
```

You can install `execdir` in a different directory using the `PREFIX` variable:

```text
$ make PREFIX=/usr install
gcc -Wall -Wextra -Werror -pedantic -std=c11 -O2 execdir.c  -o execdir
mkdir -p /usr/bin
install -p -m 0755 execdir /usr/bin
ln -s /usr/bin/execdir /usr/bin/xdir
```

## License

Copyright (c) 2022 Gustavo Costa. Distributed under the MIT license.
