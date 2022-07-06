# execdir

A tool that lets you run a command in a specific directory. It supports shell commands and path aliases. **execdir** will try to get an alias if the path doesn't exist.

## Examples

Run a command in a specific directory:

```text
$ execdir ~/Fedora/SCM/nq git status
On branch rawhide
Your branch is up to date with 'origin/rawhide'.

nothing to commit, working tree clean
```

Run a shell command in a specific directory:

```text
$ execdir -s ~/Desktop echo \$PWD
/home/xfgusta/Desktop
```

Create an alias for a path:

```text
$ execdir -a nq ~/Fedora/SCM/nq
```

Use an alias:

```text
$ execdir nq pwd
/home/xfgusta/Fedora/SCM/nq
```

List all aliases:

```text
$ execdir -l
nq    /home/xfgusta/Fedora/SCM/nq
```

Delete an alias:

```text
$ execdir -r nq
```

There's also a symlink called **x**, so you can type less using it.

## Installation

### Arch Linux

[**execdir**](https://aur.archlinux.org/packages/execdir) package from AUR

```text
git clone https://aur.archlinux.org/execdir.git
cd execdir
makepkg -si
```

### Fedora Linux

[**execdir**](https://copr.fedorainfracloud.org/coprs/xfgusta/execdir/) package from Copr

```text
dnf copr enable xfgusta/execdir
dnf install execdir
```

### From source

The install directory defaults to `/usr/local`:

```text
make install
```

You can install **execdir** in a different directory using the `PREFIX` variable:

```text
make PREFIX=/usr install
```

## License

Copyright (c) 2022 Gustavo Costa. Distributed under the MIT license.
