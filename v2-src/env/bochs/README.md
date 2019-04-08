# bochs instructions

## install

I am OSX envrionment, so just type command blow

```
brew install bochs
```

Then the bochs installed. and besides, source code installment also available.

And there's some issue about configuration file.

First issue it's path. you can see some of them in configuration file, there are different if the way you install were different.

Second, bios tools and keyboadr in conf file. if you can't up bochs correcetly, check the bios and keyboadr configuration path and name.

## start

To start bochs, configuration file must be specified.

The bochsrc is the configuration file so that you can use command blow to start bochs

```
bochs -f ./bochsrc(The file's path)
```
