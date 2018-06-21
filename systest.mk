systest: systest.c
	$(CC) -o systest $(CFLAGS) $(CPPFLAGS) systest.c
clean:
	rm -f systest systest.exe
