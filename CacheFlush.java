//import sun.misc.Unsafe;
//import java.lang.reflect.Field;

public class CacheFlush<T>{
    public int cacheFlush(T obj){
	/*
	//Get an unsafe object
	long address = 0;
	try{
	    Field f = Unsafe.class.getDeclaredField("theUnsafe");
	    f.setAccessible(true);
	    Unsafe unsafe = (Unsafe) f.get(null);
	    //Use obj to get mem address of passed obj
	    address = unsafe.getInt(obj, 0);
	    //unsafe.arrayBaseOffset(obj.getClass())
	}
	catch(Exception e){
	    System.out.println("Error: Unable to make unsafe obj. " +
			       e.getMessage());
	    return -1;
	}
	System.out.println("Obj addr: " + address);
	*/
	return clfsh(obj);
    }
    private native int clfsh(T obj);
    static{
	System.loadLibrary("CacheFlush");
    }
}
