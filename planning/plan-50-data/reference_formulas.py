import numpy as np
from scipy.special import ellipk
ETA0=376.730313668
def zl(u):
    fu=6+(2*np.pi-6)*np.exp(-(30.666/u)**0.7528)
    return ETA0/2/np.pi*np.log(fu/u+np.sqrt(1+(2/u)**2))
def eeff(u,er):
    a=1+np.log((u**4+(u/52)**2)/(u**4+0.432))/49+np.log(1+(u/18.1)**3)/18.7
    b=0.564*((er-0.9)/(er+3))**0.053
    return (er+1)/2+(er-1)/2*(1+10/u)**(-a*b)
def ms_z0(w,h,t,er):
    u=w/h; tt=t/h
    du1=tt/np.pi*np.log(1+4*np.e/tt*np.tanh(np.sqrt(6.517*u))**2) if tt>0 else 0
    dur=du1*(1+1/np.cosh(np.sqrt(er-1)))/2
    ur=u+dur
    return zl(ur)/np.sqrt(eeff(ur,er))
def ms_diff(w,s,h,t,er): return 2*ms_z0(w,h,t,er)*(1-0.48*np.exp(-0.96*s/h))
def we(w,t): return w+0.5*(t/np.pi)*(1+np.log(4*np.pi*w/t)) if t>0 else w
def sl_centred(w,b,t,er):
    k=np.tanh(np.pi*we(w,t)/(2*b)); kp=np.sqrt(1-k*k)
    return 30*np.pi/np.sqrt(er)*ellipk(kp**2)/ellipk(k**2)
def sl_odd_centred(w,s,b,t,er):
    a=np.pi*we(w,t)/(2*b); c=np.pi*(w+s)/(2*b)
    ko=np.tanh(a)/np.tanh(c); kp=np.sqrt(1-ko*ko)
    return 30*np.pi/np.sqrt(er)*ellipk(kp**2)/ellipk(ko**2)
def sl_z0(w,hA,hB,t,er):
    za=sl_centred(w,2*hA+t,t,er); zb=sl_centred(w,2*hB+t,t,er); return 2*za*zb/(za+zb)
def sl_diff(w,s,hA,hB,t,er):
    za=2*sl_odd_centred(w,s,2*hA+t,t,er); zb=2*sl_odd_centred(w,s,2*hB+t,t,er); return 2*za*zb/(za+zb)
def current(width_mil,thick_mm,dT,external):
    k=0.048 if external else 0.024
    area=width_mil*(thick_mm/0.0254)
    return k*dT**0.44*area**0.725
def width_for_current(I,thick_mm,dT,external):
    k=0.048 if external else 0.024
    area=(I/(k*dT**0.44))**(1/0.725); return area/(thick_mm/0.0254)
if __name__=="__main__":
    import skrf as rf, warnings; warnings.filterwarnings("ignore")
    for (w,h,t,er) in [(0.35,0.2104,0.035,4.4),(3.0,1.51,0.035,4.5),(0.2,0.0994,0.035,4.1)]:
        sk=rf.media.MLine(rf.Frequency(1,1,1,'mhz'),w=w*1e-3,h=h*1e-3,t=t*1e-3,ep_r=er).z0[0].real
        print("MS z0 w=%g h=%g t=%g er=%g: mine=%.3f skrf=%.3f"%(w,h,t,er,ms_z0(w,h,t,er),sk))
    print("MS diff w=0.15 s=0.15 h=0.2104 t=0.035 er=4.4: %.3f"%ms_diff(0.15,0.15,0.2104,0.035,4.4))
    print("SL centred w=0.15 b=1.3 t=0.0152 er=4.6: %.3f"%sl_centred(0.15,1.3,0.0152,4.6))
    print("SL asym w=0.15 hA=0.0994 hB=0.55 t=0.0152 er=4.3: %.3f"%sl_z0(0.15,0.0994,0.55,0.0152,4.3))
    print("SL diff w=0.12 s=0.12 hA=0.0994 hB=0.55 t=0.0152 er=4.3: %.3f"%sl_diff(0.12,0.12,0.0994,0.55,0.0152,4.3))
    print("I 10mil 1oz(0.035) 10C ext: %.4f A ; int 0.0152: %.4f"%(current(10,0.035,10,True),current(10,0.0152,10,False)))
    print("width for 1A, 0.035, 10C, ext: %.3f mil"%width_for_current(1.0,0.035,10,True))
    # solver: width for 50 ohm on h=0.2104 er=4.4 t=0.035 (bisection on mil)
    lo,hi=0.1,100.0
    for _ in range(60):
        mid=(lo+hi)/2
        if ms_z0(mid*0.0254,0.2104,0.035,4.4)>50: lo=mid
        else: hi=mid
    print("50 ohm microstrip width mil: %.3f"%lo)
    lo,hi=0.1,400.0
    for _ in range(60):
        mid=(lo+hi)/2
        if ms_z0(mid*0.0254,1.51,0.035,4.5)>50: lo=mid
        else: hi=mid
    print("50 ohm 2-layer 1.51mm width mil: %.2f (cap 100)"%lo)
