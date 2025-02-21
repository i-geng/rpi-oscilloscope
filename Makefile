# When make clean, it will call make clean in all subdirectories
clean:
	make clean -C src
	make clean -C libpi
	make clean -C libunix