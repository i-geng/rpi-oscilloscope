# RPI Oscilloscope

## Setup

### Setup pi-install
Go to `/bin`, copy the `pi-install.macos` or `pi-install.linux`(depends on your OS) and rename it to `pi-install`.

### Add CS140E_FINAL_PROJ_PATH to your environment
In your `.zshrc` or `.bashrc`, add the following:
```
export CS140E_FINAL_PROJ_PATH=<path to this repo>
```

## Git

### Before you commit
Run `make clean` in the root directory to clean all the files.

## Makefile

When you create a new folder in `src`, you need to add it to the Makefile.

In the `src/Makefile`, add the following:
```
clean:
    ...existing clean rules...
	make clean -C <name of the new folder>
```

Also, create a Makefile in the new folder with clean rules.




