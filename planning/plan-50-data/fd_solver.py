import numpy as np, scipy.sparse as sp, scipy.sparse.linalg as sla
C0=299792458.0; EPS0=8.8541878128e-12
def solve(nx, ny, eps, fixed):
    """eps: (ny,nx) cell relative permittivity at nodes' cells; fixed: (ny,nx) array nan=free else V. Node-centered grid;
    edge permittivity = mean of the two adjacent cells approximated by node eps average."""
    idx = np.arange(nx*ny).reshape(ny,nx)
    rows=[];cols=[];vals=[]
    diag=np.zeros(nx*ny)
    def add(a,b,e):
        # conductance e between nodes a,b
        rows.extend(a.ravel()); cols.extend(b.ravel()); vals.extend(-e.ravel())
        rows.extend(b.ravel()); cols.extend(a.ravel()); vals.extend(-e.ravel())
        np.add.at(diag,a.ravel(),e.ravel()); np.add.at(diag,b.ravel(),e.ravel())
    ex=(eps[:,:-1]+eps[:,1:])/2; add(idx[:,:-1],idx[:,1:],ex)
    ey=(eps[:-1,:]+eps[1:,:])/2; add(idx[:-1,:],idx[1:,:],ey)
    A=sp.coo_matrix((vals+list(diag),(rows+list(range(nx*ny)),cols+list(range(nx*ny)))),shape=(nx*ny,nx*ny)).tocsr()
    f=~np.isnan(fixed.ravel()); V=np.where(f,fixed.ravel(),0.0)
    free=~f
    b=-A[free][:,f]@V[f]
    V[free]=sla.spsolve(A[free][:,free].tocsc(),b)
    V=V.reshape(ny,nx)
    # energy
    cap=0.0
    cap+=np.sum(ex*(V[:,:-1]-V[:,1:])**2); cap+=np.sum(ey*(V[:-1,:]-V[1:,:])**2)
    return cap  # relative, per unit length times EPS0 (C = EPS0*cap) for V=1 conductor
def zfrom(cdiel,cair): return 1.0/(C0*EPS0*np.sqrt(cdiel*cair))

def microstrip(w,h,t,er,d,half=None,odd_gap=None):
    """single or (odd mode of) coupled pair when odd_gap given (symmetry wall at x=0 grounded)."""
    X=w+8*h+ (odd_gap or 0); Y=8*h+t
    if odd_gap is None: x0=-X; x1=X
    else: x0=0.0; x1=X+ odd_gap/2+w
    nx=int(round((x1-x0)/d))+1; ny=int(round(Y/d))+1
    xs=x0+np.arange(nx)*d; ys=np.arange(ny)*d
    XX,YY=np.meshgrid(xs,ys)
    def run(er):
        eps=np.where(YY<=h+1e-9,er,1.0)
        fixed=np.full((ny,nx),np.nan)
        fixed[0,:]=0; fixed[-1,:]=0; fixed[:,-1]=0
        if odd_gap is None: fixed[:,0]=0
        else: fixed[:,0]=0  # odd mode: wall at x=0 grounded
        if odd_gap is None: xa,xb=-w/2,w/2
        else: xa,xb=odd_gap/2, odd_gap/2+w
        m=(XX>=xa-1e-9)&(XX<=xb+1e-9)&(YY>=h-1e-9)&(YY<=h+t+1e-9)
        fixed[m]=1.0
        return solve(nx,ny,eps,fixed)
    c=run(er); ca=run(1.0)
    if odd_gap is None: return zfrom(c,ca)
    return 2*zfrom(c,ca)
def stripline(w,b,t,er,d,odd_gap=None):
    X=w+4*b+(odd_gap or 0)
    if odd_gap is None: x0=-X; x1=X
    else: x0=0.0; x1=X+odd_gap/2+w
    nx=int(round((x1-x0)/d))+1; ny=int(round(b/d))+1
    xs=x0+np.arange(nx)*d; ys=np.arange(ny)*d
    XX,YY=np.meshgrid(xs,ys)
    def run(er):
        eps=np.full((ny,nx),er)
        fixed=np.full((ny,nx),np.nan)
        fixed[0,:]=0; fixed[-1,:]=0; fixed[:,-1]=0; fixed[:,0]=0
        if odd_gap is None: xa,xb=-w/2,w/2
        else: xa,xb=odd_gap/2, odd_gap/2+w
        yc=b/2
        m=(XX>=xa-1e-9)&(XX<=xb+1e-9)&(YY>=yc-t/2-1e-9)&(YY<=yc+t/2+1e-9)
        fixed[m]=1.0
        return solve(nx,ny,eps,fixed)
    c=run(er); ca=run(1.0)
    if odd_gap is None: return zfrom(c,ca)
    return 2*zfrom(c,ca)
