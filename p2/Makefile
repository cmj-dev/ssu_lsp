CC = gcc

ssu_sdup : ssu_sdup.c fmd5 fsha1 help
	$(CC) -o ssu_sdup ssu_sdup.c

fmd5 : ssu_find-md5.c ssu_find.c ssu_find.h
	$(CC) -o fmd5 ssu_find-md5.c ssu_find.c ssu_find.h -lcrypto

fsha1 : ssu_find-sha1.c ssu_find.c ssu_find.h
	$(CC) -o fsha1 ssu_find-sha1.c ssu_find.c ssu_find.h -lcrypto

help : ssu_help.c
	$(CC) -o help ssu_help.c

remove : 
	rm ssu_sdup fmd5 fsha1 help