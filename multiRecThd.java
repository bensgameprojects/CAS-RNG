import java.util.concurrent.atomic.*;

public class multiRecThd implements Runnable{
    private int[] n;
    private int[] trace;
    private RecInfo infoObj;
    private int cnt;
    private int index;
    public multiRecThd(int[] t, RecInfo info, int thdCnt){
	trace = t;
	infoObj = info;
	cnt = thdCnt;
	n = new int[thdCnt];
	for(int i = 0; i< thdCnt; i++){
	    n[i] = 0;
	}
	index = 0;
    }
    public void run(){
	int count = cnt;
	while(count > 1){
	    //of the remaining threads
	    //reconstruct ordering...
	    //of remaining threads check obs:
	    //for all i
	    boolean found = false;
	    /*
	    for(int i = 0; i < cnt; i++){
		if(!found && n[i] < infoObj.numRounds){
		    //if all obs. of thread i are -1,
		    boolean all = true;
		    for(int j = 0; j < cnt; j++){
			if(j != i){
			    if(infoObj.thdObs[i][n[i]].get(j) != -1){
				all = false;
			    }
			}
		    }
		    if(all == true){
			//Then we have WiRi
			trace[index] = i+1;
			//trace[2*index+1] = -i -1;
			//We processed it! :0
			index++;
			n[i]++;
			found = true;
		    }
		}
	    }
	    if(!found){*/
	    //else found is false:
	    //some observations of thread i are -1 but not all...
	    //of remaining threads check obs:
	    //for all i
	    for(int i = 0; i < cnt; i++){
		if(!found && n[i] < infoObj.numRounds){
		    //if thread i's (obs for j is < index of j)
		    boolean all = true;
		    for(int j = 0; j < cnt; j++){
			if(i != j){
			    if(infoObj.thdObs[i][n[i]].get(j) >= n[j]){
				all = false;
			    }
			}
		    }
		    //for all j != i,
		    if(all == true){
			// then we have WiRi
			trace[index] = i+1;
			//trace[2*index+1] = -i -1;
			//We processed it! :0
			index++;
			n[i]++;
			found = true;
		    }
		}
	    }
	    
	    if(!found){
		System.out.println("They actually in parallel ree :(");
	    }
	    //else at least 1 j is the same
	       //and these j's and i have a parallel write Wi||Wj's
	       
	       //for all i
	          //for all j != i
	             //if(i's obs of j == index of j)
	                 //then WiWj1W2...Wjk
	                 //keep track of j1, ..., jk
	       //while(any i has observation of j > index of j)
	    
	          //for all i
	             //if i's obs. of j > index of j
	                //
	    count = 0;
	    //update remaining threads
	    for(int i = 0; i < cnt; i++){
		if(n[i] < infoObj.numRounds){
		    count++;
		}
	    }
	}
	//while remaining thread has rounds
	//add all at end
	for(int i = 0; i < cnt; i++){
	    while(n[i] < infoObj.numRounds){
		trace[index] = i+1;
		//trace[2*index+1] = -i -1;
		index++;
		n[i]++;
	    }
	}
    }
}
