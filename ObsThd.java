import java.util.concurrent.atomic.*;

public class ObsThd implements Runnable{
    private AtomicIntegerArray oldMem;
    private AtomicIntegerArray newMem;
    private int rounds;
    private AtomicIntegerArray[] Observations;
    private int threadID;
    private AtomicReference<AtomicIntegerArray> SM;
    //Constructor
    public ObsThd(AtomicReference<AtomicIntegerArray> sharedMem,
		  int givenRounds, int ID, AtomicIntegerArray[] Obs){
	SM = sharedMem;
	rounds = givenRounds;
	threadID = ID;
	Observations = Obs;
    }
    //Observer function
    //Co-OBS
    public void run(){
	//Run a given number of rounds
	CacheFlush<AtomicReference<AtomicIntegerArray>> cf =
	    new CacheFlush<AtomicReference<AtomicIntegerArray>>();
	for(int i = 0; i < rounds; i++){
	    //Init that we haven't finished round
	    boolean success = false;
	    //Continue while round is not complete
	    while(!success){
		//The JVM cannot reorder atomic reads and writes...
		//And they are always treated as volatile
		//Get the current AtomicIntegerArray
		//Each round gets the current snapshot of shared memory (oldMem)
		oldMem = SM.get();
		//Then reconstructs a new shared memory with
		//The proposed change.
		newMem = new AtomicIntegerArray(oldMem.length());
		//This makes sure the atomic reference changes
		//Whenever there is a successful CAS
		//And each array is read by many, written by 1.
		for(int j = 0; j < oldMem.length(); j++){
		    newMem.set(j, oldMem.get(j));
		}
		//Write the change to the memory atomically
		newMem.set(threadID, i);
		//We flush the reference from cache
		//So we have interleaving of random accesses
		//to the atomicReference SM
		//Flush Atomic Reference SM from cache...
		cf.cacheFlush(SM);
		//If the CAS succeeds, then our proposed change (newMem)
		//was current at some point in memory.
		//Update the atomic reference:
		success = SM.compareAndSet(oldMem, newMem);
		//If the CAS fails, then our snapshot (newMem)
		//was not current at some point in memory
		//And we need to try again.
		//Flush Atomic Reference SM from cache...
	        cf.cacheFlush(SM);
	    }
	    //We get here when CAS succeeds
	    //And newMem was a snapshot of the memory:
	    //read the other locations
	    Observations[i] = newMem;
	    //Observations[i] = SM.get();
	    //All observations will 
	}
    }
    
}
