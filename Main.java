import java.util.concurrent.atomic.*;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class Main{
    public static void main(String[] args){
	int numRounds = 100 ;
	int obsCount = 2;
        boolean printObs = true;
	if(args.length != 1 && args.length != 2 && args.length != 3){
	    System.out.println("No parameters specified...");
	    System.out.println("Using default value of 100");
	    System.out.println("Using default print flag: TRUE");
	    System.out.println("Usage: java Main [numRounds]");
	    System.out.println("Or: java Main [numRounds] [ObserverCount]");
	    System.out.println("Or: java Main [numRounds] [ObserverCount] [printFlag T/F]");
	}
	else{
	    try{
		numRounds = Integer.parseInt(args[0]);
	    }
	    catch(Exception e){
		System.out.println("Error: Unable to parse rounds..");
		System.out.println("Using Default value of 100");
		System.out.println("Usage: java Main [numRounds]");
	        System.out.println("Or: java Main [numRounds] [ObserverCount]");
		System.out.println("Or: java Main [numRounds] [ObserverCount] [printFlag T/F]");
		numRounds = 100;
	    }
	    if(numRounds <= 0){
		System.out.println("Error: Rounds must be a positive integer.");
		System.out.println("Using Default value of 100");
		System.out.println("Usage: java Main [numRounds]");
	        System.out.println("Or: java Main [numRounds] [ObserverCount]");
		System.out.println("Or: java Main [numRounds] [ObserverCount] [printFlag T/F]");
		numRounds = 100;
	    }
	    if(args.length > 1){
		try{
		    obsCount = Integer.parseInt(args[1]);
		}
		catch(Exception e){
		    System.out.println("Error: Unable to parse Observer Count");
		    System.out.println("Using Default value of 2");
		    System.out.println("Or: java Main [numRounds] [ObserverCount]");
		    System.out.println("Or: java Main [numRounds] [ObserverCount] [printFlag T/F]");
		    obsCount = 2;
		}
		if(obsCount <2 || obsCount > 255){
		    System.out.println("Error: Invalid number of observers. Must be in [2,255]");
		    System.out.println("Using Default value of 2");
		    System.out.println("Or: java Main [numRounds] [ObserverCount]");
		    System.out.println("Or: java Main [numRounds] [ObserverCount] [printFlag T/F]");
		    obsCount = 2;
		}
	    }
	    if(args.length == 3){
		if(args[2].equals("F")){
		    printObs = false;
		}
		else{
		    printObs = true;
		}
	    }
	}

	//Create the shared Atomic Integers
	AtomicIntegerArray sharedMem = new AtomicIntegerArray(obsCount);
	//Create obsCount arrays 
	AtomicIntegerArray[][] thdObs = new AtomicIntegerArray[obsCount][numRounds];
	for(int i = 0; i < obsCount; i++){
	    //Initialize the variables to  -1:
	    sharedMem.set(i, -1);
	}
	AtomicReference<AtomicIntegerArray> SM = new
	    AtomicReference<AtomicIntegerArray>(sharedMem);
	//Create the observation arrays:
	
	//Create two thread objects
	Thread[] observers = new Thread[obsCount];
	//Run the observer threads
	for(int ID = 0; ID < obsCount; ID++){
	    observers[ID] = new Thread(new ObsThd(SM, numRounds, ID,
						  thdObs[ID]));
	    observers[ID].start();
	}
	//Look pretty
	//Wait for the threads to finish (join)
	for(int ID=0; ID < obsCount; ID++){
	    try{
		observers[ID].join();
	    }
	    catch(Exception e){
		System.out.println("Error: Unable to join thread. "+
				   e.getMessage());
	    }
	}
	if(printObs){
	    //Print the observations!
	    for(int ID = 0; ID < obsCount; ID++){
		System.out.println("ID: " + ID + " Observations");
		for(int i = 0; i < numRounds; i++){
		    System.out.print("[");
		    for(int j = 0; j < obsCount; j++){
			//if(j != ID){
			    System.out.print(thdObs[ID][i].get(j));
			    //}
			    if(j != obsCount-1)
				System.out.print(" ");
		    }
		    System.out.print("], ");
		}
		System.out.println("");
	    }
	}
	/*
	  Reconstruct the Trace:
	  TODO: Parallelize the reconstruction...
	 */
	//The max length is observers*rounds
	int maxTraceLength = obsCount*numRounds;
	//Make RecInfo obj to share resources between threads...
	RecInfo infoObj = new RecInfo(maxTraceLength, obsCount, numRounds, thdObs);
	//Note: in algorithm N is numRounds
	int[] trace = new int[maxTraceLength];
	Thread recThread = new Thread(new multiRecThd(trace, infoObj, obsCount));
	recThread.start();
	try{
	    recThread.join();
	}
	catch(Exception e){
	    System.out.println("Error: Unable to join thread!");
	}
	//Trace is full and reconstructed
	//Trace should be length maxTraceLength
	if(printObs){
	    System.out.println("Reconstructed Trace: ");
	    for(int i = 0; i < maxTraceLength; i++){
		System.out.print("W" + (trace[i]-1) + "R" + (trace[i]-1)
				 + ", ");
	    }
	    System.out.println("");
	}
	byte[] byteTrace = new byte[maxTraceLength];
	for(int i = 0; i < maxTraceLength; i++){
	    byteTrace[i] = (byte)(trace[i]-1);
	}
	File tracef = new File("trace.bin");
	try{
	    FileOutputStream fos = new FileOutputStream(tracef);
	    fos.write(byteTrace);
	    fos.close();
	}
	catch(IOException e){
	    System.out.println("Error: unable to open file.");
	    System.exit(0);
	}
    }
}
