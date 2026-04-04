# Deps
* Linux
* Docker
* GNU Make
* Python 3.x

# Preparing image
```console
$ sudo make build-image
```

# Local workspace files

After cloning the repository, configure local workspace files that won't be 
pushed to the remote:

```console
$ git update-index --skip-worktree ai-doc/branch.md
$ git update-index --skip-worktree ai-doc/current.md
```

These files are meant for local development notes and won't affect the 
repository.

# Copying 
License - [Zlib](https://github.com/edomin/steroids/blob/master/COPYING)
