import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

x = np.linspace(0,4,50) # Example data

def func(x, a, b, c, d):
    return a * np.exp(b * x) + c * np.exp(d * x)

y = func(x, 14.18, -2.917, 0.0164, -0.0808) # Example exponential data

# Here you give the initial parameters for a,b,c which Python then iterates over
# to find the best fit
popt, pcov = curve_fit(func,x,y,p0=(1.0,1.0,1.0,1.0))

print(popt) # This contains your three best fit parameters

p1 = popt[0] # This is your a
p2 = popt[1] # This is your b
p3 = popt[2] # This is your c
p4 = popt[3] # This is your d

residuals = y - func(x,p1,p2,p3,p4)
fres = sum( (residuals**2)/func(x,p1,p2,p3,p4) ) # The chi-sqaure of your fit

print(fres)

""" Now if you need to plot, perform the code below """
curvey = func(x,p1,p2,p3,p4) # This is your y axis fit-line

plt.plot(x, curvey, 'red', label='The best-fit line')
plt.scatter(x,y, c='b',label='The data points')
plt.legend(loc='best')
plt.xlabel('x')
plt.ylabel('y')
plt.show()