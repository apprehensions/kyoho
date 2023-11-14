# kyoho
Bare bones C+POSIX Roblox Player bootstrapper

```
make
./kyoho
```

`kyoho` uses an external C program `unarp` (`kyoho` assumes it is in PWD)
to extract packages, as Roblox has specific file paths for their zipped
packages, and cannot be extracted with `unzip`.
