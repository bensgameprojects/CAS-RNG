import java.util.concurrent.atomic.*;
public class RecInfo{
    public int maxTraceLength;
    public int obsCount;
    public int numRounds;
    public AtomicIntegerArray[][] thdObs;
    public RecInfo(int mtl, int oc, int nr, AtomicIntegerArray[][] ob){
	maxTraceLength = mtl;
	obsCount = oc;
	numRounds = nr;
	thdObs = ob;
    }
}
