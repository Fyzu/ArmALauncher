package fr.soe.a3s.domain.repository;


@SuppressWarnings("rawtypes")
public interface SyncTreeNode extends java.lang.Comparable{

	public String getName();
	public void setName(String name);
	public String getDestinationPath();
	public SyncTreeDirectory  getParent();
	public void setParent(SyncTreeDirectory parent);
	public boolean isLeaf();
	public void setDeleted(boolean value);
	public boolean isDeleted();
	public void setUpdated(boolean value);
	public boolean isUpdated();
	
}
