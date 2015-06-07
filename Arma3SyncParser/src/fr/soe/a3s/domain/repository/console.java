package fr.soe.a3s.domain.repository;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.GZIPInputStream;

public class console {
	
    public static void main( String[] args ) throws Exception
    {
    	if(args.length != 2) {
            System.out.println( "error" );
    		return;
    	}
    	
        System.out.println( "Start" );
        ArrayList<String> arr = new ArrayList<String>();
        SyncTreeDirectory sync = null;
        try {
        	sync = (SyncTreeDirectory) readSync(args[1]);
		} catch (ClassNotFoundException | IOException e) {
			e.printStackTrace();
		}
		for (SyncTreeNode n : sync.getList()) {
			arr.add(n.getName());
		}
		arr.add(" - - sha1 - - ");
        List<SyncTreeLeaf> listleaf = new ArrayList<SyncTreeLeaf>();
        getLeafs(sync, listleaf);
        
    	String filePath = null;
    	String oldParentName = null;
    	String parentName = null;
        for (SyncTreeLeaf n : listleaf) {
        	filePath = null;
        	oldParentName = null;
        	parentName = null;
        	SyncTreeDirectory directory = n.getParent();
        	for(int i = 0; i<10; i++) {
        		if(directory == null)
        			break;
        		parentName = directory.getName();
        		if (parentName == oldParentName || parentName.contains("racine"))
        			break;
        		String temp = filePath;
        		if(temp == null)
        			filePath = parentName;
        		else {
        			filePath = parentName + '\\' + temp;
        		}
        		directory = directory.getParent();
        		oldParentName = parentName;
        	}
        	arr.add(filePath);
			arr.add(n.getName());
			arr.add(n.getSha1());
			arr.add(String.valueOf(n.getSize()));
		}
		try {
			File fout = new File(args[1] + ".txt");
			FileOutputStream fos = new FileOutputStream(fout);
		 
			BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(fos));
			for (String str : arr) {
				bw.write(str);
				bw.newLine();
			}
			bw.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
        System.out.println( "Successfully" );
    }
    
	private static void getLeafs(SyncTreeDirectory syncTreeDirectory,
			List<SyncTreeLeaf> listleaf) {

		for (SyncTreeNode node : syncTreeDirectory.getList()) {
			if (node.isLeaf()) {
				SyncTreeLeaf leaf = (SyncTreeLeaf) node;
				listleaf.add(leaf);
			} else {
				SyncTreeDirectory directory = (SyncTreeDirectory) node;
				getLeafs(directory, listleaf);
			}
		}
	}
	
	private static void generate(SyncTreeNode syncTreeNode, ArrayList<String> arr) {

		if (!syncTreeNode.isLeaf()) {
			SyncTreeDirectory directory = (SyncTreeDirectory) syncTreeNode;
			for (SyncTreeNode n : directory.getList()) {
				arr.add(n.getName());
				generate(n, arr);
			}
		} else {
			final SyncTreeLeaf leaf = (SyncTreeLeaf) syncTreeNode;
			if (leaf.getDestinationPath() != null) {
				arr.add(leaf.getDestinationPath() + "/" + leaf.getName());
			}
		}
	}
    

	public static SyncTreeDirectory readSync(String path)
			throws FileNotFoundException, IOException, ClassNotFoundException {

		SyncTreeDirectory sync = null;
		File file = new File(path);
		if (file.exists()) {
			ObjectInputStream fRo = new ObjectInputStream(
					new GZIPInputStream(new FileInputStream(file)));
			sync = (SyncTreeDirectory) fRo.readObject();
			fRo.close();
		}
		return sync;
	}
}
