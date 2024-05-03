import java.util.concurrent.atomic.*;
public class RecThd implements Runnable{
    private int ID0;
    private int ID1;
    private int[] trace;
    private RecInfo infoObj;
    private int[] n;
    public RecThd(int id0, int id1, int[] t, RecInfo info){
	ID0 = id0;
	ID1 = id1;
	trace = t;
	infoObj = info;
	n = new int[2];
	for(int i = 0; i < 2; i++){
	    n[i] = 0;
	}
    }
    //REC alg for 2 threads
    public void run(){
	int count = 0;
	//While we still have observations in both arrays:
	while(n[ID0%2] < infoObj.numRounds && n[ID1%2] < infoObj.numRounds){
	    //if thread ID0's n[ID0]'th observation is -1
	    if(infoObj.thdObs[ID0][n[ID0%2]].get(ID1) == -1){
		//then thread ID0 wrote first
		//so we have [wID0 rID0] [wID1]
		trace[2*(n[ID0%2] + n[ID1%2])] = ID0+1;
		trace[2*(n[ID0%2] + n[ID1%2])+1] = -ID0-1;
		//observation processed so increment.
		n[ID0%2] = n[ID0%2] + 1;
	    }
	    else if(infoObj.thdObs[ID1][n[ID1%2]].get(ID0) == -1){
		//then thread ID1 wrote first
		//so we have [wID1 rID1] [wID0]
		trace[2*(n[ID0%2] + n[ID1%2])] = ID1+1;
		trace[2*(n[ID0%2] + n[ID1%2])+1] = -ID1-1;
		//observation processed so increment.
		n[ID1%2] = n[ID1%2] + 1;
	    }
	    //if thread ID0's n[ID0]th thdObservation is less than n[ID1],
	    //we came after so put it in the order now
	    else if(infoObj.thdObs[ID0][n[ID0%2]].get(ID1) < n[ID1%2]){
		trace[2*(n[ID0%2] + n[ID1%2])] = ID0+1;
		trace[2*(n[ID0%2] + n[ID1%2])+1] = -ID0-1;
		//observation processed so increment.
		n[ID0%2] = n[ID0%2] + 1;
	    }
	    else if(infoObj.thdObs[ID1][n[ID1%2]].get(ID0) < n[ID0%2]){
		trace[2*(n[ID0%2] + n[ID1%2])] = ID1+1;
		trace[2*(n[ID0%2] + n[ID1%2])+1] = -ID1-1;
		//observation processed so increment.
		n[ID1%2] = n[ID1%2] + 1;
	    }
	    else{
		//the writes happened at the same time
		//W0W1
		trace[2*(n[ID0%2] + n[ID1%2])] = ID0+1;
		trace[2*(n[ID0%2] + n[ID1%2])+1] = ID1+1;
		count++;
		while(infoObj.thdObs[ID1][n[ID1%2]].get(ID0) > n[ID0%2] || infoObj.thdObs[ID0][n[ID0%2]].get(ID1) > n[ID1%2]){
		    if(infoObj.thdObs[ID1][n[ID1%2]].get(ID0) > n[ID0%2]){
			//R0W0
			trace[2*(n[ID0%2] + n[ID1%2]+1)] = -ID0-1;
			trace[2*(n[ID0%2] + n[ID1%2]+1)+1] = ID0+1;
			//observation processed so increment.
			n[ID0%2] = n[ID0%2] + 1;
		    }
		    else{
			//R1W1
			trace[2*(n[ID0%2] + n[ID1%2]+1)] = -ID1-1;
			trace[2*(n[ID0%2] + n[ID1%2]+1)+1] = ID1+1;
			//observation processed so increment.
			n[ID1%2] = n[ID1%2] + 1;
		    }
		}
		//The reads happened at the same time
		//R0R1
		trace[2*(n[ID0%2] + n[ID1%2]+1)] = -ID0-1;
		trace[2*(n[ID0%2] + n[ID1%2]+1)+1] = -ID1-1;
		//Increment both observations since we have [wID0w1][rID0rID1]
		n[ID0%2] = n[ID0%2] + 1;
		n[ID1%2] = n[ID1%2] + 1;
	    }
	}
	while(n[ID0%2] < infoObj.numRounds){
	    //W0R0
	    trace[2*(n[ID0%2] + n[ID1%2])] = ID0+1;
	    trace[2*(n[ID0%2] + n[ID1%2])+1] = -ID0-1;
	    n[ID0%2] = n[ID0%2] + 1;
	}
	while(n[ID1%2] < infoObj.numRounds){
	    //W1R1
	    trace[2*(n[ID0%2] + n[ID1%2])] = ID1+1;
	    trace[2*(n[ID0%2] + n[ID1%2])+1] = -ID1-1;
	    n[ID1%2] = n[ID1%2] + 1;
	}
	if(count>0){
	    System.out.println("W0W1 happened " + count + " times.");
	}
	else{
	    System.out.println("W0W1 never happened");
	}
    }
}
