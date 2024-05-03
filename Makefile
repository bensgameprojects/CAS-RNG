all: casrng
	echo "Run with: ./casrng"
	echo "Run with: java -Djava.library.path=. Main [numRounds] [numObservers] [printFlag T/F]"
	echo "Run with: ./blumelias [numrounds] [numObservers] [NBINSTANCES]"
casrng: blumelias Main.class
	gcc Driver.c -o casrng
Main.class: ObsThd.class RecInfo.class multiRecThd.class libCacheFlush.so
	#Run with: java -Djava.library.path=. Main [numRounds] [numObservers] [printFlag T/F]
	javac Main.java
ObsThd.class:
	javac ObsThd.java
RecInfo.class:
	javac RecInfo.java
multiRecThd.class:
	javac multiRecThd.java
libCacheFlush.so: CacheFlush.o
	#Create the shared object file...
	gcc -shared -fPIC -o libCacheFlush.so CacheFlush.o -lc
CacheFlush.o: CacheFlush.h CacheFlush.class
	#Compile the C code
	gcc -c -fPIC -I/usr/lib/jvm/java-11-openjdk-amd64/include -I/usr/lib/jvm/java-11-openjdk-amd64/include/linux -I/usr/src/linux-headers-4.18.0-21/arch/sh/include/uapi/ CacheFlush.c -o CacheFlush.o
CacheFlush.h: CacheFlush.class
	#Create the c header
	javac -h . CacheFlush.java
CacheFlush.class:
	#Compile the Java class
	javac CacheFlush.java
blumelias: blumEliasDriver.o blumelias.o 
	#Link object files together
	gcc blumelias.o blumEliasDriver.o -o blumelias -lgmp -lgmpxx
blumelias.o:
	#Make blum elias file
	gcc -c blumelias.c -l blumelias.h
blumEliasDriver.o:
	#Make corng object file
	gcc -c blumEliasDriver.c

clean:
	#Removing all class files and shared objects...
	rm -f *.class *.so *.o CacheFlush.h blumelias
